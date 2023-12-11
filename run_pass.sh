set -ex


# compile pass
cd GPUPrefetch/build
cmake ..
make

cd -

# compile to llvm and run optimization pass
clang -emit-llvm -S main.cpp -Xclang -disable-O0-optnone -o main_unoptimized.ll
opt -load-pass-plugin=GPUPrefetch/build/GPUPrefetchPass/GPUPrefetchPass.so -passes='gpu-prefetch-pass' main_unoptimized.ll -S -o main_optimized.ll

# compile unoptimized and optimized code to executable
clang main_unoptimized.ll -o main_unoptimized
clang main_optimized.ll -o main_optimized 
