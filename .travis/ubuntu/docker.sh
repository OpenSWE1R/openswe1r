#!/bin/bash -ex

cd /openswe1r

apt-get update
apt-get install -y build-essential libsdl2-dev libopenal-dev libenet-dev libglew-dev wget git

# Get a recent version of CMake
wget -nv https://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.sh
echo y | sh cmake-3.9.0-Linux-x86_64.sh --prefix=cmake
export PATH=/openswe1r/cmake/cmake-3.9.0-Linux-x86_64/bin:$PATH

mkdir build && cd build

# Install unicorn from source
mkdir unicorn
cd unicorn
wget -nv https://github.com/unicorn-engine/unicorn/archive/1.0.1.tar.gz
tar xf 1.0.1.tar.gz --strip-components=1
UNICORN_ARCHS="x86" ./make.sh
export UNICORNDIR="`pwd`"
cd ..

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
