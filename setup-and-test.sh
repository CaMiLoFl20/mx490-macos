#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
work_dir=${MX490_WORK_DIR:-"$project_dir/work"}
source_dir="$work_dir/AirSane"
build_dir="$work_dir/AirSane-build"

command -v brew >/dev/null 2>&1 || {
  echo "Homebrew is required: https://brew.sh/" >&2
  exit 1
}

brew install sane-backends cmake pkgconf

if [ ! -d "$source_dir/.git" ]; then
  mkdir -p "$work_dir"
  git clone https://github.com/SimulPiscator/AirSane.git "$source_dir"
fi

if git -C "$source_dir" apply --check "$project_dir/airsane-mx490.patch"; then
  git -C "$source_dir" apply "$project_dir/airsane-mx490.patch"
elif git -C "$source_dir" apply --reverse --check "$project_dir/airsane-mx490.patch"; then
  echo "AirSane patch is already applied."
else
  echo "AirSane has local changes that conflict with the MX490 patch." >&2
  exit 1
fi

cmake -S "$source_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --parallel

echo "Build complete: $build_dir/airsaned"
echo "Start AirSane, then verify Image Capture lists Canon MX490 series (AirScan)."
