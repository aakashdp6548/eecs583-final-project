sudo apt-get install libomp-dev gcc-offload-nvptx
ln -s /usr/local/cuda-11.8/lib64/libcudart.so /usr/local/lib/   # make symlink to libcudart from lib directory

cmake -S llvm -B build -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_LINKER=gold -DLLVM_ENABLE_RUNTIMES="openmp" -DLLVM_TARGETS_TO_BUILD="NVPTX"