// #include <stdio.h>
#include "hook.h"

void hook_sub_call(int);
void hook_sub_return(int);

__attribute__((print_func)) int sub(int a) {
    hook_sub_call(a);
    hook_sub_return(a);
    return a;
}

#define TRUE 1

HOOK("src/main.c", sub, AT(FUNCTION_RETURN))
void hook_sub_return(int ret) {
}

HOOK("src/main.c", sub, AT(FUNCTION_CALL))
void hook_sub_call(int a) {
}

__attribute__((print_func)) int main() {
    int a = 1;
    int b = 2;
    sub(a);
    if (a == sub(a)) {
        sub(b);
        return TRUE;
    }
    return TRUE;
}