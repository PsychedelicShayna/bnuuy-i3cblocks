#ifndef _ARENA_H
#define _ARENA_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "common.h"

// (mu)ltiple (g)reater or (e)qual to (pow)er of (2), mugepow :D.. mudge pow?
// mud cow! also otherwise known as alignup_pow2, or align_uintptr, but not
// only is this technically more accurate, but it's also more fun to say (:
static inline uintptr_t align_mugepow2(uintptr_t v, size_t align)
{
    assert(is_pow2(align));
    return (v + (align - 1)) & ~(uintptr_t)(align - 1);
}

#define KiB(n) ((uint64_t)(n) << 10)
#define MiB(n) ((uint64_t)(n) << 20)
#define GiB(n) ((uint64_t)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ARENA_BASE_POS (sizeof(marena))
#define ARENA_ALIGN    alignof(max_align_t)

#define marena_alloc_array(arena, T) \
    (T*)arena_alloc((arena), sizeof(T), alignof(T), false)

#define marena_alloc_array_nz(arena, T) \
    (T*)arena_alloc((arena), sizeof(T), alignof(T), true)

#define marena_alloc_struct(arena, T, count) \
    (T*)arena_alloc((arena), sizeof(T) * (count), alignof(T), false)

#define marena_alloc_struct_nz(arena, T, count) \
    (T*)arena_alloc((arena), sizeof(T) * (count), alignof(T), true)

#define SWAP_PTRS(a, b)       \
    {                         \
        wchar_t* temp = a;    \
        a             = b;    \
        b             = temp; \
    }

/* marena_header
 * --------------------------------
 * Whenever an allocation is made, arena stores a header at the end past the
 * allocated memory block describing the allocation. When no allocations have
 * been made, there is still a header right after the marena struct itself with
 * magic MARMAGIC_ROOT and size 0. This essentially serves as a linked list of
 * allocations within the arena in order to track them.
 *
 * [marena][Rheader][........................................................]
 * [marena][Rheader][######][Aheader][.......................................]
 * [marena][Rheader][######][Aheader][######][Aheader][......................]
 * [marena][Rheader][######][Aheader][######][Aheader][######][Aheader][.....]
 */

#define MARMAGIC_ROOT  0xC0DCAFEDEADEBEEF // initial base header
#define MARMAGIC_ALLOC 0xDEADBEEFC0DECAFE // subsequent allocations

typedef struct {
    uint64_t magic;
    uint64_t size;

    // prev/next marena_header*'s in linked list. next determined when next
    // marena_alloc call is made, last_alloc_header is accessed and next is
    // set to the newest marena_header*. also updated by marena_drop.
    void* prev_header; // NULL if root/initial base header.
    void* next_header; // NULL if latest.
} marena_header;

typedef struct marena {
    size_t         capacity; // totla size of arena given in marena_create
    void*          raw;      // initil pointer received by malloc, untouched.
    uintptr_t      offset;   // nbytes past initial marena_header (might remove)
    marena_header* last_alloc_header; // latest header in linked list
    uint64_t       count; // corresponds +1 marena_alloc, -1 marena_drop
} marena;

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
                   bool     non_zero)
{
    assert(arena != NULL && "marena_alloc: arena is NULL");
    assert(new_mem_size > 0 && "marena_alloc: size is zero");
    assert(alignment > 0 && "marena_alloc: alignment is zero");
    assert((alignment & (alignment - 1)) == 0 &&
           "marena_alloc: alignment is not power of 2");

    marena_header* curr_alloc_header = arena->last_alloc_header;

    assert((arena->count == 0 && curr_alloc_header->magic == MARMAGIC_ROOT) ||
           (arena->count >= 1 && curr_alloc_header->magic == MARMAGIC_ALLOC));

    uintptr_t new_mem_start_aligned = align_mugepow2(
      (uintptr_t)curr_alloc_header + sizeof(marena_header), alignment);

    uintptr_t new_header_addr = align_mugepow2(
      new_mem_start_aligned + new_mem_size, sizeof(marena_header));

    uintptr_t arena_end = (uintptr_t)arena + sizeof(marena) + arena->capacity;

    assert(new_header_addr + sizeof(marena_header) <= arena_end &&
           "marllock: arena offset exceeds capacity");

    marena_header new_header =
      (marena_header) { .magic       = MARMAGIC_ALLOC,
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
}

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
               current_header->magic == MARMAGIC_ALLOC &&
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

        assert(prev_header->magic == MARMAGIC_ALLOC ||
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
