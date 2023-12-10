#include <stdio.h>
#include <omp.h>

# define N 1500000000

int main() {
  float *x = (float*) malloc(N * sizeof(float));
  float *y = (float*) malloc(N * sizeof(float));
  float a = 1;

  double total_size = 2 * sizeof(float) * N + sizeof(float);
  printf("Total Size: %f GB\n", (float)((total_size/1024.0)/1024.0/1024.0));

  for (long i = 0; i < N; ++i) {
    y[i] = a * x[i] + y[i];
  }

  free(x);
  free(y);
}