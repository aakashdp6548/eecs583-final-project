#! /bin/bash
sudo apt-get install libomp-dev gcc-offload-nvptx ninja-build libc++-dev
ln -s /usr/local/cuda-11.8/lib64/libcudart.so /usr/local/lib/   # make symlink to libcudart from lib directory

# build llvm with openmp
git clone --depth=1 https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build && cd build
cmake ../llvm/ -DCMAKE_BUILD_TYPE=Release -G Ninja -DLIBOMPTARGET_DEVICE_ARCHITECTURES=sm_75 -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="openmp"
ninja install
clang --version
llvm-config --version