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

List list_hook = {NULL, NULL, 0};

#define END_OF_WORD(char_) (char_ == ' ' \
    || char_ == '(' \
    || char_ == ')' \
    || char_ == '{' \
    || char_ == '}' \
    || char_ == ',' \
    || char_ == '\n' \
    || char_ == '\0' \
    || char_ == '"' \
    || char_ == ';' \
    || char_ == '\t' \
    || char_ == '*' )

typedef struct {
    char source[CHUNK_SIZE];
    char file[CHUNK_SIZE];
    char function[CHUNK_SIZE];
    char at[CHUNK_SIZE];
    char function_call[CHUNK_SIZE];
} Hook;

void load_list_hook(char * file, char * source) {
    FILE *file_ = fopen(file, "rb");
    if (!file_) {
        return;
    }
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

bool match_file(char * file, char * hook_file) {
    int end_file_char = strlen(file);
    int end_hook_file_char = strlen(hook_file);
    while (file[end_file_char] == hook_file[end_hook_file_char]) {
        end_file_char--;
        end_hook_file_char--;
    }
    return end_hook_file_char == 0;
}

char *get_c_file(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strstr(argv[i], ".c")) {
            return argv[i];
        }
    }
    return NULL;
}

void gen_out_name(char *file_name, char *out_name) {
    char last_char = '\0';
    for (int i = 0; i < strlen(file_name); i++) {
        if (file_name[i] == 'c' && last_char == '.') {
            out_name[i] = 'o';
            out_name[i+1] = 'u';
            out_name[i+2] = 't';
            out_name[i+3] = '.';
            out_name[i+4] = 'c';
            break;
        }
        out_name[i] = file_name[i];
        last_char = file_name[i];
    }
}

