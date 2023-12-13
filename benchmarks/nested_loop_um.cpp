#pragma omp requires unified_shared_memory

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <cuda_runtime_api.h>

# define N 30000

int main() {
    double *A, *x;
    cudaMallocManaged((void**)&A, sizeof(double) * N * N);
    cudaMallocManaged((void**)&x, sizeof(double) * N);

    double total_size = sizeof(double) * N * N + sizeof(double) * N;
    printf("Total Size: %f GB\n", total_size/1024.0/1024.0/1024.0);

    double total = 0;

    // fill arrays
    #pragma omp target teams distribute parallel for
    for (long i = 0; i < N; ++i) {
        for (long j = 0; j < N; ++j) {
          A[i*N + j] = 0.01;
        }
        x[i] = 1;
    }

    // x^tAx
    #pragma omp target teams distribute parallel for
    for (long i = 0; i < N; ++i) {
        double sum = 0;
        for (long j = 0; j < N; ++j) {
          sum += A[i*N + j] * x[j];  // &x + i*M + j
          sum += A[i*N + j] * x[j]; 
        }
        total += sum * x[i];  // &y + i
    }

    printf("Total: %f\n", total);  // 24502500 for N = 100
}