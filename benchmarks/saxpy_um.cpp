#pragma omp requires unified_shared_memory

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <cuda_runtime_api.h>

# define a 1
# define N 300000000

int main() {
    int *x, *y;
    cudaMallocManaged((void**)&x, N * sizeof(int));
    cudaMallocManaged((void**)&y, N * sizeof(int));
    long total = 0;

    double total_size = 2 * sizeof(float) * N;
    printf("Total Size: %f GB\n", (float)((total_size/1024.0)/1024.0/1024.0));

    // fill arrays
    #pragma omp target teams distribute parallel for
    for(long i = 1; i < N; ++i) {
        x[i] = 1;
        y[i] = i;
    }

    // saxpy
    #pragma omp target teams distribute parallel for
    for(long i = 1; i < N; ++i) {
        double temp = a*x[i] + y[i];
        temp = temp / i;
        total += temp;
    }

    // printf("Total: %ld\n", total);
    cudaFree(x);
    cudaFree(y);
}