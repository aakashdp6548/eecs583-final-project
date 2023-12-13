#! /bin/bash
BENCHMARK=$1
ITERS_AHEAD="${2:-1}"

set -ex

# generate unoptimized IR
clang++ -emit-llvm -fopenmp -fopenmp-targets=nvptx64 -Xclang -disable-O0-optnone \
    benchmarks/$BENCHMARK.cpp -S -o $BENCHMARK.ll

# run pass on IR
opt -load-pass-plugin=GPUPrefetch/build/GPUPrefetchPass/GPUPrefetchPass.so \
    -passes="mem2reg,gpu-prefetch-pass" -prefetch-iters=$ITERS_AHEAD \
    $BENCHMARK.ll -S -o "$BENCHMARK"_prefetch.ll

# compile unoptimized IR
clang -fopenmp -fopenmp-targets=nvptx64 -lcudart -L/usr/local/bin/cuda/lib64 \
    $BENCHMARK.ll -o $BENCHMARK

# compile optimized IR
clang -fopenmp -fopenmp-targets=nvptx64 -lcudart -L/usr/local/bin/cuda/lib64 \
    "$BENCHMARK"_prefetch.ll -o "$BENCHMARK"_prefetch