#include "malloc0.h"

// used to debuyg like this log_i(1111);log_i(i);log_i(2222);log_i(j);
// __attribute__((import_module("env"), import_name("log_i"))) void log_i(int i);

extern unsigned char __heap_base;
size_t __heap_tail = &__heap_base;
size_t __heap_mark = &__heap_base;

size_t memory_size(void) {
  return __builtin_wasm_memory_size(0);
}

size_t memory_grow(int delta) {
    return __builtin_wasm_memory_grow(0, delta);
}

void free_all() {
  __heap_tail = __heap_mark;
}

void free_all_mark() {
  __heap_mark = __heap_tail;
}

void* free_get_mark() {
  return (void *)__heap_tail;
}

void free_to_mark(void *mark) {
  __heap_tail = (size_t)mark;
}


void* malloc(size_t n) {
  // size is marked at both ends to construct a pointer of previous block
  // [4b:size][4b:flag][n:allocated][4b:size]

  n += (8 - (n % _mem_alignment)) % _mem_alignment;
  // check if size is enough
  size_t total = __heap_tail + n + 3*_size_t_bytes;
  size_t size = memory_size()<<16;
  if (total > size) {
    memory_grow( (total>>16)-(size>>16)+1 );
  }
  unsigned int r = __heap_tail;
  *((size_t *)r)=n;
  r+= _size_t_bytes;
  *((size_t *)r)=_mem_flag_used;
  r+= _size_t_bytes;
  __heap_tail = r + n;
  *((size_t *)__heap_tail) = n;
  __heap_tail += _size_t_bytes;
  
  return (void *)r;
}

void free(void* p) {
  
  size_t n;
  // null case
  if (!p) return;
  size_t r=(size_t)p;
  r-=_size_t_bytes;
  // already free
  if (*((size_t *)r) != _mem_flag_used) {
    return;
  }
  // mark it as free
  size_t flag = _mem_flag_free;
  *((size_t *)r) = flag;
  // calc ptr_tail
  r-=_size_t_bytes;
  n = *(size_t *)r; // size of current block
  size_t ptr_tail = ((size_t)p)+n+_size_t_bytes;
  // if not at tail return without moving __heap_tail
  if (__heap_tail != ptr_tail) {
    return;
  }
  __heap_tail = r;
  while(r > (size_t)&__heap_base) {
    r-=_size_t_bytes;
    n = *(size_t *)r; // size of previous block
    r-=n;
    r-=_size_t_bytes;
    flag = *((size_t *)r);
    if (flag != _mem_flag_free) break;
    r-=_size_t_bytes;
    n = *(size_t *)r; // size of current block
    __heap_tail = r;
  }

}


