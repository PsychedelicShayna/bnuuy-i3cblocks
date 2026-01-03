#ifndef _ARENA_H
#define _ARENA_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "common.h"

typedef int8_t  bool8_t;
typedef int32_t bool32_t;

#define KiB(n) ((uint64_t)(n) << 10)
#define MiB(n) ((uint64_t)(n) << 20)
#define GiB(n) ((uint64_t)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ARENA_BASE_POS (sizeof(marena))
// #define ARENA_ALIGN    (sizeof(void*))

#define ARENA_ALIGN alignof(max_align_t)

#define marlloc_array(arena, T) \
    (T*)marlloc((arena), sizeof(T), alignof(T), false)
#define marlloc_arraynz(arena, T) \
    (T*)marlloc((arena), sizeof(T), alignof(T), true)

#define marlloc_struct(arena, T, count) \
    (T*)marlloc((arena), sizeof(T) * (count), alignof(T), false)

#define marlloc_structnz(arena, T, count) \
    (T*)marlloc((arena), sizeof(T) * (count), alignof(T), true)

#define SWAP_PTRS(a, b)       \
    {                         \
        wchar_t* temp = a;    \
        a             = b;    \
        b             = temp; \
    }

struct marena_header;

/* The region header should be placed at the end of each allocated region not
 * the start. It describes what type of allocation is responsible for it such
 * as whether it's part of a partition, a regular marlloc, or the base malloc
 * initially performed when using marmalloc and its variants -- determined by
 * what the `magic` number is. `size` is the amount of bytes managed by the
 * region, and with that, the previous region address can be calculated via
 * `(marena_header*)((uint8_t*)header - header->size)` but `prev` exists
 * so you don't have to do that, and makes freeing various regins at once
 * much easier. Likewise, `next` points to the next region, and is initially
 * `NULL` until a region is marllocated, at which point, the alloation updates
 * the header at: `*(marena_header*)((uint8_t*)marena+marena->offset)` BEFORE
 * ever increments `marena->offset` and then writes the new one to the end of
 * aligned `marena->offset + (newsize+sizeof(marena_header))`
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct {
    uint64_t magic; // Magic comes first. Deliberate. Rest doesn't matter.
                    // Too much to explain just trust your past self on this.

    uint64_t size;
    void*    prev_header;
    void*    next_header;
} marena_header;

// Maybe every time we marrloc we push a small header before the returned
// pointer that tracks the size of the allocation and a magic number for
// validation. Then we can walk backwards from offset to count active
// allocations.
//
// We could also maintain a linked list of active allocations, but that
// would add overhead to each allocation and deallocation. The header
// approach seems more efficient for this use case.
// A: Sure, in the linked list approach, each allocation would have a header
// that contains a pointer to the next allocation in the list. When an
// allocation is made, it would be added to the front of the list, and when
// it is dropped, it would be removed from the list. This would allow us to
// easily track active allocations, but it would add overhead to each
// allocation and deallocation, as we would need to update the pointers in
// the list.
//
// I like that.
// marlloc_array: knows what the value of offset is and therefore the offset of
// the previous allocation, and stores it, before incrementing offset to
// allocate the new data, and it can then place that previous offset pointer in
// the header of the next allocation. so basically since offset is the end of
// the last allocation, and the start of the next allocation, we just leave data
// there for the next call to marlloc_array to use in order to maintain the of
// allocations together and possibly even update the marena struct with
// metadata or a pointer to

// Memory Arena Allopcator
// A simple linear allocator that allows fast allocations and deallocations
// in a contiguous block of memory. Allocations are done in a stack-like
// fashion, and deallocations can only be done in reverse order of
// allocations. This makes it very fast for scenarios where you need to
// allocate and deallocate many small objects quickly. My implementation
// also supports partitioning memory from the arena into multiple sub-arenas
// that can be used independently but share the same underlying memory block
// without additional malloc calls, preventing scenarios where multiple
// arenas would fragment memory, or where you would need to malloc multiple
// times to create multiple arenas simply due to arena working like a stack.
typedef struct marena {
    // The total number of bytes this arena manages excluding the size of the
    // marena struct itself and any alignment padding or header data.
    size_t capacity;

    // The raw base memory returned by malloc which should be equal to the
    // address of any instance of this structure create with marmalloc()
    void* raw;

    // Relative offset in bytes from the start of the marena struct to the
    // next free byte in the arena, i.e., the "offset" of the arena. May not
    // be aligned. Use ALIGN_UP_POW2 after doing arithmetic with this value.
    uintptr_t offset;

    marena_header* last_alloc_header;

    // uintptr_t position; // This is raw+offset

    // The number of undropped allocations still living in the arena.
    uint64_t count;
} marena;

marena* marena_create(uint64_t capacity);

void* mar_alloc(marena*  arena,
                uint64_t size,
                size_t   alignment,
                bool32_t non_zero);

size_t marena_drop(marena* arena, uint64_t count);
void   marena_drop_all(marena* arena);
void   marena_truncate(marena* arena, uint64_t pos);
void   marena_free(marena** arena);

marena* armalloc_parted(uint64_t nbytes, marena** parts, size_t nparts);

marena* marlloc_partition(marena*  mar,
                          uint64_t nbytes,
                          marena** parts,
                          size_t   nparts);

// For the base of the memory arena, which is handled specially. Only used
// initially per arena when calling marmalloc (including partitions).
#define MARMAGIC_ROOT 0xC0DCAFEDEADEBEEF

// All continuations from the base, i.e. marlloc and its variants.
#define MARMAGIC_CONT 0xDEADBEEFC0DECAFE

// (RESIST THE URGE TO MAKE THIS VARIADIC; TOO IMPORTANT TO BE IMPLICIT)
// clang-format off
// In case the macros change, I don't have to replace every instance of
// a marena_header throughout the entire code. I can just change what magic
// defaults to for different allocation types  ___v_v_v_v___ right there.
#define MARMAGIC_ROOT_HEADER (marena_header) { MARMAGIC_ROOT, 0, NULL, NULL }
#define MARMAGIC_CONT_HEADER (marena_header) { MARMAGIC_CONT, 0, NULL, NULL }
// clang-format on
//
//
// Base address is aligned to alignof(max_align_t) (or the maximum you need).
// Offsets (offset, offset, …) are stored as size_t/uintptr_t, not as
// pointers. Every allocation calls align_up(current_address,
// alignof(requested_type)) before bumping the offset. All casts from uint8_t*
// (or void*) to a struct pointer happen after the address has been aligned.
// Allocation size includes possible padding: you never rely on “adding
// sizeof(header) to the malloc size”; you add ALIGN-1 once and round the
// base. Freeing: keep the original pointer returned by malloc (the one before
// you rounded the base) so you can free() it correctly.

// x + (a-1) & ~(uptr)(a-1), a := alignment, x:= uintptr_t

// static void* arena_alloc(marena* a, size_t sz, size_t align)
// {
//     uintptr_t cur = (uintptr_t)a + a->offset; /* current top */
//     uintptr_t nxt = alignup(cur, align) + sz; /* where the object would end
//     */
//
//     if(nxt - (uintptr_t)a > a->capacity)
//         return NULL; /* out of space */
//
//     a->offset = nxt - (uintptr_t)a; /* bump the offset */
//     return (void*)(cur + (alignup(cur, align) - cur)); /* aligned start
//     address */
// }

