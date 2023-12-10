#include <stdio.h>
#include <omp.h>

#define N 200000000

int main()
{
  float *a   = (float *)malloc(sizeof(float) * N);
  float *b   = (float *)malloc(sizeof(float) * N);
  float *c   = (float *)malloc(sizeof(float) * N);

  double total_size = 3 * sizeof(float) * N;
  printf("Total Size: %f GB\n", (float)((total_size/1024.0)/1024.0/1024.0));

  double time = -omp_get_wtime();

  #pragma omp target data map(to:a[0:N], b[0:N], c[0:N])
  {
    #pragma omp target teams distribute parallel for 
    for (int i = 0; i < N; i++){
      a[i] = (float)i;
      b[i] = 2.0 * (float)i;
    }

    // add two vectors
    #pragma omp target teams distribute parallel for
    for (int i=0; i<N; i++){
      c[i] = a[i] + b[i];
    }
  }

  time +=  omp_get_wtime();

  free(a);
  free(b);

  printf("Time: %.3fs\n", time);

  return 0;
}