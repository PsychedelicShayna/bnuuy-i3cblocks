#ifndef _MARENA_H
#define _MARENA_H

// arena_test.c
// Compile: gcc arena_test.c -o arena_test -Wall -Wextra -O2
// Run: ./arena_test

#include <assert.h>
#include <dirent.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define KiB(n) ((uint64_t)(n) << 10)
#define MiB(n) ((uint64_t)(n) << 20)
#define GiB(n) ((uint64_t)(n) << 30)



static inline bool is_pow2(size_t x)
{
    return x && ((x & (x - 1)) == 0);
}

// (mu)ltiple (g)reater or (e)qual to (pow)er of (2), mugepow :D.. mudge pow?
// mud cow! also otherwise known as alignup_pow2, or align_uintptr, but not
// only is this technically more accurate, but it's also more fun to say (:
static inline uintptr_t align_mugepow2(uintptr_t v, size_t align)
{
    assert(is_pow2(align));
    return (v + (align - 1)) & ~(uintptr_t)(align - 1);
}

static inline bool is_aligned_ptr(void* p, size_t align)
{
    return (((uintptr_t)p) & (align - 1)) == 0;
}

/* * * * * * *** * * * * * * * * * * * * * * * * * * * * *** * * * * * **
 * <3 <3 <3 ***  * Arena Allocator Base Implementation *  *** <3 <3 <3  *
 * * * * * * *** * * * * * * * * * * * * * * * * * * * * *** * * * * * **/

#define ARENA_ALIGN alignof(max_align_t)

#define PUSH_STRUCT(arena, T) \
    (T*)marena_alloc((arena), sizeof(T), alignof(T), false)

#define PUSH_STRUCT_NZ(arena, T) \
    (T*)marena_alloc((arena), sizeof(T), alignof(T), true)

#define PUSH_ARRAY(arena, T, count) \
    (T*)marena_alloc((arena), sizeof(T) * (count), alignof(T), false)

#define PUSH_ARRAY_NZ(arena, T, count) \
    (T*)marena_alloc((arena), sizeof(T) * (count), alignof(T), true)

#define ARENA_MAGIC_ROOT 0xC0DCAFEDEADEBEEFull
// ^ lazy

// Welcome to the DeadBeef Code Cafe! - my signature sentinel~
#define ARENA_MAGIC_ALLOC 0xDEADBEEFC0DECAFEull

/* marena_header and marena -- internal memory layout R=root, A=alloc
 * --------------------------------------------------------------------------
 * Whenever an allocation is made, arena stores a header at the end past the
 * allocated memory block describing the allocation. When no allocations have
 * been made, there is still a header right after the marena struct itself with
 * magic ARENA_MAGIC_ROOT and size 0. This essentially serves as a LIFO linked
 * list of allocations within the arena in order to track them.
 *
 * [marena][Rheader][........................................................]
 * [marena][Rheader][######][Aheader][.......................................]
 * [marena][Rheader][######][Aheader][######][Aheader][......................]
 * [marena][Rheader][######][Aheader][######][Aheader][######][Aheader][.....]
 */

typedef struct marena_header {
    uint64_t  magic;      // ARENA_MAGIC_ROOT or ARENA_MAGIC_ALLOC
    uint64_t  size;       // requested size
    uintptr_t user_start; // absolute address returned to user
    void*     prev_header;
    void*     next_header;
} marena_header;

typedef struct marena {
    size_t         capacity; // user capacity (bytes)
    uintptr_t      limit;    // absolute end = (uintptr_t)marena* + malloc_size
    marena_header* last_alloc_header; // latest header (root initially)
    uint64_t       count;             // outstanding allocations
} marena;

