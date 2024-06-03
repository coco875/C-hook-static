#include <stdio.h>

void sub(int a, int b, int *result) {
    *result = a - b;
    printf("sub: %d\n", a);
}

int add(int a, int b) {
    int result;
    sub(a, -b, &result);
    printf("add: %d\n", a);
    return result;
}

int main() {
    printf("Hello, World!\n");
    add(1, 2);
    return 0;
}