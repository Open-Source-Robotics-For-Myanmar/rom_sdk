#!/usr/bin/env python3
"""Convert and verify bc_mlp_v2_debiased.hpp as rom::intelligence::model::rl::bc_mlp_v4_ppo."""

from __future__ import annotations

import math
import pathlib
import re
import subprocess
import sys
import tempfile
from typing import Dict, List, Sequence, Tuple


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
HPP_PATH = pathlib.Path(__file__).resolve().parent / "bc_mlp_v2_debiased.hpp"
ROM_RL_DIR = REPO_ROOT / "rom" / "intelligence" / "model" / "rl" / "bc_mlp_v4_ppo"
ROM_HEADER_PATH = ROM_RL_DIR / "BcMlpV4Ppo.h"
ROM_SOURCE_PATH = ROM_RL_DIR / "BcMlpV4Ppo.cc"


def parse_float_arrays(hpp_text: str) -> Dict[str, List[float]]:
    pattern = re.compile(
        r"constexpr\s+std::array<float,\s*[^>]+>\s+(\w+)\s*=\s*\{([^}]*)\};",
        re.MULTILINE | re.DOTALL,
    )
    arrays: Dict[str, List[float]] = {}
    for match in pattern.finditer(hpp_text):
        name = match.group(1)
        body = match.group(2)
        values = [
            float(tok.rstrip("fF"))
            for tok in re.findall(r"[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?[fF]?", body)
        ]
        arrays[name] = values
    return arrays


def f32(val: float) -> float:
    import struct

    return struct.unpack("<f", struct.pack("<f", float(val)))[0]


def wrap_pi(angle: float) -> float:
    return math.remainder(angle, 2.0 * math.pi)


def clamp_f(val: float, lo: float, hi: float) -> float:
    return f32(max(lo, min(hi, val)))


class PythonCausalFeatureObserver:
    def __init__(self) -> None:
        self.history: List[Tuple[float, float, float, float, float, float]] = []

    def reset(self) -> None:
        self.history.clear()

    def update(
        self,
        time_s: float,
        valid: bool,
        ex: float,
        ey: float,
        los_v: float,
        los_z: float,
        width: float,
        vx_ned: float,
        vy_ned: float,
        vz_ned: float,
        cx: float = 960.0,
        cy: float = 540.0,
    ) -> Tuple[bool, List[float]]:
        if not valid or not (cx > 0.0) or not (cy > 0.0):
            self.history.clear()
            return False, [0.0] * 13
        if self.history:
            step_dt = time_s - self.history[-1][0]
            if step_dt <= 0.0 or step_dt > 0.05001:
                self.history.clear()
        if len(self.history) == 4:
            self.history.pop(0)
        self.history.append((time_s, ex, ey, los_v, los_z, width))
        if len(self.history) < 4:
            return False, [0.0] * 13
        old = self.history[0]
        elapsed = time_s - old[0]
        if not (0.005 < elapsed <= 0.25):
            if elapsed > 0.25:
                self.history.clear()
            return False, [0.0] * 13
        d_elevation = wrap_pi(los_v - old[3]) / elapsed
        d_azimuth = wrap_pi(los_z - old[4]) / elapsed
        dw = (width - old[5]) / elapsed

        out = [
            clamp_f(ex / cx, -1.5, 1.5),
            clamp_f(ey / cy, -1.5, 1.5),
            clamp_f(los_v / (math.pi / 4.0), -2.0, 2.0),
            clamp_f(d_elevation / 2.0, -2.0, 2.0),
            clamp_f(d_azimuth / 2.0, -2.0, 2.0),
            clamp_f((width - 100.0) / 150.0, -1.0, 2.0),
            clamp_f(dw / 100.0, -2.0, 2.0),
            clamp_f(math.hypot(vx_ned, vy_ned) / 8.0, 0.0, 1.5),
            clamp_f(vz_ned / 5.0, -1.5, 1.5),
            f32(math.sin(los_z)),
            f32(math.cos(los_z)),
            clamp_f(vx_ned / 8.0, -1.5, 1.5),
            clamp_f(vy_ned / 8.0, -1.5, 1.5),
        ]
        return True, out


