// clang -O3 -flto --target=wasm32 -mbulk-memory --no-standard-libraries -Wl,--export-all -Wl,--no-entry -Wl,--lto-O3 -Wl,-z,stack-size=$[40 * 1024] -o str_toggle.wasm str_toggle.c malloc0.c
// wasm-objdump -x str_toggle.wasm 
#include "malloc0.h"

__attribute__((import_module("env"), import_name("log_i"))) void log_i(int i);
// __attribute__((import_module("env"), import_name("log_s"))) void log_s(int i, char *ptr);

extern size_t __heap_tail;
extern size_t __heap_mark;

char *str_toggle(int n, char *ptr) {
    char *dst=malloc(n+1);
    char ch;
    char x = 'a' ^ 'A';
    int i;
    for(i=0; i<n; ++i) {
        ch = *(ptr + i);
        if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z')) {
            ch ^= x; 
        }
        *(dst + i) = ch;
    }
    return dst;
}

