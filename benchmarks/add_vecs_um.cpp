#pragma omp requires unified_shared_memory

#include <stdio.h>
#include <omp.h>
#include <cuda_runtime_api.h>

#define N 400000000

int main()
{
  float *a, *b, *c;
  cudaMallocManaged((void**)&a, N * sizeof(float));
  cudaMallocManaged((void**)&b, N * sizeof(float));
  cudaMallocManaged((void**)&c, N * sizeof(float));

  double total_size = 3 * sizeof(float) * N;
  printf("Total Size: %f GB\n", (float)((total_size/1024.0)/1024.0/1024.0));

  double time = -omp_get_wtime();

  #pragma omp target teams distribute parallel for 
  for (int i = 0; i < N; i++){
    a[i] = (float)i;
    b[i] = 2.0 * (float)i;
  }

  // add two vectors
  // prefetch()
  #pragma omp target teams distribute parallel for
  for (int i=0; i<N; i++){
    c[i] = a[i] + b[i];
  }

  time +=  omp_get_wtime();

  cudaFree(a);
  cudaFree(b);
  cudaFree(c);

  printf("Time: %.3fs\n", time);

  return 0;
}