/* Create arena with `capacity` bytes usable by the caller. */
marena* marena_create(uint64_t capacity)
{
    if(capacity == 0) {
        return NULL;
    }

    size_t malloc_size =
      sizeof(marena) + sizeof(marena_header) + capacity + ARENA_ALIGN - 1;

    void* raw = malloc(malloc_size);

    if(!raw) {
        return NULL;
    }

    memset(raw, 0, malloc_size);

    uintptr_t base  = (uintptr_t)raw;
    uintptr_t limit = base + malloc_size;

    marena* a = (marena*)base;

    *a = (marena) {
        .capacity          = capacity,
        .limit             = limit,
        .last_alloc_header = NULL,
        .count             = 0,
    };

    uintptr_t header_addr =
      align_mugepow2(base + sizeof(marena), alignof(marena_header));

    assert(header_addr + sizeof(marena_header) <= limit);

    marena_header* root = (marena_header*)header_addr;

    *root = (marena_header) {
        .magic       = ARENA_MAGIC_ROOT,
        .size        = 0,
        .user_start  = 0,
        .prev_header = NULL,
        .next_header = NULL,
    };

    a->last_alloc_header = root;

    // sanity: region from `root+header` to `limit` must be at least `capacity`
    uintptr_t user_area_start =
      align_mugepow2(header_addr + sizeof(marena_header), ARENA_ALIGN);

    assert(user_area_start + capacity <= limit);

    return a;
}

/* Allocate `new_mem_size` bytes with `alignment` (pow2). If `non_zero`
 * zerofill the returned block. Returns NULL on OOM or invalid params. */
void* marena_alloc(marena* a,
                   size_t  new_mem_size,
                   size_t  alignment,
                   bool    non_zero)
{
    if(!a) {
        fprintf(stderr, "marena_alloc: invalid null arena pointer\n");
        return NULL;
    }

    if(new_mem_size == 0) {
        fprintf(stderr, "marena_alloc: invalid size 0 requested\n");
        return NULL;
    }

    if(alignment == 0 || !is_pow2(alignment)) {
        fprintf(stderr,
                "marena_alloc: invalid alignment %zu (must be pow2 > 0)\n",
                alignment);
        return NULL;
    }

    marena_header* curr = a->last_alloc_header;

    assert(curr != NULL);
    fprintf(stderr, "Magic/Count: %lx, %zu\n", curr->magic, a->count);

    assert((a->count == 0 && curr->magic == ARENA_MAGIC_ROOT) ||
           (a->count >= 1 && curr->magic == ARENA_MAGIC_ALLOC));

    uintptr_t curr_end   = (uintptr_t)curr + sizeof(marena_header);
    uintptr_t user_start = align_mugepow2(curr_end, alignment);

    uintptr_t new_header_addr =
      align_mugepow2(user_start + new_mem_size, alignof(marena_header));

    if(new_header_addr + sizeof(marena_header) > a->limit) {
        fprintf(stderr,
                "marena_alloc: OOM (requested %zu bytes with align %zu)\n",
                new_mem_size,
                alignment);
        return NULL; // OOM
    }

    marena_header* newh = (marena_header*)new_header_addr;

    *newh = (marena_header) {
        .magic       = ARENA_MAGIC_ALLOC,
        .size        = new_mem_size,
        .user_start  = user_start,
        .prev_header = (void*)curr,
        .next_header = NULL,
    };

    curr->next_header    = (void*)newh;
    a->last_alloc_header = newh;
    a->count += 1;

    void* ret = (void*)user_start;

    if(non_zero) {
        memset(ret, 0, new_mem_size);
    }

    /* invariants */
    assert(is_aligned_ptr(ret, alignment));
    assert((uintptr_t)ret + new_mem_size <= (uintptr_t)newh);

    return ret;
}

// --- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

typedef struct marena_part {
    uint8_t* base;     // start of this partition's memory
    size_t   capacity; // total usable bytes
    size_t   offset;   // bump pointer
} marena_part;