int main(int argc, char *argv[]) {
    // char file_name[] = "test/src/main.c";
    char *file_name = get_c_file(argc, argv);
    if (!file_name) {
        printf("No C file found\n");
        return 1;
    }
    FILE *file = fopen(file_name, "r");

    // char file_out_name[] = "test/src/main.out.c";
    char file_out_name[CHUNK_SIZE] = "";
    gen_out_name(file_name, file_out_name);
    FILE *file_out = fopen(file_out_name, "w");

    load_list_hook("hook.bin", file_name);

    char str_[CHUNK_SIZE] = "";
    bool in_quote = false;

    char arg[CHUNK_SIZE] = "";
    int arg_len = 0;

    char full_arg_list[16][CHUNK_SIZE] = {""};
    int full_arg_list_idx = 0;
    int full_arg_list_len = 0;

    int arg_num = 0;
    Hook *hook = malloc(sizeof(Hook));
    strcpy(hook->source, file_name);

    char word_name[CHUNK_SIZE] = "";
    int word_name_len = 0;
    bool end_of_word = false;

    char function_name[CHUNK_SIZE] = "";
    bool in_function = false;

    bool in_hook = false;
    int in_hook_level = 0;
    bool finalise_hook = false;
    for (int size = 0; (size = fread(str_, 1, CHUNK_SIZE, file)) > 0; ) {
        for (int i = 0; i < size; i++) {
            char char_ = str_[i];
            if (char_ == '"') {
                in_quote = !in_quote;
            }

            if (in_quote) {
                fwrite(&char_, 1, 1, file_out);
            }

            if (!in_quote) {
                if (char_ == ')') { // end of function call
                    parenthesis_count--;
                    if (in_hook_level == parenthesis_count && in_hook) {
                        in_hook = false;

                        strcpy(hook->at, arg);
                        for (int i = 0; i < arg_len; i++) {
                            arg[i] = '\0';
                        }
                        arg_len = 0;
                        finalise_hook = true;
                    }
                    if (parenthesis_count == 0) {
                        printf("function name: %s\n", function_name);
                        for (int i = 0; i <= full_arg_list_idx; i++) {
                            printf("Arg: %s\n", full_arg_list[i]);
                        }
                    }
                }
                if (char_ == '}') { // end of code block
                    if (in_function && bracket_count == 1) {
                        in_function = false;
                        for (int i = 0; function_name[i] != '\0'; i++) {
                            function_name[i] = '\0';
                        }
                    }
                    bracket_count--;
                }
            }

            if (in_hook && char_ != ' ' && char_ != '"' && char_ != '\n') { // inside a hook function
                // printf("%c", char_);
                if (char_==',' && !in_quote && parenthesis_count == in_hook_level + 1) {
                    if (arg_num == 0) {
                        // printf("Hook file: %s\n", arg);
                        strcpy(hook->file, arg);
                    } else if (arg_num == 1) {
                        // printf("Hook function: %s\n", arg);
                        strcpy(hook->function, arg);
                    }
                    for (int i = 0; i < arg_len; i++) {
                        arg[i] = '\0';
                    }
                    arg_len = 0;
                    arg_num++;
                } else {
                    arg[arg_len++] = char_;
                }
            }

            
            if (parenthesis_count>0 && !(char_ == ',' && !in_quote && parenthesis_count == 1)) {
                full_arg_list[full_arg_list_idx][full_arg_list_len++] = char_;
            }

            if (char_ == ',' && !in_quote && parenthesis_count == 1) {
                full_arg_list_idx++;
                full_arg_list_len = 0;
            }

            if (!in_quote) {

                if (char_ == '{') { // start of code block
                    if (parenthesis_count == 0 && bracket_count == 0 && function_name[0] != '\0') {
                        printf("Function: %s\n", function_name);
                        bool hook_inserted = false;
                        in_function = true;
                        ListElement *element = list_hook.head;
                        while (element) {
                            Hook *hook = element->data;
                            if (strcmp(hook->function, function_name) == 0 && strcmp(hook->at, "AT(FUNCTION_CALL)") == 0) {
                                if (!hook_inserted) {
                                    fwrite("{\n", 1, 2, file_out);
                                    hook_inserted = true;
                                }
                                printf("Hook found: %s %s %s\n", hook->file, hook->function, hook->at);
                                fprintf(file_out, "    %s();\n", hook->function_call);
                            }
                            element = element->next;
                        }
                        if (hook_inserted) {
                            fprintf(file_out, "    void* result = original_%s();\n", function_name);
                        }
                        ListElement *element_ = list_hook.head;
                        while (element_) {
                            Hook *hook = element_->data;
                            if (strcmp(hook->function, function_name) == 0 && strcmp(hook->at, "AT(FUNCTION_RETURN)") == 0) {
                                if (!hook_inserted) {
                                    fwrite("{\n", 1, 2, file_out);
                                    fprintf(file_out, "    void* result = original_%s();\n", function_name);
                                    hook_inserted = true;
                                }
                                printf("Hook found: %s %s %s\n", hook->file, hook->function, hook->at);
                                fprintf(file_out, "    %s();\n", hook->function_call);
                            }
                            element_ = element_->next;
                        }
                        if (hook_inserted) {
                            fprintf(file_out, "    return result;\n}\ninline void* original_%s() ", function_name);
                        }
                    }
                    bracket_count++;
                }

                if (char_ == '(') { // start of function call
                    if (parenthesis_count == 0) {
                        for (int i = 0; i <= full_arg_list_idx; i++) {
                            int size = 0;
                            while (full_arg_list[i][size] != '\0') {
                                full_arg_list[i][size++] = '\0';
                            }
                        }
                        full_arg_list_len = 0;
                        full_arg_list_idx = 0;
                    }
                    if (parenthesis_count == 0 && bracket_count == 0) {
                        strcpy(function_name, word_name);
                    }
                    if (strcmp(word_name, HOOK_KEYWORD) == 0) {
                        printf("Hook detected\n");
                        hook = malloc(sizeof(Hook));
                        strcpy(hook->source, file_name);
                        in_hook = true;
                        in_hook_level = parenthesis_count;
                    }
                    if (finalise_hook) {
                        strcpy(hook->function_call, word_name);
                        printf("\nHook: %s %s %s %s\n", hook->file, hook->function, hook->at, hook->function_call);
                        list_append(&list_hook, hook);
                        hook = NULL;
                        arg_num = 0;
                        finalise_hook = false;
                    }
                    parenthesis_count++;
                }

                if (END_OF_WORD(char_)) {
                    if (!end_of_word) {
                        fwrite(word_name, 1, word_name_len, file_out);
                    }
                    fwrite(&char_, 1, 1, file_out);
                    end_of_word = true;
                } else {
                    if (end_of_word) {
                        // printf("%s\n", word_name);
                        for (int i = 0; i < word_name_len; i++) {
                            word_name[i] = '\0';
                        }
                        word_name_len = 0;
                        end_of_word = false;
                    }
                    word_name[word_name_len++] = char_;
                }
            }
        }
    }

    save_list_hook("hook.bin");
}