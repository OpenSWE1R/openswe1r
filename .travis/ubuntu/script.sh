#!/bin/bash -ex

docker run -v $(pwd):/openswe1r ubuntu:16.04 /bin/bash -ex /openswe1r/.travis/ubuntu/docker.sh