static inline void* marena_part_alloc(marena_part* p, size_t size, size_t align)
{
    assert(p);
    assert(align && ((align & (align - 1)) == 0));

    uintptr_t curr    = (uintptr_t)p->base + p->offset;
    uintptr_t aligned = align_mugepow2(curr, align);

    size_t new_offset = (aligned - (uintptr_t)p->base) + size;
    assert(new_offset <= p->capacity && "partition overflow");

    p->offset = new_offset;
    return (void*)aligned;
}

static inline void marena_part_reset(marena_part* p)
{
    assert(p);
    p->offset = 0;
}

void* marena_partition_from(marena*      arena,
                            uint64_t     total_bytes,
                            marena_part* parts,
                            size_t       nparts,
                            size_t       align)
{
    assert(arena);
    assert(parts);
    assert(nparts > 0);

    void* region = marena_alloc(arena, total_bytes, align, false);

    if(!region) {
        return NULL;
    }

    size_t part_size = total_bytes / nparts;
    part_size        = align_mugepow2(part_size, align);

    for(size_t i = 0; i < nparts; ++i) {
        uint8_t* base = (uint8_t*)region + i * part_size;

        parts[i] = (marena_part) {
            .base     = base,
            .capacity = part_size,
            .offset   = 0,
        };
    }

    return region;
}

marena* marena_create_partitioned(uint64_t     total_bytes,
                                  marena_part* parts,
                                  size_t       nparts)
{
    assert(parts);
    assert(nparts > 0);

    // Create parent arena normally
    marena* arena = marena_create(total_bytes);
    if(!arena) {
        return NULL;
    }

    // Allocate one big block from it
    void* region = marena_alloc(arena, total_bytes, ARENA_ALIGN, false);
    assert(region);

    size_t part_size = total_bytes / nparts;
    part_size        = align_mugepow2(part_size, ARENA_ALIGN);

    for(size_t i = 0; i < nparts; ++i) {
        uint8_t* base = (uint8_t*)region + i * part_size;

        parts[i] = (marena_part) {
            .base     = base,
            .capacity = part_size,
            .offset   = 0,
        };
    }

    return arena;
}

bool carve_region_into_parts(marena_part* dst_parts,
                             size_t       npartitions,
                             void*        src_region_ptr,
                             size_t       src_region_size,
                             const size_t requested_sizes[],
                             bool         requested_percentages)
{
    if(!src_region_ptr || src_region_size == 0 || !dst_parts ||
       npartitions == 0) {
        fprintf(stderr,
                "carve_region_into_parts: invalid params\n"
                "  src_region_ptr=%p\n"
                "  src_region_size=%zu\n"
                "  dst_parts=%p\n"
                "  npartitions=%zu\n",
                src_region_ptr,
                src_region_size,
                (void*)dst_parts,
                npartitions);

        return false;
    }

    uintptr_t src_region_addr     = (uintptr_t)src_region_ptr;
    size_t    src_bytes_remaining = src_region_size;

    size_t partition_sizes[npartitions];
    memset(partition_sizes, 0, sizeof(size_t) * npartitions);

    if(requested_percentages) {
        size_t requested_sizes_sz[npartitions];

        /* requested_sizes are percents; must be in 0..100. */
        size_t sm_pct = 0;

        for(size_t i = 0; i < npartitions; ++i) {
            size_t pct = requested_sizes[i];
            if(pct > 100) {
                fprintf(stderr,
                        "carve_region_into_parts: requested percentage %zu "
                        "exceeds 100\n",
                        pct);
                return false;
            }

            requested_sizes_sz[i] =
              (src_region_size * pct) / 100; /* integer truncation */

            sm_pct += pct;
        }
        /* if sum < 100, allocate rest to last partition */
        size_t allocated = 0;
        for(size_t i = 0; i < npartitions; ++i) allocated += requested_sizes[i];
        if(allocated < src_region_size) {
            requested_sizes_sz[npartitions - 1] += src_region_size - allocated;
        }
    } else {
        size_t sum = 0;

        for(size_t i = 0; i < npartitions; ++i) {
            partition_sizes[i] = requested_sizes[i];
            sum += partition_sizes[i];
        }

        if(sum > src_region_size) {
            fprintf(
              stderr,
              "carve_region_into_parts: requested sizes exceed region size\n "
              " sum(requested_sizes) = %zu > src_region_size = %zu\n",
              sum,
              src_region_size);
            return false;
        }

        /* if sum < region_size, give remainder to last partition */
        if(sum < src_region_size) {
            partition_sizes[npartitions - 1] += src_region_size - sum;
        }
    }

    // size_t    remaining_after = src_region_size;

    for(size_t i = 0; i < npartitions; ++i) {
        uintptr_t aligned_part_addr =
          align_mugepow2(src_region_addr, ARENA_ALIGN);

        size_t aligned_part_remainder =
          (size_t)(aligned_part_addr - src_region_addr);

        /* check capacity: gap + target must fit into remaining_after */
        if(partition_sizes[i] + aligned_part_remainder > src_region_size) {
            fprintf(stderr,
                    "carve_region_into_parts: partition %zu cannot fit\n"
                    "  src_bytes_remaining=%zu\n"
                    "  aligned_part_remainder=%zu\n"
                    "  partition_sizes[%zu]=%zu\n",
                    i,
                    src_bytes_remaining,
                    aligned_part_remainder,
                    i,
                    partition_sizes[i]);
            return false; /* can't fit */
        }

        dst_parts[i].base     = (uint8_t*)aligned_part_addr;
        dst_parts[i].capacity = partition_sizes[i];
        dst_parts[i].offset   = 0;

        /* advance cur_ptr and remaining_after */
        src_region_addr = aligned_part_addr + partition_sizes[i];
        src_bytes_remaining -= (aligned_part_remainder + partition_sizes[i]);
    }

    return true;
}

