#include<stdio.h>

int main() {

    int N = 100000;
    int v1[N], v2[N], v3[N];

    for (int i = 0; i < N; i++) {
        v3[i] = v1[i] + v2[i];

        = v1[i + 1] 
    }

    printf("done\n");
    return 0;
}

/*
%25 = getelementptr i32, ptr %11, %24
%prefetch = getelementptr i32, ptr %11, %24 + iter_ahead * memsize
call void @llvm.prefetch.p0(ptr %prefetch, 0, 3, 1)
%26 = load i32, ptr %25, align 4


*/