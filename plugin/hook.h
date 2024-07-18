#ifndef HOOK_H
#define HOOK_H

// #define HOOK(file, function, position, ...) \
//     __attribute__((annotate("hook:" #file #function ":" position ":" #__VA_ARGS__)))

#define HOOK(file, function, ...) __attribute__((hook(file, function, #__VA_ARGS__)))

#endif