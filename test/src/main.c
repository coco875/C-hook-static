#include <stdio.h>
#include "hook.h"

int sub(int a, int b) {
    return a - b;
}

int add(int a, int b) {
    return sub(a, -b);
}

int main() {
    printf("Hello, World!\n");
    return 0;
}