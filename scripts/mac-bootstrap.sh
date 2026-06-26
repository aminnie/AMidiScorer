#!/usr/bin/env bash
# Option 1: First-time / clean Mac setup — check toolchain, install CMake (and optional tools) via Homebrew,
# clone JUCE if missing, then run scripts/mac-build.sh with the same arguments.
#
# Does not install Xcode/CLT automatically (requires GUI). Does not auto-run the Homebrew installer
# (security / interactivity); install Homebrew from https://brew.sh if `brew` is missing.
#
# Usage:
#   ./scripts/mac-bootstrap.sh
#   ./scripts/mac-bootstrap.sh --release --ninja
#   (same flags as mac-build.sh: --release, --ninja, --xcode)

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
JUCE_ROOT="${JUCE_ROOT:-${ROOT_DIR}/.deps/JUCE}"
JUCE_URL="${JUCE_URL:-https://github.com/juce-framework/JUCE.git}"

echo "==> MidiScorer macOS bootstrap (Option 1)"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "This script is for macOS only." >&2
  exit 1
fi

if ! xcode-select -p >/dev/null 2>&1; then
  echo "" >&2
  echo "Xcode Command Line Tools are not installed (or xcode-select has no path)." >&2
  echo "Install them, then re-run this script:" >&2
  echo "  xcode-select --install" >&2
  echo "If you use full Xcode, install it from the App Store and run:" >&2
  echo "  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer" >&2
  exit 1
fi

if ! command -v clang >/dev/null 2>&1 && ! xcrun --find clang >/dev/null 2>&1; then
  echo "No C compiler found. Install Xcode or Command Line Tools." >&2
  exit 1
fi

HAS_BREW=0
if command -v brew >/dev/null 2>&1; then
  HAS_BREW=1
  echo "==> Homebrew: $(brew --version | head -1)"
else
  echo "==> Homebrew not found; continuing with existing toolchain."
fi

ensure_brew_pkg() {
  local name="$1"
  if [[ "${HAS_BREW}" -ne 1 ]]; then
    echo "Missing required tool '${name}' and Homebrew is unavailable." >&2
    echo "Install Homebrew from https://brew.sh or install '${name}' manually, then re-run." >&2
    exit 1
  fi
  if brew list "${name}" >/dev/null 2>&1; then
    echo "==> ${name}: already installed"
  else
    echo "==> Installing ${name} via Homebrew..."
    brew install "${name}"
  fi
}

if command -v cmake >/dev/null 2>&1; then
  echo "==> cmake: $(cmake --version | head -1)"
else
  ensure_brew_pkg cmake
fi

CMAKE_VER_LINE="$(cmake --version | head -1)"
CMajor="$(echo "${CMAKE_VER_LINE}" | /usr/bin/awk '{ for(i=1;i<=NF;i++) if($i ~ /^[0-9]+\.[0-9]+/) { split($i,a,"."); print a[1]; exit } }')"
CMinor="$(echo "${CMAKE_VER_LINE}" | /usr/bin/awk '{ for(i=1;i<=NF;i++) if($i ~ /^[0-9]+\.[0-9]+/) { split($i,a,"."); print a[2]; exit } }')"
CMajor="${CMajor:-0}"
CMinor="${CMinor:-0}"
if [[ "${CMajor}" -lt 3 ]] || { [[ "${CMajor}" -eq 3 ]] && [[ "${CMinor}" -lt 22 ]]; }; then
  echo "CMake 3.22+ required. Found: ${CMAKE_VER_LINE}" >&2
  echo "Try: brew upgrade cmake" >&2
  exit 1
fi

if ! command -v git >/dev/null 2>&1; then
  ensure_brew_pkg git
fi

for arg in "$@"; do
  if [[ "${arg}" == "--ninja" ]]; then
    if ! command -v ninja >/dev/null 2>&1; then
      ensure_brew_pkg ninja
    else
      echo "==> ninja: $(ninja --version)"
    fi
    break
  fi
done

echo "==> JUCE at ${JUCE_ROOT}"
if [[ ! -f "${JUCE_ROOT}/CMakeLists.txt" ]]; then
  echo "==> Cloning JUCE (shallow)..."
  mkdir -p "$(dirname "${JUCE_ROOT}")"
  git clone --depth 1 "${JUCE_URL}" "${JUCE_ROOT}"
else
  echo "    JUCE already present."
fi

exec bash "${ROOT_DIR}/scripts/mac-build.sh" "$@"
