#!/bin/bash -ex

set -o pipefail

# Locate our unicorn installation
export UNICORNDIR="`pwd`/unicorn"

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