/* Drop 'count' allocations (LIFO). Returns dropped count. */
size_t marena_drop(marena* a, uint64_t count)
{
    if(!a) {
        return 0;
    }
    if(count == 0) {
        return 0;
    }
    if(a->count == 0) {
        return 0;
    }

    size_t dropped = 0;

    while(count-- > 0 && a->count > 0) {
        marena_header* last = a->last_alloc_header;
        assert(last != NULL);

        if(last->magic == ARENA_MAGIC_ROOT) {
            break;
        }

        marena_header* prev = (marena_header*)last->prev_header;

        assert(prev != NULL);
        assert(prev->magic == ARENA_MAGIC_ROOT ||
               prev->magic == ARENA_MAGIC_ALLOC);

        /* unlink */
        prev->next_header    = NULL;
        a->last_alloc_header = prev;
        a->count -= 1;
        dropped += 1;
    }

    printf("Dropped: %zu\n", dropped);

    return dropped;
}

/* Reset to only root header; drops everything. */
void marena_reset(marena* a)
{
    if(!a) {
        return;
    }

    while(a->last_alloc_header &&
          a->last_alloc_header->magic != ARENA_MAGIC_ROOT) {
        marena_drop(a, 1);
    }

    assert(a->last_alloc_header &&
           a->last_alloc_header->magic == ARENA_MAGIC_ROOT);

    assert(a->count == 0);
}

/* Free entire arena */
void marena_free(marena* a_ptr)
{
    if(!a_ptr) {
        fprintf(stderr, "marena_free called on null %p\n", (void*)a_ptr);
        return;
    }

    free(a_ptr);
}

/* ----------------- validation + visualization helpers ---------- */

