#!/bin/sh
# Full path to root directory
root_dir="$(dirname $(dirname $(realpath $0)))"
cmake --build $root_dir/out/Debug --config Debug -j