/* Example usage */
// marena_header* next_hdr =
// arena_alloc(mar, sizeof(marena_header), alignof(marena_header));
// if(next_hdr) {
//     next_hdr->magic = MARMAGIC_ROOT;
//     /* … */
// }

marena* marena_create(uint64_t capacity)
{
    size_t malloc_size =
      (sizeof(marena) +        // Enough for the marena struct itself
       sizeof(marena_header) + // Enough for the first header following it
       capacity +              // Enough for the requested capacity
       ARENA_ALIGN - 1         // And alignment padding at worst case
      );

    assert(malloc_size >= capacity);

    void* data_ptr = malloc(malloc_size);
    assert(data_ptr != NULL);

    // uintptr_t data_addr = align_mugepow2((uintptr_t)data_ptr,
    // alignof(marena));
    // ^
    // This is pointless, malloc garuantees that the address is aligned
    // to at least alignof(max_align_t), which is >= alignof(marena)
    // so we can just use data_ptr as is.

    uintptr_t data_addr = (uintptr_t)data_ptr;

    // We now have an aligned address to place the marena struct. We cast the
    // aligned address to a marena* pointer to write the struct data.
    marena* arena = (marena*)data_addr;

    *arena = (marena) {
        .capacity = capacity,
        .offset   = 0, // this is where the header will go.
        .count = 0, // the number of un-dropped allocations/calls to (mar_alloc)
        .raw   = data_ptr,
    };

    // Sanity check: pointer values should be identical to malloc return value
    assert(arena == data_ptr && arena->raw == data_ptr);
    // Sanity check 2: same but for contents of memory. Just making sure (:
    assert(!memcmp(arena, data_ptr, sizeof(marena)));

    // Now we want to set up the first header right after the marena struct.
    // Its position should be = aligned(marena + sizeof(marena))
    uintptr_t header_addr =
      align_mugepow2(data_addr + sizeof(marena), alignof(marena_header));

    marena_header* header = (marena_header*)header_addr;

    *header = (marena_header) {
        .magic       = MARMAGIC_ROOT,
        .size        = 0,
        .prev_header = NULL,
        .next_header = NULL,
    };

    arena->offset            = header_addr - (uintptr_t)arena;
    arena->last_alloc_header = header;

    return arena;
}

