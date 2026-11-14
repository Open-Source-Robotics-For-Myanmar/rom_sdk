#ifndef ROM_INTELLIGENCE_MODEL_RL_BC_MLP_V4_PPO_BCMLPV4PPO_H_
#define ROM_INTELLIGENCE_MODEL_RL_BC_MLP_V4_PPO_BCMLPV4PPO_H_

// Ported from bc_mlp_v2_debiased.hpp (generated from bc_mlp_v4_ppo.pt).
// Checkpoint SHA256: 04433b83aba5a2b4059794a42ff559ac7e24033f70652960173d6bbdb369ea53
// Format: bc_mlp_v2 | Contract: legacy25_bc_v2
// Azimuth convention: atan2(North, East) [East = 0, North = +pi/2]
// Outputs: [action_vx_ned_mps, action_vy_ned_mps, action_vz_ned_mps, action_yaw_rate_radps]

#include <array>
#include <cstddef>
#include <deque>

namespace rom::intelligence::model::rl::bc_mlp_v4_ppo
{

  constexpr std::size_t kInputDim = 13;
  constexpr std::size_t kHidden1 = 32;
  constexpr std::size_t kHidden2 = 16;
  constexpr std::size_t kOutputDim = 4;

  constexpr double kFocalLength = 810.0;
  constexpr double kImageWidth = 1920.0;
  constexpr double kImageHeight = 1080.0;
  constexpr double kCx = 960.0;
  constexpr double kCy = 540.0;

  constexpr std::array<const char*, kInputDim> kFeatureNames = {
      "ex_norm",       "ey_norm",          "los_v_norm",      "dlos_v_norm",
      "dlos_z_norm",   "w_norm",           "dw_norm",         "v_horiz_norm",
      "vz_norm",       "los_azimuth_sin",  "los_azimuth_cos", "vx_norm",
      "vy_norm"};

  constexpr std::array<const char*, kOutputDim> kActionNames = {
      "action_vx_ned_mps", "action_vy_ned_mps", "action_vz_ned_mps",
      "action_yaw_rate_radps"};

  struct RawSample
  {
    double time_s = 0.0;
    double ex = 0.0;
    double ey = 0.0;
    double los_v = 0.0;
    double los_z = 0.0;
    double width = 0.0;
  };

  class CausalFeatureObserver
  {
  public:
    void Reset();
    void reset()
    {
      Reset();
    }

    bool Update(
        double time_s,
        bool valid,
        double ex,
        double ey,
        double los_v,
        double los_z,
        double width,
        double vx_ned,
        double vy_ned,
        double vz_ned,
        std::array<float, kInputDim>& out_features,
        double cx = kCx,
        double cy = kCy);

    bool update(
        double time_s,
        bool valid,
        double ex,
        double ey,
        double los_v,
        double los_z,
        double width,
        double vx_ned,
        double vy_ned,
        double vz_ned,
        std::array<float, kInputDim>& out_features,
        double cx = kCx,
        double cy = kCy)
    {
      return Update(
          time_s, valid, ex, ey, los_v, los_z, width, vx_ned, vy_ned, vz_ned,
          out_features, cx, cy);
    }

  private:
    std::deque<RawSample> history_;
  };

  std::array<float, kOutputDim> ForwardPhysical(
      const std::array<float, kInputDim>& obs);

  inline std::array<float, kOutputDim> forward_physical(
      const std::array<float, kInputDim>& obs)
  {
    return ForwardPhysical(obs);
  }

  std::array<float, kOutputDim> LimitPhysicalAction(
      const std::array<float, kOutputDim>& raw,
      double speed_limit = 8.0,
      double vz_limit = 3.0,
      double yaw_rate_limit = 1.8);

  inline std::array<float, kOutputDim> limit_physical_action(
      const std::array<float, kOutputDim>& raw,
      double speed_limit = 8.0,
      double vz_limit = 3.0,
      double yaw_rate_limit = 1.8)
  {
    return LimitPhysicalAction(raw, speed_limit, vz_limit, yaw_rate_limit);
  }

  // Convenience class wrapping feature observation, MLP forward pass, and action limiting.
  class BcMlpV4Ppo
  {
  public:
    using FeatureVector = std::array<float, kInputDim>;
    using ActionVector = std::array<float, kOutputDim>;

    struct ActionLimits
    {
      double speed_limit = 8.0;
      double vz_limit = 3.0;
      double yaw_rate_limit = 1.8;
    };

    BcMlpV4Ppo() = default;
    explicit BcMlpV4Ppo(const ActionLimits& limits) : limits_(limits)
    {
    }

    void Reset()
    {
      observer_.Reset();
    }

    void reset()
    {
      observer_.Reset();
    }

    bool UpdateFeatures(
        double time_s,
        bool valid,
        double ex,
        double ey,
        double los_v,
        double los_z,
        double width,
        double vx_ned,
        double vy_ned,
        double vz_ned,
        FeatureVector& out_features,
        double cx = kCx,
        double cy = kCy)
    {
      return observer_.Update(
          time_s, valid, ex, ey, los_v, los_z, width, vx_ned, vy_ned, vz_ned,
          out_features, cx, cy);
    }

    bool Step(
        double time_s,
        bool valid,
        double ex,
        double ey,
        double los_v,
        double los_z,
        double width,
        double vx_ned,
        double vy_ned,
        double vz_ned,
        ActionVector& out_action,
        double cx = kCx,
        double cy = kCy);

    static ActionVector Forward(const FeatureVector& obs)
    {
      return ForwardPhysical(obs);
    }

    static ActionVector LimitAction(
        const ActionVector& raw,
        double speed_limit = 8.0,
        double vz_limit = 3.0,
        double yaw_rate_limit = 1.8)
    {
      return LimitPhysicalAction(raw, speed_limit, vz_limit, yaw_rate_limit);
    }

    const ActionLimits& limits() const
    {
      return limits_;
    }

    void set_limits(const ActionLimits& limits)
    {
      limits_ = limits;
    }

  private:
    CausalFeatureObserver observer_;
    ActionLimits limits_;
  };

}  // namespace rom::intelligence::model::rl::bc_mlp_v4_ppo

#endif  // ROM_INTELLIGENCE_MODEL_RL_BC_MLP_V4_PPO_BCMLPV4PPO_H_
