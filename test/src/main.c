#include <stdio.h>

void sub(int a, int b, int *result) {
    result = a - b;
}

int add(int a, int b) {
    int result;
    sub(a, -b, &result);
    return result;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}