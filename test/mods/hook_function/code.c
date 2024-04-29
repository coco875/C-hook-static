#include <hook.h>
#include "main.h"

#define TRUE 1

HOOK("src/main.c", add, AT(FUNCTION_RETURN))
int hook_add_return(int ret) {
    return ret + 1;
}

HOOK("src/main.c", add, AT(FUNCTION_CALL))
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}

HOOK("src/main.c", add, AT(FUNCTION_RETURN, "src/main.c", sub))
int hook_return_main_printf(int ret) {
    printf("hook_return_main_printf: %d\n", ret);
    return 0;
}

HOOK("src/main.c", add, AT(FUNCTION_CALL, "src/main.c", sub), TRUE)
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}