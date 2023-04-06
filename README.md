# malloc0: Tiny Memory Allocator for WebAssembly

## Features

- WASM size in few bytes
- very fast allocate and free a block in O(1)
- blocks are aligned to 4 bytes (32-bits)
- zero-cost mass reset using `free_all()` (simulate a reboot)
- mark of `free_all()` can be changed to be after loading some data using `free_all_mark()`
- zero-cost `free_to_mark()` to do mass reset up to a given mark obtained by `free_get_mark()`,
  - get mark into global variable or stack
  - load game level
  - play
  - game over
  - free to mark

## Why?

- I believe WebAssembly in web does not need full WASI nor full C library.
- embedded/IoT/edge
- WASM modules inside Python, ...etc.

allocate, process, free all see this [demo page](https://muayyad-alsadi.github.io/wasm-demos/histogram-matching/index.html)

## How to use it?

include the two files in your project directory and add the `.c` to your list of C files
we have a demo `index.html` that calls `str_toggle.wasm`

```
export CFLAGS="-O3 -flto --target=wasm32 -mbulk-memory --no-standard-libraries -Wl,--export-all -Wl,--no-entry -Wl,--lto-O3 -Wl,-z,stack-size=$[4 * 1024]"
export CFILES="malloc0.c str_toggle.c"
clang $CFLAGS -o str_toggle.wasm $CFILES
```

## How it works

Inspired by [Surma's article](https://surma.dev/things/c-to-webassembly/)
that showed that you can have a very small allocator in WASM.
That allocator did not have functioning `free()`.
I wanted to create an allocator that supports `free()` and that is simple and small with the above features (ex. zero-cost mass reset).


Each allocated block is wrapped like this:

- 4 bytes containing the size
- 4 bytes a flag indicating being used or free
- the actual block
- 4 bytes containing the size

the size before and after the block is not redundant, it serves as pointers in linked lists.

```
[4b:size_1][4b:flag_1][Nb:block_1][4b:size_1] [4b:size_2][4b:flag_2][Nb:block_2][4b:size_2]
                      ^                                             ^
                                  {   previus,    size,      flag,      block  }
```

if we are freing `block_2` we can the address of block_1 by reading `size_1` which is just before it
think of it as previous pointer in linked lists.

when a blocked is freed, its flag is changed to indicate free status,
but it need not be directly usable,
if there is a continuous free part at the end then that free memory is usable

## Happy Scenarios

```
char *ptr1 = malloc(n1);
// do_something1(ptr1);
free(ptr1);
char *ptr2 = malloc(n2);
// do_something2(ptr2);
free(ptr2);
```

```
char *ptr1 = malloc(n1);
char *ptr2 = malloc(n2);
// do_something_with(ptr1, ptr2);
free(ptr2);
free(ptr1);
```

# Suboptimal Scnario - also works fine


```
char *ptr1 = malloc(n1);
char *ptr2 = malloc(n2);
// do_something_with(ptr1, ptr2);
free(ptr1); // <--- free won't be usable at this point
free(ptr2); // <--- both will be usable here
```

