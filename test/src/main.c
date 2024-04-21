#include <stdio.h>
#include "hook.h"

HOOK("main.c", add, AT(FUNCTION_CALL))
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}

int add(int a, int b) {
    return a + b;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}