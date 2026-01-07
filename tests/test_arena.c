#include "arena.h"
#include <stddef.h>

void print_marena_header(marena_header* header)
{
    // uint8_t* current = ((uint8_t*)arena + arena->offset -
    // sizeof(marena_header));

    // marena_header* header = arena->last_alloc_header;

    // marena_header* header =
    //   (marena_header*)alignup((uintptr_t)current, alignof(max_align_t));

    printf("Marena Header at %p\n", (void*)header);
    printf("  Magic: %lx\n", header->magic);
    printf("  Size: %zu\n", header->size);
    printf("  Prev: %p\n", (void*)header->prev_header);
    printf("  Next: %p\n", (void*)header->next_header);
}

void print_marena(marena* arena)
{
    printf("Marena at %p\n", (void*)arena);
    printf("  Capacity: %zu\n", arena->capacity);
    printf("  Offset: %zu\n", arena->offset);
    printf("  Count: %zu\n", arena->count);
}

int main(void)
{
    marena* arena = marena_create(MiB(1));

    printf("Created\n");
    print_marena(arena);
    void*          mem = marena_alloc(arena, 4096, alignof(max_align_t), true);
    /* hexdump */(mem, 4096);
    marena_header* hmem1 = arena->last_alloc_header;

    printf("Allocated\n");

    void* mem2 = marena_alloc(arena, 4096, alignof(max_align_t), false);
    marena_header* hmem2 = arena->last_alloc_header;
    printf("Allocated2\n");

    void* mem3 = marena_alloc(arena, 4096, alignof(max_align_t), false);
    marena_header* hmem3 = arena->last_alloc_header;
    printf("Allocated3\n");

    marena_header* hmem4 = arena->last_alloc_header;
    void* mem4 = marena_alloc(arena, 4096, alignof(max_align_t), false);
    printf("Allocated3\n");

    print_marena_header(hmem1);
    print_marena_header(hmem2);
    print_marena_header(hmem3);
    print_marena_header(hmem4);
    print_marena(arena);
    marena_drop(arena, 1);
    marena_drop(arena, 1);
    printf("DROPPED 1------------------------------\n");
    print_marena(arena);
    marena_drop_all(arena);
    printf("DROPPED ALL------------------------------\n");

    print_marena(arena);
    print_marena_header(hmem3);
    print_marena_header(hmem4);
    //
    // marena_drop(arena, 1);
    // print_marena(arena);
    // print_marena_header(hmem3);
    // print_marena_header(hmem4);

    marena_free(&arena);
    printf("FREE ARENA------------------------------\n");

    if(arena == NULL) {
        printf("Arena freed successfully, %p\n", (void*)arena);
    } else {
        printf("Arena free failed, %p\n", (void*)arena);
    }

    printf("Done testing arena\n");
    return 0;
}
