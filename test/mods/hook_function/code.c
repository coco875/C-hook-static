#include <hook.h>
#include "main.h"

HOOK("main.c", add, AT(FUNCTION_RETURN))
int hook_add_return(int ret) {
    return ret + 1;
}

HOOK("main.c", add, AT(FUNCTION_CALL))
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}

HOOK("main.c", main, AT(FUNCTION_RETURN, printf))
int hook_return_main_printf(int ret) {
    printf("hook_return_main_printf: %d\n", ret);
    return 0;
}

HOOK("main.c", main, AT(FUNCTION_CALL, printf))
void hook_add_call(int a) {
    printf("hook_add_call: %d\n", a);
}