void* marena_alloc(marena*  arena,
                   uint64_t new_mem_size,
                   size_t   alignment,
                   bool32_t non_zero)
{
    assert(arena != NULL && "marena_alloc: arena is NULL");
    assert(new_mem_size > 0 && "marena_alloc: size is zero");
    assert(alignment > 0 && "marena_alloc: alignment is zero");
    assert((alignment & (alignment - 1)) == 0 &&
           "marena_alloc: alignment is not power of 2");

    marena_header* curr_alloc_header = arena->last_alloc_header;

    assert((arena->count == 0 && curr_alloc_header->magic == MARMAGIC_ROOT) ||
           (arena->count >= 1 && curr_alloc_header->magic == MARMAGIC_CONT));

    uintptr_t new_mem_start_aligned = align_mugepow2(
      (uintptr_t)curr_alloc_header + sizeof(marena_header), alignment);

    uintptr_t new_header_addr = align_mugepow2(
      new_mem_start_aligned + new_mem_size, sizeof(marena_header));

    uintptr_t arena_end = (uintptr_t)arena + sizeof(marena) + arena->capacity;

    assert(new_header_addr + sizeof(marena_header) <= arena_end &&
           "marllock: arena offset exceeds capacity");

    marena_header new_header =
      (marena_header) { .magic       = MARMAGIC_CONT,
                        .size        = new_mem_size,
                        .prev_header = (void*)curr_alloc_header,
                        .next_header = NULL };

    marena_header* new_header_ptr = (marena_header*)new_header_addr;
    memcpy(new_header_ptr, &new_header, sizeof(marena_header));

    curr_alloc_header->next_header = (void*)new_header_addr;
    arena->offset                  = new_header_addr - (uintptr_t)arena;
    arena->count += 1;

    arena->last_alloc_header = new_header_ptr;

    void* new_mem_ptr = (void*)new_mem_start_aligned;

    if(!non_zero) {
        memset(new_mem_ptr, 0, new_mem_size);
    }

    return (void*)new_mem_start_aligned;

    /*
```hy
        (let [req 2048 a 1024]
             (print
        )


```
2048 1024 0 24 1024
2048 1024 0 24 25
2048 1024 0 24
2048 1024 24

    */
}
// This is useful for dropping memory mainly. Since it can find out wher the
// start of everything was.
// if(prev_header->magic == MARMAGIC_ROOT && prev_header->prev == mar) {
// }

// uint64_t old_aligned = ALIGN_UP_POW2(mar->offset, ARENA_ALIGN);
// uint64_t new_aligned = old_aligned + size + sizeof(marena_header);
//
// uint8_t* region_start = ((uint8_t*)mar->offset + old_aligned);
//
// // marena_header header =
// // marena_header(size, (void*)old_aligned, (void*)new_aligned);
//
// if(new_aligned > mar->capacity) {
//     return NULL;
// }
//
// mar->offset = new_aligned;
// void* ptr = (uint8_t*)mar + old_aligned;
//
// if(!non_zero) {
//     memset(ptr, 0, size);
// }
//
// return ptr;