def forward_physical_py(obs: Sequence[float], weights: Dict[str, List[float]]) -> List[float]:
    w0, b0 = weights["kW0"], weights["kB0"]
    w2, b2 = weights["kW2"], weights["kB2"]
    w4, b4 = weights["kW4"], weights["kB4"]
    mean, scale = weights["kActionMean"], weights["kActionScale"]

    h1: List[float] = []
    for i in range(32):
        acc = f32(b0[i])
        row = i * 13
        for j in range(13):
            acc = f32(acc + f32(w0[row + j]) * f32(obs[j]))
        h1.append(f32(math.tanh(acc)))

    h2: List[float] = []
    for i in range(16):
        acc = f32(b2[i])
        row = i * 32
        for j in range(32):
            acc = f32(acc + f32(w2[row + j]) * h1[j])
        h2.append(f32(math.tanh(acc)))

    out: List[float] = []
    for i in range(4):
        acc = f32(b4[i])
        row = i * 16
        for j in range(16):
            acc = f32(acc + f32(w4[row + j]) * h2[j])
        out.append(f32(f32(acc * f32(scale[i])) + f32(mean[i])))
    return out


def limit_physical_action_py(
    raw: Sequence[float],
    speed_limit: float = 8.0,
    vz_limit: float = 3.0,
    yaw_rate_limit: float = 1.8,
) -> List[float]:
    out = [f32(v) for v in raw]
    if any(not math.isfinite(v) for v in out):
        return [0.0, 0.0, 0.0, 0.0]
    horiz_speed = math.hypot(float(out[0]), float(out[1]))
    eff_speed_limit = max(0.0, speed_limit)
    if horiz_speed > eff_speed_limit and horiz_speed > 1e-9:
        scale = f32(eff_speed_limit / horiz_speed)
        out[0] = f32(out[0] * scale)
        out[1] = f32(out[1] * scale)
    out[2] = clamp_f(out[2], -vz_limit, vz_limit)
    out[3] = clamp_f(out[3], -yaw_rate_limit, yaw_rate_limit)
    return out


