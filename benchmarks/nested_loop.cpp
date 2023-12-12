#include <stdio.h>
#include <stdlib.h>

# define N 10000

int main() {
    int *A = (int*)malloc(sizeof(int) * N * N);
    int *x = (int*)malloc(sizeof(int) * N);

    // fill arrays
    for (long i = 0; i < N; ++i) {
        for (long j = 0; j < N; ++j) {
          A[i*N + j] = 1;
        }
        x[i] = i;
    }

    // x^tAx
    double total = 0;
    for (long i = 0; i < N; ++i) {
        double sum = 0;
        for (long j = 0; j < N; ++j) {
           sum += A[i*N + j] * x[j];  // &x + i*M + j
        }
        total += sum * x[i];  // &y + i
    }
    printf("Total: %f\n", total);  // 24502500 for N = 100
}