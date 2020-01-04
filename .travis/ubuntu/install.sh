#!/bin/bash -ex

sudo apt-get update
sudo apt-get -y install build-essential libsdl2-dev libopenal-dev libenet-dev libglew-dev wget git

# Install unicorn from source
mkdir unicorn
cd unicorn
wget -nv https://github.com/unicorn-engine/unicorn/archive/1.0.1.tar.gz
tar xf 1.0.1.tar.gz --strip-components=1
UNICORN_ARCHS="x86" ./make.sh
cd ..
