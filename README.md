# Software Prefetch for Accelerating GPU Programs
Code for EECS 583 F23 final project.

## Usage:
Directory structure:
```
root
  benchmarks/
  cfgs/
  build_pass.sh
  install.sh
  run_benchmarks.sh
```
1. From the root directory, run `install.sh` to install dependencies and LLVM with the OpenMP runtime enabled. Be aware this will take a while (4-6 hours).

2. Run `build_pass.sh` to compile the `GPUPrefetch/GPUPrefetchPass/Pass.cpp`.

3. Run `run_benchmark.sh <BENCHMARK> <PREFETCH_DISTANCE>` where `BENCHMARK` is the name of a benchmark in the `benchmarks` directory and `PREFETCH_DISTANCE` is the number of iterations ahead to prefetch. For example,
```
sh ./run_benchmark.sh saxpy_offload 4
```

4. The `BENCHMARK` and `BENCHMARK_prefetch` executables are ready to be run.
