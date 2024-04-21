#!/bin/sh
root_dir="$(dirname $(dirname $(realpath $0)))"
cmake --build $root_dir/out/Release --config Release -j
