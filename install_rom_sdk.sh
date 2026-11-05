#!/usr/bin/env bash
# Builds, tests and installs rom_sdk into /opt/rom.
#
# Steps: create temp build dir -> configure -> build -> test -> install ->
#        save install manifest -> register /opt/rom/lib with ldconfig ->
#        remove build dir.
#
# DESTDIR=<dir> stages the install under <dir> without sudo or ldconfig.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly REPO_ROOT
readonly PREFIX="/opt/rom"
readonly DEST="${DESTDIR:-}"
readonly MANIFEST="${PREFIX}/share/rom/install_manifest.txt"
readonly LD_CONF="/etc/ld.so.conf.d/rom.conf"

build_type="Release"
run_tests="ON"
jobs="$(nproc 2>/dev/null || echo 2)"

usage() {
  cat <<USAGE
Usage: $(basename "$0") [options]

Options:
  --skip-tests         Do not build or run unit tests
  --build-type <type>  CMake build type (default: Release)
  --jobs <n>           Parallel build jobs (default: ${jobs})
  -h, --help           Show this help
USAGE
}

log() { printf '\033[1;32m[rom]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[rom] error:\033[0m %s\n' "$*" >&2; exit 1; }

# Runs a command with root privileges unless staging into DESTDIR.
as_root() {
  if [[ -n "${DEST}" || "${EUID}" -eq 0 ]]; then
    "$@"
  else
    sudo "$@"
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-tests) run_tests="OFF"; shift ;;
    --build-type) build_type="${2:?missing value}"; shift 2 ;;
    --jobs) jobs="${2:?missing value}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) usage; die "unknown option: $1" ;;
  esac
done

command -v cmake >/dev/null || die "cmake not found (sudo apt install cmake)"
command -v c++ >/dev/null || die "C++ compiler not found (sudo apt install g++)"

if [[ -f "${DEST}${MANIFEST}" ]]; then
  log "Existing install found in ${DEST}${PREFIX}; removing it first"
  "${REPO_ROOT}/uninstall_rom_sdk.sh"
fi

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/rom_sdk_build.XXXXXX")"
trap 'rm -rf "${build_dir}"' EXIT
log "Build dir: ${build_dir}"

log "Configuring (${build_type}, tests=${run_tests})"
cmake -S "${REPO_ROOT}" -B "${build_dir}" \
  -DCMAKE_BUILD_TYPE="${build_type}" \
  -DROM_BUILD_TESTS="${run_tests}"

log "Building with ${jobs} jobs"
cmake --build "${build_dir}" -j "${jobs}"

if [[ "${run_tests}" == "ON" ]]; then
  log "Running tests"
  ctest --test-dir "${build_dir}" --output-on-failure
fi

log "Installing into ${DEST}${PREFIX}"
as_root env DESTDIR="${DEST}" cmake --install "${build_dir}"

# Keep the manifest inside the prefix so uninstall works after the build
# dir is gone. The manifest lists itself so it is removed too.
cp "${build_dir}/install_manifest.txt" "${build_dir}/rom_manifest.txt"
printf '\n%s\n' "${MANIFEST}" >> "${build_dir}/rom_manifest.txt"
as_root install -D -m 644 "${build_dir}/rom_manifest.txt" "${DEST}${MANIFEST}"

log "Registering ${PREFIX}/lib with the dynamic loader"
echo "${PREFIX}/lib" > "${build_dir}/rom.conf"
as_root install -D -m 644 "${build_dir}/rom.conf" "${DEST}${LD_CONF}"
if [[ -z "${DEST}" ]]; then
  as_root ldconfig
fi

log "Done. Use -DCMAKE_PREFIX_PATH=${PREFIX} with find_package(rom)."