// Drops `count` allocations from the arena in LIFO order, regarldesso of
// what they are or how big they are. It walks the linked list of
// allocations using the headers stored at the end of each allocation to
// find the previous allocation and updates the arena's offset accordingly.
// Returns the number of elements encountered and dropped.
size_t marena_drop(marena* arena, uint64_t count)
{
    // size = MIN(size, arena->offset - ARENA_BASE_POS);
    // arena->offset -= size;

    size_t deallocs = 0;

    if(arena->count <= 0 || arena->offset == ARENA_BASE_POS) {
        fprintf(stderr, "mardrop (WARN): nothing to drop!\n");
        return deallocs;
    }

    for(uint64_t i = 0; i < count; ++i) {
        if(arena->offset <= ARENA_BASE_POS || arena->count == 0) {
            // No more allocations to drop
            break;
        }

        marena_header* current_header = arena->last_alloc_header;

        assert(current_header != NULL);

        assert(current_header->magic == MARMAGIC_ROOT ||
               current_header->magic == MARMAGIC_CONT &&
                 "marena_drop: invalid magic number in current header");

        fprintf(stderr,
                "current header prev: %p\n",
                (void*)current_header->prev_header);
        // fprintf(stderr, "current header next: %p\n",
        //         (void*)current_header->next_header);

        fprintf(stderr, "current header at %p\n", (void*)current_header);
        fprintf(stderr,
                "arena offset calc %p\n, ",
                (void*)((uintptr_t)arena + arena->offset));
        assert(current_header ==
                 (marena_header*)((uintptr_t)arena + arena->offset) &&
               "marena_drop: current header does not match arena offset");

        // This shouldn't trigger. If the count is 0 then duh nothing was ever
        // allocated.
        if((current_header->magic == MARMAGIC_ROOT ||
            current_header->prev_header == arena) &&
           arena->count > 0) {
            // Reached the base of the arena
            fprintf(stderr,
                    "mardrop (WARN): Reached base of arena at pos %lu but this "
                    "is bubious, we should have broken on a previous "
                    "iteration.\n",
                    arena->offset);

            arena->offset = ARENA_BASE_POS;
            break;
        }

        // There is no case where prev is null. The only sensible condition
        // would be if we've never allocated anything, but even the 0th
        // alloc has a header with the MARMAGIC_ROOT magic number set and a
        // prev that points to the `marena*`
        assert(current_header->prev_header != NULL &&
               "marena_drop: previous header in current is NULL");

        uintptr_t prev_header_addr = (uintptr_t)current_header->prev_header;
        marena_header* prev_header = (marena_header*)prev_header_addr;

        assert(prev_header != NULL && "marena_drop: previous header is NULL");

        assert(prev_header->magic == MARMAGIC_CONT ||
               prev_header->magic == MARMAGIC_ROOT);

        assert(prev_header_addr >= (uintptr_t)arena + sizeof(marena));

        // This is the last allocated element.
        if(prev_header->magic == MARMAGIC_ROOT) {
            arena->offset = prev_header_addr - (uintptr_t)arena;
            arena->count--;
            deallocs++;
            break;
        }

        arena->last_alloc_header = prev_header;

        fprintf(stderr,
                "set arena offset to %p\nbasd on prev header at %p -  %zu\n",
                (void*)(prev_header_addr - prev_header->size),
                (void*)prev_header,
                prev_header->size);

        arena->offset = prev_header_addr - (uintptr_t)arena;
        arena->count--;
        deallocs++;
    }

    return deallocs;
}

void marena_drop_all(marena* arena)
{
    assert(arena != NULL && "marena_dropaall: arena is NULL");
    marena_drop(arena, arena->count);
}


