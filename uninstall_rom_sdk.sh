#!/usr/bin/env bash
# Uninstalls rom_sdk from /opt/rom.
#
# Steps: create temp build dir -> configure -> run CMake uninstall with the
#        manifest saved by install_rom_sdk.sh -> remove ldconfig entry ->
#        remove build dir.
#
# DESTDIR=<dir> must match the value used for install_rom_sdk.sh.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly REPO_ROOT
readonly PREFIX="/opt/rom"
readonly DEST="${DESTDIR:-}"
readonly MANIFEST="${PREFIX}/share/rom/install_manifest.txt"
readonly LD_CONF="/etc/ld.so.conf.d/rom.conf"

usage() {
  cat <<USAGE
Usage: $(basename "$0") [-h|--help]

Removes every file installed by install_rom_sdk.sh from ${PREFIX}.
USAGE
}

log() { printf '\033[1;32m[rom]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[rom] warning:\033[0m %s\n' "$*" >&2; }
die() { printf '\033[1;31m[rom] error:\033[0m %s\n' "$*" >&2; exit 1; }

as_root() {
  if [[ -n "${DEST}" || "${EUID}" -eq 0 ]]; then
    "$@"
  else
    sudo "$@"
  fi
}

case "${1:-}" in
  -h|--help) usage; exit 0 ;;
  "") ;;
  *) usage; die "unknown option: $1" ;;
esac

command -v cmake >/dev/null || die "cmake not found (sudo apt install cmake)"

if [[ ! -f "${DEST}${MANIFEST}" ]]; then
  if [[ -d "${DEST}${PREFIX}" ]]; then
    die "${DEST}${MANIFEST} not found. Remove manually: sudo rm -rf ${DEST}${PREFIX}"
  fi
  log "rom_sdk is not installed in ${DEST}${PREFIX}; nothing to do"
  exit 0
fi

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/rom_sdk_build.XXXXXX")"
trap 'rm -rf "${build_dir}"' EXIT
log "Build dir: ${build_dir}"

# Configure only to generate cmake_uninstall.cmake; nothing is compiled.
log "Configuring"
cmake -S "${REPO_ROOT}" -B "${build_dir}" -DROM_BUILD_TESTS=OFF >/dev/null

cp "${DEST}${MANIFEST}" "${build_dir}/install_manifest.txt"

log "Removing files from ${DEST}${PREFIX}"
as_root env DESTDIR="${DEST}" cmake -P "${build_dir}/cmake_uninstall.cmake"

if [[ -f "${DEST}${LD_CONF}" ]]; then
  log "Removing ${DEST}${LD_CONF}"
  as_root rm -f "${DEST}${LD_CONF}"
  if [[ -z "${DEST}" ]]; then
    as_root ldconfig
  fi
fi

if [[ -d "${DEST}${PREFIX}" ]]; then
  warn "${DEST}${PREFIX} still contains files not installed by rom_sdk:"
  find "${DEST}${PREFIX}" -mindepth 1 >&2
fi

log "Done."
