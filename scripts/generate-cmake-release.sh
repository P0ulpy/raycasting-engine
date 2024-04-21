#!/bin/sh
root_dir="$(dirname $(dirname $(realpath $0)))"
cmake -S $root_dir/ -B $root_dir/out/Release -DCMAKE_BUILD_TYPE=Release
