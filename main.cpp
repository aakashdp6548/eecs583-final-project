#include <stdio.h>
#include <stdlib.h>

# define N 100000000

int main() {
    int *a = (int*)malloc(N * sizeof(int));
    int total = 0;
    for(long int i = 1; i < N; i+=1) {
        a[i] = 1;
    }

    for(long int i = 1; i < N; i+=1) {
        total += a[i];
    }

    printf("%d\n", total);
    free(a);
}