void marena_free(marena** arena)
{
    if(arena != NULL && *arena != NULL) {
        marena* a = *arena;
        memset(a, 0, a->capacity + sizeof(marena));
        free(a);
        *arena = NULL;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * Partition Type Allocation
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * *
 */

// #define ARENA_ENABLE_PARTITIONS
#ifdef ARENA_ENABLE_PARTITIONS

void* MaPartition();
void* MaCreate_partitioned();

marena* armalloc_parted(uint64_t nbytes, marena** parts, size_t nparts)

{
    if(nparts == 0 || parts == NULL) {
        return NULL;
    }

    uintptr_t offset = align_mugepow2(nbytes / nparts, ARENA_ALIGN);
    marena*   arena  = (marena*)malloc(ARENA_BASE_POS + offset * nparts);

    if(!arena) {
        return NULL;
    }

    arena->capacity = align_mugepow2(offset * nparts, ARENA_ALIGN);
    arena->offset   = ARENA_BASE_POS;

    for(uint64_t i = 0; i < nparts; ++i) {
        // If we just use arena + offset * i then first iteration will just
        // be the address of arena because offset * 0 (i) = 0; we don't want
        // to overwritee arena->nbytes and arena->pos.

        uint8_t* location_in_arena =
          ((uint8_t*)arena + ARENA_BASE_POS) + offset * i;

        marena* partition =
          (marena*)align_mugepow2(location_in_arena, ARENA_ALIGN);

        parts[i]           = partition;
        parts[i]->capacity = offset;
        parts[i]->offset   = ARENA_BASE_POS;
    }

    return arena;
}

marena*
  marlloc_partition(marena* mar, uint64_t nbytes, marena** parts, size_t nparts)
{
    // Already aligned
    void* region_base = mar_alloc(mar, nbytes, 8, false);

    // Need to align mem+partsize though bc offset might not be pow2

    // Approximate size of each partition if the requested `size` is evenly
    // distributed across `num_partitions`
    size_t partition_size = nbytes / nparts;

    if(!region_base) {
        return NULL;
    }

    for(uint64_t i = 0; i < nparts; i++) {
        // If we don't add 1 to i, then offset * i will be 0 for the first
        // iter which means arena + 0, so it will point to the start of the
        // arena.
        uint8_t* loc = (uint8_t*)region_base + (partition_size * i);

        parts[i] = (marena*)ALIGN_UP_POW2(loc, ARENA_ALIGN);

        parts[i]->capacity = partition_size;
        parts[i]->offset   = ARENA_BASE_POS;
    }

    return region_base;
}

#endif

#ifdef SCRATCH

typedef struct {
    marena*  arena;
    uint64_t start_pos;
} marena_temp;

marena_temp marmalloc_scratch(marena** conflicts, uint32_t nconflicts);
void        marfree_scratch(marena_temp scratch);

marena_temp marmalloc_temp(marena* arena);
void        marfree_temp(marena_temp arena);

static __thread marena* _scratch_arena[2] = { NULL, NULL };
static __thread marena* _global_arena     = NULL;

marena_temp marmalloc_scratch(marena** conflicts, uint32_t num_conflicts)
{
    int32_t scratch_index = -1;
    for(int32_t i = 0; i < 2; ++i) {
        bool32_t conflict_found = false;

        for(uint32_t j = 0; j < num_conflicts; ++j) {
            if(_scratch_arena[i] == conflicts[j]) {
                conflict_found = true;
                break;
            }
        }

        if(!conflict_found) {
            scratch_index = i;
            break;
        }
    }

    if(scratch_index == -1) {
        return (marena_temp) { 0 };
    }

    marena** selected = &_scratch_arena[scratch_index];

    if(*selected == NULL) {
        *selected = marmalloc(MiB(64));
    }

    return marmalloc_temp(*selected);
}

// Truncate the

// Create a new temporary arena from the existing arena.
marena_temp marmalloc_temp(marena* arena)
{
    return (marena_temp) { .arena = arena, .start_pos = arena->offset };
}

// Return the temporary arena's base arena to its previous stat by
// truncating to the size before the temporary arena.
void marfree_temp(marena_temp temp)
{
    martrunc(temp.arena, temp.start_pos);

    void marfree_scratch(marena_temp scratch)
    {
        marfree_temp(scratch);
    }
// testing autodoc
#endif

#endif // !_ARENA_H
