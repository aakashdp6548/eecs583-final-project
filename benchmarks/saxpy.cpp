#include <stdio.h>
#include <stdlib.h>

# define N 1000000000

int main() {
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int a = 1;
    long total = 0;

    double total_size = 2 * sizeof(float) * N;
    printf("Total Size: %f GB\n", (float)((total_size/1024.0)/1024.0/1024.0));

    // fill arrays
    for(long i = 1; i < N; ++i) {
        x[i] = 1;
        y[i] = i;
    }

    // saxpy
    for(long i = 1; i < N; ++i) {
        double temp = a*x[i] + y[i];
        temp = temp / i;
        total += temp;
    }

    printf("%ld\n", total);
    free(x);
    free(y);
}