// Validate header chain and invariants; return true if OK, false otherwise
bool validate_arena(const marena* a)
{
    if(!a) {
        return false;
    }

    if(!a->last_alloc_header) {
        return false;
    }

    // find root by walking prev pointers
    marena_header* h       = a->last_alloc_header;
    size_t         counted = 0;

    while(h->prev_header) {
        marena_header* p = (marena_header*)h->prev_header;
        if(p == NULL) {
            return false;
        }

        if(!(p->magic == ARENA_MAGIC_ALLOC || p->magic == ARENA_MAGIC_ROOT)) {
            return false;
        }

        // next ptr should point back to h unless root's next is first alloc
        if(p->next_header != (void*)h) {
            // it's possible if someone manually corrupted, consider invalid
            return false;
        }

        h = p;

        counted++;
        if(counted > 1000000) {
            return false; // runaway prevention
        }
    }

    // now h is root
    if(h->magic != ARENA_MAGIC_ROOT) {
        return false;
    }

    // walk forward via next_header and check layout constraints
    marena_header* cur            = h;
    uint64_t       observed_count = 0;

    while(cur) {
        // header must be within [raw, limit)
        uintptr_t hdr_addr = (uintptr_t)cur;

        if(hdr_addr < (uintptr_t)a ||
           hdr_addr + sizeof(marena_header) > a->limit) {

            return false;
        }

        if(cur->magic == ARENA_MAGIC_ALLOC) {
            // user_start should be within arena base malloc (a) and limit.
            if(cur->user_start < (uintptr_t)a || cur->user_start >= a->limit) {
                return false;
            }

            if(cur->user_start + cur->size > hdr_addr) {
                return false; /* allocation overruns header */
            }

            observed_count++;
        } else {
            if(cur->magic != ARENA_MAGIC_ROOT) {
                return false;
            }
        }

        if(!cur->next_header) {
            break;
        }

        cur = (marena_header*)(cur->next_header);
    }

    // observed_count should equal a->count
    if(observed_count != a->count) {
        return false;
    }

    return true;
}

/* Print a readable representation of the arena layout */
void print_arena_layout(const marena* a)
{
    if(!a) {
        printf("arena: NULL\n");
        return;
    }

    printf("=== ARENA LAYOUT ===\n");
    printf("arena base (raw) : %p\n", (void*)a);
    printf("arena limit      : %p\n", (void*)a->limit);
    printf("capacity (user)  : %zu bytes\n", a->capacity);
    printf("alloc count      : %llu\n", (unsigned long long)a->count);
    printf("last header addr : %p\n", (void*)a->last_alloc_header);

    /* find root */
    marena_header* h = a->last_alloc_header;
    while(h->prev_header) h = (marena_header*)h->prev_header;
    printf("root header addr : %p\n", (void*)h);

    /* print headers forward */
    marena_header* cur = h;
    int            idx = 0;
    while(cur) {
        uintptr_t ha = (uintptr_t)cur;
        printf("[%2d] header @ %p | magic=0x%016llx | size=%llu | "
               "user_start=%p | prev=%p | next=%p\n",
               idx,
               (void*)ha,
               (unsigned long long)cur->magic,
               (unsigned long long)cur->size,
               (void*)cur->user_start,
               cur->prev_header,
               cur->next_header);

        if(cur->magic == ARENA_MAGIC_ALLOC) {
            /* show a tiny hex dump (first up to 16 bytes) of the user area */
            size_t dump            = (cur->size > 16) ? 16 : (size_t)cur->size;
            const unsigned char* p = (const unsigned char*)(cur->user_start);
            printf("      data (first %zu bytes):", dump);
            for(size_t i = 0; i < dump; ++i) {
                printf(" %02x", p[i]);
            }
            printf("\n");
        }

        if(!cur->next_header)
            break;
        cur = (marena_header*)cur->next_header;
        idx++;
    }

    printf("====================\n");
}

/* ---------- tests ---------- */

#ifdef MARENA_TEST_SUITE
/* A very thorough test that prints state and performs many scenarios. */

