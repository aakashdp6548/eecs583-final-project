set -ex


# compile pass
cd GPUPrefetch/build
cmake ..
make

cd -

# compile to llvm and run optimization pass
clang -emit-llvm -S main.cpp -Xclang -disable-O0-optnone -o main.ll
opt -load-pass-plugin=GPUPrefetch/build/GPUPrefetchPass/GPUPrefetchPass.so -passes='mem2reg,gpu-prefetch-pass' -prefetch-iters=$1 main.ll -S -o main_prefetch.ll

# compile unoptimized and optimized code to executable
clang main.ll -o main
clang main_prefetch.ll -o main_prefetch 
