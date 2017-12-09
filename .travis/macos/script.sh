#!/bin/bash -ex

set -o pipefail

export MACOSX_DEPLOYMENT_TARGET=10.9

mkdir build && cd build
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;x86_64h" -DCMAKE_BUILD_TYPE=Release
make -j4