int mega_test(void)
{
    printf("\n=== MEGA TEST START ===\n");

    /* Create an arena (2 MiB user capacity) */
    const uint64_t capacity = MiB(2);
    marena*        a        = marena_create(capacity);
    assert(a && "marena_create failed");
    printf("Created arena at %p, capacity %llu\n",
           (void*)a,
           (unsigned long long)capacity);

    /* Basic allocations with different alignments & sizes and capture pointers
     */
    enum { N = 256 };
    // void*  ptrs[N];
    // size_t sizes[N];
    // size_t aligns[N];

    for(size_t i = 0; i < N; ++i) {
        size_t align = 1u << (i % 6); /* 1,2,4,8,16,32 */
        size_t sz    = (i % 127) + 1;
        void*  p     = marena_alloc(a, (uint64_t)sz, align, false);
        if(!p) {
            printf("Allocation %zu failed (OOM). Breaking\n", i);
            // ptrs[i]   = NULL;
            // sizes[i]  = 0;
            // aligns[i] = 0;
            break;
        }
        /* fill a pattern so we can inspect later */
        memset(p, (0xA0 + (i & 0xFF)), sz);
        // ptrs[i]   = p;
        // sizes[i]  = sz;
        // aligns[i] = align;

        /* quick invariants */
        assert(is_aligned_ptr(p, align));
        assert(validate_arena(a));
    }

    print_arena_layout(a);

    /* Drop the last 16 allocations and print layout */
    size_t dropped = marena_drop(a, 16);
    printf("Dropped %zu allocations (last 16). Now count=%llu\n",
           dropped,
           (unsigned long long)a->count);
    print_arena_layout(a);

    /* Re-allocate some and exercise non_zero */
    for(int i = 0; i < 16; ++i) {
        void* p = marena_alloc(a, 64, 16, true);
        assert(p != NULL);
        /* should be zeroed */
        for(int b = 0; b < 64; ++b) {
            assert(((unsigned char*)p)[b] == 0);
        }
    }
    printf("Added 16 zeroed 64-byte allocations (align16)\n");
    print_arena_layout(a);

    /* Edge-case: invalid alignment request */
    void* bad = marena_alloc(a, 32, 3, false);
    assert(bad == NULL);

    /* Edge-case: zero size */
    void* zero = marena_alloc(a, 0, 8, false);
    assert(zero == NULL);

    /* Boundary test: allocate chunks until OOM (but not to crash) */
    size_t succ = 0;
    while(true) {
        void* p = marena_alloc(a, 4096, 8, false);
        if(!p)
            break;
        succ++;
        if(succ > 10000)
            break;
    }
    printf("Allocated %zu more 4KB blocks until OOM at count=%llu\n",
           succ,
           (unsigned long long)a->count);
    print_arena_layout(a);

    /* Reset and confirm clean */
    marena_reset(a);
    assert(a->count == 0);
    printf("After reset: count=%llu\n", (unsigned long long)a->count);
    print_arena_layout(a);

    /* Randomized allocate/drop test with validation after every op */
    srand((unsigned)time(NULL) ^ 0xC0FFEE);

    enum { OPS = 2000 };
    for(int op = 0; op < OPS; ++op) {
        int r = urandint(int, 0, 100);

        if(r < 60) {
            /* allocate */
            // size_t sz        = (rand() % 512) + 1;
            size_t sz = urandint(size_t, 0, 512) + 1;

            size_t align_pow = 1u << (urandint(size_t, 0, 6));
            void*  p         = marena_alloc(a, sz, align_pow, false);
            if(!p) {
                /* OOM is ok; try dropping some then continue */
                marena_drop(a, 1ull + (urandint(size_t, 0, 4)));
            } else {
                /* scribble */

                memset(p, (urandint(unsigned, 0, UINT32_MAX) & 0xFF), sz);
            }
        } else {
            /* drop a small random amount */
            marena_drop(a, 1 + (urandint(size_t, 0, 3)));
        }
        if((op & 127) == 0) {
            /* periodic snapshot & validation */
            bool ok = validate_arena(a);
            printf("[random op %d] validate=%d, count=%llu\n",
                   op,
                   ok,
                   (unsigned long long)a->count);
            if(!ok) {
                printf("Validation failed mid-random test — printing layout\n");
                print_arena_layout(a);
                assert(ok);
            }
        }
    }

    /* Now demonstrate nested arena leak scenario (user mistake):
     * Create a second arena (nested_arena) and store its pointer inside a block
     * allocated from 'a'. Then free 'a' without freeing nested_arena.
     * This simulates user forgetting to free the nested arena: nested_arena
     * remains allocated (a leak) until explicitly freed.
     */
    marena* nested = marena_create(KiB(64));
    assert(nested);
    void* holder = marena_alloc(a, sizeof(marena*), alignof(marena*), false);
    assert(holder);
    memcpy(
      holder, &nested, sizeof(nested)); /* store pointer inside arena memory */
    printf("Created nested arena at %p and stored its pointer into parent "
           "arena memory %p\n",
           (void*)nested,
           holder);
    print_arena_layout(a);

    /* Now free the parent arena (this will free parent->raw but NOT nested) */
    marena_free(a);
    printf("Freed parent arena via marena_free(a). Nested arena still at %p "
           "(leaked if not freed).\n",
           (void*)nested);

    /* To avoid real leak in this test run, free nested now */
    marena_free(nested);
    printf("Freed nested arena now to avoid leak.\n");

    /* Recreate the parent arena to continue more tests (fresh) */
    a = marena_create(capacity);
    assert(a);

    /* Final cleanup */
    marena_free(a);
    printf("Parent arena freed cleanly at end.\n");

    printf("=== MEGA TEST COMPLETE ===\n\n");
    return 0;
}

