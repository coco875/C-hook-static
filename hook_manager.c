#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define STR_SIZE 1024

typedef struct hook {
    // char file[STR_SIZE];
    // char function[STR_SIZE];
    struct hook *next;
    char source[STR_SIZE];
    char at[STR_SIZE];
    bool cancelable;
    int priority;
    char function_call[STR_SIZE];
    int num_args;
} Hook;

typedef struct hook_function {
    struct hook_function *next;
    Hook hook;
} HookFunction;

typedef struct hook_file {
    struct hook_file *next;
    char *path;
    struct HookFunction *functions;
} HookFile;

HookFile *list_hook = NULL;

inline void _add_hook(Hook hook, Hook newHook) {
    Hook *currentHook = &hook;
    while (currentHook->next != NULL) {
        currentHook = currentHook->next;
    }
    currentHook->next = &newHook;
}

inline void _add_hook_function(HookFile *hookFile, char *functionName, Hook hook) {
    HookFunction *hookFunction = (HookFunction *) hookFile->functions;
    while (hookFunction->next != NULL) {
        if (strcmp(hookFunction->hook.source, functionName) == 0) {
            _add_hook(hookFunction->hook, hook);
            return;
        }
        hookFunction = hookFunction->next;
    }
    HookFunction *newHookFunction = malloc(sizeof(HookFunction));
    hookFunction->next = newHookFunction;
    _add_hook(newHookFunction->hook, hook);
}

void add_hook(char *file, char *functionName, Hook hook) {
    HookFile *hookFile = list_hook;
    while (hookFile->next != NULL) {
        if (strcmp(hookFile->path, file) == 0) {
            _add_hook_function(hookFile, functionName, hook);
            return;
        }
        hookFile = hookFile->next;
    }
    HookFile *newHookFile = malloc(sizeof( HookFile));
    newHookFile->path = file;
    hookFile->next = newHookFile;
    _add_hook_function(newHookFile, functionName, hook);
}