#! /bin/bash
set -ex
cd GPUPrefetch/build
cmake ..
make
cd -