/* ---------- quick entry-point ---------- */

#endif // MARENA_TEST_SUITE

// Define COMPILED_AS_C_TU
#ifndef COMPILED_AS_C_TU
#define COMPILED_AS_C_TU
#endif

#ifdef MARENA_HASH_TEST
typedef struct {
    pthread_mutex_t lock;
    void*           data;
    size_t          size;
    size_t          sztype;
} mutexwrap;

#include <sha256.h>

#define MeguMiB  MiB
#define Megumins MiB

#include <pthread.h>
#include <stdatomic.h>

typedef struct {
    int             ctr;
    pthread_mutex_t mutex;
} SharedData;

typedef struct {
    pthread_cond_t  notify;
    pthread_mutex_t mutex;
    int             tasks;
    int             shutdown;
    int             active;
    pthread_t       workers[4];

    // File paths threads dump in here when done with a direntry.
    marena_part* files_done;

    // When a thread find a direntry, it adds it to the pool's "todo"
    marena_part* dirs_pending;

} ThreadPool;

ThreadPool pool;

void* worker(void* data)
{
    long tid = *(long*)data;

    while(1) {
        pthread_mutex_lock(&pool.mutex);

        // wait to be woken up and wait for tasks. no task, go back to sleep.
        while(pool.tasks == 0 && pool.shutdown == 0) {
            pthread_cond_wait(&pool.notify, &pool.mutex);
        }

        if(!pool.dirs_pending->offset) {
            continue; // maybe
        }

        if(pool.shutdown) {
            printf("Thread: %ld detected shutdown. bbye\n", tid);
        }

        usleep(1000);
    }

    return NULL;
}

