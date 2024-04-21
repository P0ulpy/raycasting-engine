#!/bin/sh
# Init vcpkg submodule
git submodule sync --recursive
git submodule update --init --recursive --remote --force

# Full path to root directory
root_dir="$(dirname $(dirname $(realpath $0)))"
$root_dir/vcpkg/bootstrap-vcpkg.sh -disableMetrics