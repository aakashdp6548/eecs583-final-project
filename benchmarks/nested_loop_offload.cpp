#include <stdio.h>
#include <stdlib.h>

# define N 30000

int main() {
    double *A = (double*)malloc(sizeof(double) * N * N);
    double *x = (double*)malloc(sizeof(double) * N);

    double total_size = sizeof(double) * N * N + sizeof(double) * N;
    printf("Total Size: %f GB\n", total_size/1024.0/1024.0/1024.0);

    double total = 0;

    # pragma omp target data map(tofrom:A[0:N*N], x[0:N])
    # pragma omp target data map(tofrom:total)
    {
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
            }
            total += sum * x[i];  // &y + i
        }
    }

    printf("Total: %f\n", total);  // 24502500 for N = 100
}