def run_cpp_equivalence_check(weights: Dict[str, List[float]]) -> None:
    samples = [
        (0.00, 25.0, -15.0, 0.12, 0.35, 110.0, 3.5, -1.2, -0.4),
        (0.02, 22.0, -12.0, 0.13, 0.36, 114.0, 3.6, -1.1, -0.3),
        (0.04, 19.0, -10.0, 0.14, 0.37, 118.0, 3.7, -1.0, -0.2),
        (0.06, 15.0, -8.0, 0.15, 0.38, 122.0, 3.8, -0.9, -0.1),
    ]

    py_obs = PythonCausalFeatureObserver()
    ok = False
    feats: List[float] = []
    for s in samples:
        ok, feats = py_obs.update(s[0], True, *s[1:])
    assert ok, "Expected 4th sample to produce valid features"
    py_raw = forward_physical_py(feats, weights)
    py_act = limit_physical_action_py(py_raw)

    cpp_harness = r"""
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v2_debiased.hpp"
#include "rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h"

namespace rl = rom::intelligence::model::rl::bc_mlp_v4_ppo;

int main()
{
  struct Sample
  {
    double t;
    double ex;
    double ey;
    double los_v;
    double los_z;
    double width;
    double vx;
    double vy;
    double vz;
  };

  const std::array<Sample, 4> samples{{
      {0.00, 25.0, -15.0, 0.12, 0.35, 110.0, 3.5, -1.2, -0.4},
      {0.02, 22.0, -12.0, 0.13, 0.36, 114.0, 3.6, -1.1, -0.3},
      {0.04, 19.0, -10.0, 0.14, 0.37, 118.0, 3.7, -1.0, -0.2},
      {0.06, 15.0, -8.0, 0.15, 0.38, 122.0, 3.8, -0.9, -0.1},
  }};

  rom_bc_v2::CausalFeatureObserver orig_obs;
  rl::CausalFeatureObserver sdk_obs;
  rl::BcMlpV4Ppo sdk_model;

  std::array<float, rom_bc_v2::kInputDim> orig_features{};
  std::array<float, rl::kInputDim> sdk_features{};
  rl::BcMlpV4Ppo::ActionVector sdk_step_action{};

  bool orig_ok = false;
  bool sdk_ok = false;
  bool step_ok = false;

  for (const auto& s : samples)
  {
    orig_ok = orig_obs.update(
        s.t, true, s.ex, s.ey, s.los_v, s.los_z, s.width, s.vx, s.vy, s.vz,
        orig_features);
    sdk_ok = sdk_obs.Update(
        s.t, true, s.ex, s.ey, s.los_v, s.los_z, s.width, s.vx, s.vy, s.vz,
        sdk_features);
    step_ok = sdk_model.Step(
        s.t, true, s.ex, s.ey, s.los_v, s.los_z, s.width, s.vx, s.vy, s.vz,
        sdk_step_action);
  }

  if (!orig_ok || !sdk_ok || !step_ok)
  {
    return 1;
  }

  for (std::size_t i = 0; i < rl::kInputDim; ++i)
  {
    if (std::abs(orig_features[i] - sdk_features[i]) > 1e-6f)
    {
      return 2;
    }
  }

  const auto orig_raw = rom_bc_v2::forward_physical(orig_features);
  const auto sdk_raw = rl::ForwardPhysical(sdk_features);
  const auto orig_act = rom_bc_v2::limit_physical_action(orig_raw);
  const auto sdk_act = rl::LimitPhysicalAction(sdk_raw);

  for (std::size_t i = 0; i < rl::kOutputDim; ++i)
  {
    if (std::abs(orig_raw[i] - sdk_raw[i]) > 1e-6f ||
        std::abs(orig_act[i] - sdk_act[i]) > 1e-6f ||
        std::abs(orig_act[i] - sdk_step_action[i]) > 1e-6f)
    {
      return 3;
    }
  }

  std::cout << std::setprecision(9);
  for (std::size_t i = 0; i < rl::kOutputDim; ++i)
  {
    std::cout << sdk_act[i] << (i + 1 == rl::kOutputDim ? "\n" : " ");
  }
  return 0;
}
"""
    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = pathlib.Path(tmp_dir)
        harness_cc = tmp_path / "harness.cc"
        harness_bin = tmp_path / "harness"
        harness_cc.write_text(cpp_harness, encoding="utf-8")

        subprocess.run(
            [
                "g++",
                "-std=c++17",
                "-O2",
                f"-I{REPO_ROOT}",
                str(harness_cc),
                str(ROM_SOURCE_PATH),
                "-o",
                str(harness_bin),
            ],
            check=True,
        )
        proc = subprocess.run(
            [str(harness_bin)],
            check=True,
            capture_output=True,
            text=True,
        )
        cpp_act = [float(x) for x in proc.stdout.strip().split()]

    for i, (py_v, cpp_v) in enumerate(zip(py_act, cpp_act)):
        diff = abs(py_v - cpp_v)
        assert diff <= 1e-5, f"Action[{i}] mismatch: py={py_v} vs cpp={cpp_v} (diff={diff})"

    print("[PASS] C++ rom::intelligence::model::rl::bc_mlp_v4_ppo matches bc_mlp_v2_debiased.hpp and Python reference.")
    print(
        "       Actions [vx_ned, vy_ned, vz_ned, yaw_rate] =",
        [round(x, 6) for x in cpp_act],
    )


def main() -> int:
    assert HPP_PATH.is_file(), f"Missing {HPP_PATH}"
    assert ROM_HEADER_PATH.is_file(), f"Missing {ROM_HEADER_PATH}"
    assert ROM_SOURCE_PATH.is_file(), f"Missing {ROM_SOURCE_PATH}"

    hpp_text = HPP_PATH.read_text(encoding="utf-8")
    src_text = ROM_SOURCE_PATH.read_text(encoding="utf-8")

    hpp_arrays = parse_float_arrays(hpp_text)
    src_arrays = parse_float_arrays(src_text)

    expected_shapes = {
        "kW0": 32 * 13,
        "kB0": 32,
        "kW2": 16 * 32,
        "kB2": 16,
        "kW4": 4 * 16,
        "kB4": 4,
        "kActionMean": 4,
        "kActionScale": 4,
    }

    for name, expected_len in expected_shapes.items():
        assert name in hpp_arrays, f"{name} missing in {HPP_PATH.name}"
        assert name in src_arrays, f"{name} missing in {ROM_SOURCE_PATH.name}"
        assert len(hpp_arrays[name]) == expected_len, f"{name} length != {expected_len}"
        assert hpp_arrays[name] == src_arrays[name], f"{name} values differ between .hpp and SDK .cc"

    print("[PASS] Weight & bias arrays in BcMlpV4Ppo.cc match bc_mlp_v2_debiased.hpp (100% identical).")
    run_cpp_equivalence_check(hpp_arrays)
    return 0


if __name__ == "__main__":
    sys.exit(main())
