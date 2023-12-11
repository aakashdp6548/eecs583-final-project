#include<stdio.h>

int main() {

    unsigned int N = 100;
    float v1[N];
    float v2[N];
    float v3[N];

    // for (int i = 0; i < 100; i = i+5) {
    //     v1.push_back(i);
    //     v2.push_back(i);
    // }

    for (int i = 0; i < N; i++) {
        v3[i] = v1[i] + v2[i];
    }
    printf("done");
    return 0;
