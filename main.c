#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

#define CHUNK_SIZE 1024
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define LEN(arr) ARRAY_COUNT(arr)-1

char HOOK_KEYWORD[] = "HOOK";

int bracket_count = 0;
int parenthesis_count = 0;
int hook_word_len = 0;

List list_hook = {NULL, NULL, 0};

typedef struct {
    char source[CHUNK_SIZE];
    char file[CHUNK_SIZE];
    char function[CHUNK_SIZE];
    char at[CHUNK_SIZE];
} Hook;

void load_list_hook(char * file, char * source) {
    FILE *file_ = fopen(file, "rb");
    Hook hook;
    for (int size = 0; (size = fread(&hook, 1, sizeof(Hook), file_)) > 0; ) {
        if (strcmp(hook.source, source) == 0) {
            continue;
        }
        Hook *hook_ = malloc(sizeof(Hook));
        memcpy(hook_, &hook, sizeof(Hook));
        list_append(&list_hook, hook_);
    }
}

void save_list_hook(char * file) {
    FILE *file_ = fopen(file, "wb");
    ListElement *element = list_hook.head;
    while (element) {
        Hook *hook = element->data;
        fwrite(hook, 1, sizeof(Hook), file_);
        element = element->next;
    }
}

void print_list_hook() {
    ListElement *element = list_hook.head;
    while (element) {
        Hook *hook = element->data;
        printf("Hook: %s %s %s\n", hook->file, hook->function, hook->at);
        element = element->next;
    }
}

int main() {
    char file_name[] = "test/src/main.c";
    FILE *file = fopen(file_name, "r");

    load_list_hook("hook.bin", file_name);

    char str_[CHUNK_SIZE] = "";
    bool in_quote = false;

    char arg[CHUNK_SIZE] = "";
    int arg_len = 0;

    int arg_num = 0;
    Hook *hook = malloc(sizeof(Hook));
    strcpy(hook->source, file_name);

    char function_name[CHUNK_SIZE] = "";
    int function_name_len = 0;
    bool in_hook = false;
    int in_hook_level = 0;
    for (int size = 0; (size = fread(str_, 1, CHUNK_SIZE, file)) > 0; ) {
        for (int i = 0; i < size; i++) {
            char char_ = str_[i];
            if (char_ == '"') {
                in_quote = !in_quote;
            }
            if (char_ == ')' && !in_quote) { // end of function call
                parenthesis_count--;
                if (in_hook_level == parenthesis_count && in_hook) {
                    in_hook = false;

                    strcpy(hook->at, arg);
                    for (int i = 0; i < CHUNK_SIZE; i++) {
                        arg[i] = '\0';
                    }
                    arg_len = 0;
                    hook_word_len = 0;
                    printf("\nHook: %s %s %s\n", hook->file, hook->function, hook->at);
                    list_append(&list_hook, hook);
                    hook = malloc(sizeof(Hook));
                    strcpy(hook->source, file_name);
                }
            }
            if (char_ == '}' && !in_quote) { // end of code block
                bracket_count--;
            }

            if (in_hook && char_ != ' ' && char_ != '"' && char_ != '\n') { // inside a hook function
                printf("%c", char_);
                if (char_==',' && !in_quote) {
                    if (arg_num == 0) {
                        // printf("Hook file: %s\n", arg);
                        strcpy(hook->file, arg);
                    } else if (arg_num == 1) {
                        // printf("Hook function: %s\n", arg);
                        strcpy(hook->function, arg);
                    }
                    for (int i = 0; i < CHUNK_SIZE; i++) {
                        arg[i] = '\0';
                    }
                    arg_len = 0;
                    arg_num++;
                } else {
                    arg[arg_len++] = char_;
                }
            }
            
            if (bracket_count > 0) { // inside a function
                printf("%c", char_);
            }
            if (char_ == '{' && !in_quote) { // start of code block
                bracket_count++;
            }

            if (char_ == '(' && !in_quote) { // start of function call
                // printf("%s\n", function_name);
                if (strcmp(function_name, HOOK_KEYWORD) == 0) {
                    printf("Hook detected\n");
                    in_hook = true;
                    in_hook_level = parenthesis_count;
                }
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    function_name[i] = '\0';
                }
                function_name_len = 0;
                parenthesis_count++;
            }

            function_name[function_name_len++] = char_;

            if (char_ == ' ' || char_ == '\n') {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    function_name[i] = '\0';
                }
                function_name_len = 0;
            }
        }
    }

    save_list_hook("hook.bin");
}