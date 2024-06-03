#include <stdio.h>
#include <hook.h>
#include "src/main.h"

#define TRUE 1

HOOK("src/main.c", sub, AT(FUNCTION_RETURN))
void hook_sub_return(int ret) {
    printf("hook_sub_return: %d\n", ret);
}

HOOK("src/main.c", sub, AT(FUNCTION_CALL))
void hook_sub_call(int a) {
    printf("hook_sub_call: %d\n", a);
}


HOOK("src/main.c", add, AT(FUNCTION_RETURN))
void hook_add_return(int ret) {
    printf("hook_add_return: %d\n", ret);
}

HOOK("src/main.c", add, AT(FUNCTION_CALL))
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}

HOOK("src/main.c", add, AT(FUNCTION_RETURN, "src/main.c", sub))
int hook_return_sub_printf(int ret) {
    printf("hook_return_sub_printf: %d\n", ret);
    return 0;
}

HOOK("src/main.c", add, AT(FUNCTION_CALL, "src/main.c", sub), TRUE)
void hook_sub_call_cancellable(int a) {
    printf("hook_sub_call: %d\n", a);
}