#! /bin/bash

# install openmp stuff
sudo apt-get install libomp-dev gcc-offload-nvptx
ln -s /usr/local/cuda-11.8/lib64/libcudart.so /usr/local/lib/   # make symlink to libcudart from lib directory

# build llvm with openmp
git --depth=1 clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build && cd build
cmake ../llvm/ -DCMAKE_BUILD_TYPE=Release -G Ninja -DLIBOMPTARGET_DEVICE_ARCHITECTURES=sm_75 -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="openmp"
ninja install