// A simulation of a dynamic workload in which an arena would be used:
// loading a bunch of files in memory into an arena, and hashing them,
// also storing their hash on the arena.
void sha256_threaded_arena_test(marena*      arena,
                                const char** files,
                                size_t       nfiles)
{
    memset(&pool, 0, sizeof(ThreadPool));

    // SharedData data;
    // data.ctr = 0;

    if(pthread_mutex_init(&pool.mutex, NULL) != 0) {
        perror("Mutex xinit failed");
        exit(1);
    }

    for(int i = 0; i < 4; i++) {
        pthread_create(&pool.workers[i], NULL, worker, (void*)&i);
    }

    while(1) {
        pthread_mutex_lock(&pool.mutex);

        pthread_cond_signal(&pool.notify);
        pthread_mutex_unlock(&pool.mutex);

        usleep(4000000);
    }

    for(int i = 0; i < 4; i++) {
        pthread_join(pool.workers[i], NULL);
    }

    printf("Final Counter: %d\n", pool.tasks);
    pthread_mutex_destroy(&pool.mutex);

    marena_part parts[6];
    marena_part p90meg, p5meg, p1meg1, p1meg2, p500k1, p500k2;

    void* megus = marena_alloc(arena, MiB(100), ARENA_ALIGN, false);
    fprintf(
      stdout, "Allocated 100 meg region at %p for partitioning test\n", megus);

    bool carved = carve_region_into_parts(
      parts,
      6,
      megus,
      MeguMiB(100),
      (const size_t[]) {
        MeguMiB(10), MeguMiB(5), MeguMiB(1), MeguMiB(1), KiB(500), KiB(500) },
      false);

    if(!carved) {
        fprintf(stderr, "Failed to carve 100mb region into parts\n");
        return;
    }

    // walker:
    const char* dirpath = ".";
    DIR*        dir     = opendir(dirpath);
    if(!dir) {
        fprintf(stderr, "Failed to open directory %s\n", dirpath);
        return;
    }

    struct dirent* entry;
    size_t         file_count = 0;
    const char*    file_list[32]; // max 32 files for this test

    while((entry = readdir(dir)) != NULL && file_count < 32) {
        if(entry->d_type == DT_REG) { // regular file
            file_list[file_count++] = entry->d_name;
        }
    }

    for(size_t i = 0; i < file_count; ++i) {
        fprintf(stdout, "Found file: %s\n", file_list[i]);
    }

    closedir(dir);
    return;

    // later

    uint8_t** filebufs =
      marena_alloc(arena, sizeof(uint8_t*) * 32, alignof(void*), false);

    uint8_t** hashbufs =
      marena_alloc(arena, 64 * 32, alignof(max_align_t), false);

    for(size_t i = 0; i < nfiles; ++i) {
        FILE* fd = fopen(files[i], "r");
        fseek(fd, 0, SEEK_END);
        long filesize = ftell(fd);
        assert(filesize && filesize > 0 &&
               "Empty file or file size less than 0 maybe err");
        fseek(fd, 0, SEEK_SET);

        uint8_t* file_data_buf =
          marena_alloc(arena, (size_t)filesize, alignof(max_align_t), false);

        fprintf(stdout, "File size: %zu\n", filesize);

        size_t read = fread(file_data_buf, 1, (size_t)filesize, fd);

        fclose(fd);

        if(read) {
            uint8_t digest[SHA256_DIGEST_LENGTH];

            SHA2_CTX ctx = { 0 };
            SHA256Init(&ctx);
            SHA256Update(&ctx, file_data_buf, read);
            SHA256Final(digest, &ctx);

            marena_drop(arena, 1); // drop the file data

            hexdump(digest, SHA256_DIGEST_LENGTH);

        } else {
            fprintf(stdout, "File no read\n");
        }

        return;
    }
}

#endif // MARENA_HASH_TEST

#ifndef COMPILED_AS_C_TU
int main(void)
{

#ifdef MARENA_TEST_SUITE
    puts("Running mega_test...");
    int rc = mega_test();
    printf("mega_test returned %d\n", rc);
    return rc;
#endif // MARENA_TEST_SUITE

    printf("Running ordinary test\n");

    marena* a = marena_create(MiB(101));
    assert(validate_arena(a));
    const char* files[] = { "a.out" };

    // sha256_threaded_arena_test(a, files, 1);

    marena_free(a);
    printf("Exiting with code 0\n");
    return 0;
}
#endif

#endif // _ARENA_H
