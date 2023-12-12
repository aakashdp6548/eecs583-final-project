set -ex

# compile pass
cd GPUPrefetch/build
cmake ..
make
cd -

BENCHMARK=$1
NUM_ITERS="${2:-1}"

BASE="$BENCHMARK"
OPT="$BENCHMARK"_prefetch

# compile to llvm and run optimization pass
clang -emit-llvm -S benchmarks/$BENCHMARK.cpp -Xclang -disable-O0-optnone -o $BENCHMARK.ll
opt -load-pass-plugin=GPUPrefetch/build/GPUPrefetchPass/GPUPrefetchPass.so \
    -passes='mem2reg,gpu-prefetch-pass' \
    -prefetch-iters=$NUM_ITERS "$BENCHMARK".ll \
    -S -o "$BENCHMARK"_prefetch.ll

# compile unoptimized and optimized code to executable
clang $BENCHMARK.ll -o $BENCHMARK
clang "$BENCHMARK"_prefetch.ll -o "$BENCHMARK"_prefetch 
