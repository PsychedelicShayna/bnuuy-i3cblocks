#include <locale.h>
#include <math.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "arena.h"
#include "common.h"
#include "i3bar.h"

marena* GArena = NULL;
marena* gmar1  = NULL;
marena* gmar2  = NULL;
marena* gmar3  = NULL;
marena* gmar4  = NULL;

// test the partitiooned arena function

int main1(void)
{
    setlocale(LC_ALL, "");
    const uint64_t nparts = 4;
    marena*        parts[nparts];

    // In theory across 4 partitions 1 MiB should be enough for 256 KiB each
    GArena = marmalloc_partition(MiB(1), parts, nparts);

    if(!GArena) {
        fprintf(stderr, "Failed to create partitioned arena\n");
        return EXIT_FAILURE;
    }

    gmar1 = parts[0];
    gmar2 = parts[1];
    gmar3 = parts[2];
    gmar4 = parts[3];

    wprintf(L"-1-------------------------\n");
    wprintf(
      L"Cap1: %zu, Pos1: %zu\n", gmar1->capacity - gmar1->head, gmar1->head);
    wprintf(
      L"Cap2: %zu, Pos2: %zu\n", gmar2->capacity - gmar2->head, gmar2->head);
    wprintf(
      L"Cap3: %zu, Pos3: %zu\n", gmar3->capacity - gmar3->head, gmar3->head);
    wprintf(
      L"Cap4: %zu, Pos4: %zu\n", gmar4->capacity - gmar4->head, gmar4->head);
    wprintf(L"---------------------------\n");

    float* floats[10];

    for(int i = 0; i < 10; ++i) {
        floats[i]  = marlloc(gmar1, sizeof(float), 8, false);
        *floats[i] = (float)i + 1;
    }

    i3bar_block_t* b = marlloc_array(gmar2, i3bar_block_t);
    b->full_text     = L"Hello thisis a i3baerr bbloick truct";

    wprintf(L"%ls\n", b->full_text);

    for(size_t i = 0; i < 10; ++i) {
        wprintf(L"Float %zu: %f\n", i, floats[i][0]);
    }

    fflush(stdout);

    wprintf(L"-2-------------------------\n");
    wprintf(
      L"Cap1: %zu, Pos1: %zu\n", gmar1->capacity - gmar1->head, gmar1->head);
    wprintf(
      L"Cap3: %zu, Pos2: %zu\n", gmar2->capacity - gmar2->head, gmar2->head);
    wprintf(
      L"Cap5: %zu, Pos3: %zu\n", gmar3->capacity - gmar3->head, gmar3->head);
    wprintf(
      L"Cap7: %zu, Pos4: %zu\n", gmar4->capacity - gmar4->head, gmar4->head);
    wprintf(L"---------------------------\n");

    martrunc(gmar1, ARENA_BASE_POS);

    wprintf(L"%ls\n", b->full_text);

    fflush(stdout);
    wprintf(L"-3-------------------------\n");
    wprintf(
      L"Cap1: %zu, Pos1: %zu\n", gmar1->capacity - gmar1->head, gmar1->head);
    wprintf(
      L"Cap3: %zu, Pos2: %zu\n", gmar2->capacity - gmar2->head, gmar2->head);
    wprintf(
      L"Cap5: %zu, Pos3: %zu\n", gmar3->capacity - gmar3->head, gmar3->head);
    wprintf(
      L"Cap7: %zu, Pos4: %zu\n", gmar4->capacity - gmar4->head, gmar4->head);
    wprintf(L"---------------------------\n");

    mardrop(gmar2, sizeof(i3bar_block_t));

    wprintf(L"-4-------------------------\n");
    wprintf(
      L"Cap1: %zu, Pos1: %zu\n", gmar1->capacity - gmar1->head, gmar1->head);
    wprintf(
      L"Cap3: %zu, Pos2: %zu\n", gmar2->capacity - gmar2->head, gmar2->head);
    wprintf(
      L"Cap5: %zu, Pos3: %zu\n", gmar3->capacity - gmar3->head, gmar3->head);
    wprintf(
      L"Cap7: %zu, Pos4: %zu\n", gmar4->capacity - gmar4->head, gmar4->head);
    wprintf(L"---------------------------\n");

    fflush(stdout);
    // for(uint64_t i = 0; i < partition_count; ++i) {
    //     void* ptr = arena_push(partitions[i], KiB(256), false);
    //     if(!ptr) {
    //         fprintf(stderr, "Failed to push to partition %llu\n", i);
    //         arena_destroy(GArena);
    //         return EXIT_FAILURE;
    //     }
    //     printf("Pushed 256 KiB to partition %llu at address %p\n", i, ptr);
    // }

    // marfree(GArena);
    return EXIT_SUCCESS;
}

#define dbg(a)                                              \
    printf("Arena: %zu, %zu, (%p:%p): pos %zu / cap %zu\n", \
           a->head,                                         \
           a->capacity,                                     \
           (void*)a,                                        \
           (void*)((uint8_t*)a + a->head),                  \
           a->head,                                         \
           a->capacity);

int main3(void)
{
    return 0;
    marena* arena = marmalloc(KiB(4));
    printf("sizeof arena: %zu\n", sizeof(marena));
    dbg(arena);

    marlloc(arena, 512, 8, false);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    mardrop(arena, 512);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    mardrop(arena, 512);
    dbg(arena);
    mardrop(arena, 512);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    marlloc(arena, 512, 8, false);
    dbg(arena);
    mardrop(arena, 512 * 2);
    mardrop(arena, 580 * 2);
    dbg(arena);
    marlloc(arena, 2048, 8, false);
    dbg(arena);
    marlloc(arena, 2048, 8, false);
    marlloc(arena, 1000, 8, false);
    marlloc(arena, 1000, 8, false);
    marlloc(arena, 1000, 8, false);
    marlloc(arena, 1000, 8, false);
    marlloc(arena, 1000, 8, false);
    marlloc(arena, 1000, 8, false);

    for(int i = 0; i < 10000; ++i) {
        marlloc(arena, 1, 8, false);
    }
    dbg(arena);

    marfree(arena);
    return EXIT_SUCCESS;
}

void print_marena_header(marena* arena)
{
    uint8_t* current = ((uint8_t*)arena + arena->head - sizeof(marena_header));

    marena_header* header =
      (marena_header*)alignup((uintptr_t)current, alignof(max_align_t));

    printf("Marena Header at %p\n", (void*)header);
    printf("  Magic: %lx\n", header->magic);
    printf("  Size: %zu\n", header->size);
    printf("  Prev: %p\n", (void*)header->prev);
    printf("  Next: %p\n", (void*)header->next);
}

void print_marena(marena* arena)
{
    printf("Marena at %p\n", (void*)arena);
    printf("  Capacity: %zu\n", arena->capacity);
    printf("  Head: %zu\n", arena->head);
    printf("  Count: %zu\n", arena->count);
}

void inspect_head(marena* arena)
{
    uint8_t* current = ((uint8_t*)arena + arena->head);

    marena_header* header =
      (marena_header*)alignup((uintptr_t)current, alignof(marena_header));

    if(header->magic != MARMAGIC_ROOT) {
        printf("Invalid magic number in arena header: %lx\n", header->magic);
        print_marena_header(arena);

        hexdump((uint8_t*)arena + arena->head-128, 64);
        exit(EXIT_FAILURE);
    } else {
        printf("Valid magic number in arena header: %lx\n", header->magic);
        print_marena_header(arena);
    }


    printf("-------------------------\n");
}

int main(void)
{

    marena* arena = marmalloc(512);

    printf("Allocarted: %p\n", (void*)arena);

    print_marena(arena);
    inspect_head(arena);

    // 64 -> 152, this should align to 160?
    // that's 88 bytes, and yet 24+32(header)+64 = 120 so where'd the extra
    // bytes come from?
    marlloc(arena, 24, 8, false);

    print_marena(arena);
    inspect_head(arena);

    marfree(arena);

    return 0;

    // Check the header
    //
    uint8_t* current = ((uint8_t*)arena + arena->head - sizeof(marena_header));

    marena_header* header =
      (marena_header*)alignup((uintptr_t)current, alignof(marena_header));

    if(header->magic != MARMAGIC_ROOT) {
        printf("Invalid magic number in arena header: %lx\n", header->magic);
        return EXIT_FAILURE;
    } else {
        printf("Valid magic number in arena header: %lx\n", header->magic);
        print_marena_header(arena);
    }

    marlloc(arena, 24, alignof(max_align_t), false);

    return 0;
    // unsigned index = randint(unsigned, 0, 100);

    for(int i = 0; i < 10; ++i) {
        void* ptr = marlloc(arena, 1024, alignof(max_align_t), false);
        printf("Allocated 1024 bytes at %p and filled it with random data.\n",
               ptr);

        print_marena(arena);
        print_marena_header(arena);
    }

    return 0;
    size_t heads = 0;

    while(heads < 250) {
        if(coinflip()) {
            heads++;
            size_t rand_allocs = urandint(size_t, 1, 100);

            for(int i = 0; i < rand_allocs; i++) {
                size_t rand_size = urandint(size_t, 1, 4096);
                void*  ptr =
                  marlloc(arena, rand_size, alignof(max_align_t), false);
                printf("Allocated %zu bytes at %p.\n", rand_size, ptr);
                // urandom(ptr, rand_size);
                printf("Filled with %zu bytes of random data.\n", rand_size);
                print_marena(arena);
                print_marena_header(arena);
            }

            float flipweight = 0;

            float f1 = (float)coinflip(), f2 = (float)coinflip(),
                  f3 = (float)coinflip(), f4 = (float)coinflip();

            heads += (size_t)(f1 + f2 + f3 + f4);

            flipweight += 0.25f * (float)(rand_allocs / 100) * f1;
            flipweight += 0.50f * (float)(rand_allocs / 100) * f2;
            flipweight += 0.75f * (float)(rand_allocs / 100) * f3;
            flipweight += 1.00f * (float)(rand_allocs / 100) * f4;

            if(coinflip() && coinflip() && coinflip()) {
                unsigned how_many =
                  (unsigned)(urandfloat(float,
                                        0.0f,
                                        (flipweight * 100.0f) *
                                          (float)arena->count / 100.0f)) -
                  1;

                for(unsigned i = 0; i < how_many; ++i) {
                    size_t dropped = mardrop(arena, 1);
                    printf("Dropped %zu bytes from arena.\n", dropped);
                    print_marena(arena);
                    print_marena_header(arena);
                }
            }
        }
    }

    for(int i = 0; i < 5; ++i) {
        // size_t dropped = mardrop(arena, 1024);
        // printf("Dropped %zu bytes from arena.\n", dropped);
        // print_marena(arena);
        // print_marena_header(arena);
    }

    marfree(arena);

    printf("\nFreed arena\n");

    return 0;
    // Copilot, please write test code that specifically tests the
    // marmalloc_partition function and marlloc_partition function, including
    // edge cases like requesting more memory than available, or partition sizes
    // that are not evenly divisible, or zero partitions, or 1, or numbers that
    // make no sense, and print the results to stdout neatly formatted.

    // begin test code
    const uint64_t total_size    = KiB(1);
    const size_t   nparts_list[] = { 0, 1, 2, 3, 4, 5, 10, 16, 32 };
    const size_t   num_tests     = sizeof(nparts_list) / sizeof(nparts_list[0]);

    for(size_t t = 0; t < num_tests; ++t) {
        size_t nparts = nparts_list[t];
        printf("Test %zu: Requesting %zu partitions from %zu bytes total\n",
               t + 1,
               nparts,
               total_size);

        if(nparts == 0) {
            printf("  Skipping zero partitions test (invalid case)\n");
            continue;
        }

        marena* parts[nparts];
        marena* mar = marmalloc_partition(total_size, parts, nparts);

        if(!mar) {
            printf("  Failed to create partitioned arena\n");
            continue;
        }

        printf("  Created partitioned arena at %p\n", (void*)mar);
        for(size_t i = 0; i < nparts; ++i) {
            printf("    Partition %zu: Address %p, Capacity %zu bytes\n",
                   i,
                   (void*)parts[i],
                   parts[i]->capacity);
        }

        // Test allocations in each partition
        for(size_t i = 0; i < nparts; ++i) {
            size_t alloc_size =
              parts[i]->capacity / 2; // Allocate half the partition
            void* ptr = marlloc(parts[i], alloc_size, 8, false);
            if(ptr) {
                printf("    Allocated %zu bytes in partition %zu at %p\n",
                       alloc_size,
                       i,
                       ptr);
            } else {
                printf("    Failed to allocate %zu bytes in partition %zu\n",
                       alloc_size,
                       i);
            }
        }
    }
    // marfree(arena);
    printf("  Freed partitioned arena\n\n");

    return 0;
#define EVEN_MORE_CASES
#ifdef EVEN_MORE_CASES
    // Edge case: Requesting more memory than available
    {
        size_t  nparts       = 4;
        size_t  request_size = KiB(1); // 1 KiB total
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Failed to create partitioned arena with "
                   "insufficient memory\n");
        } else {
            printf("Edge Case: Created partitioned arena with insufficient "
                   "memory\n");
            marfree(mar);
        }
    }

    // Edge case: Partition sizes that are not evenly divisible
    {
        size_t  nparts       = 3;
        size_t  request_size = KiB(1); // 1 KiB total
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Failed to create partitioned arena with "
                   "uneven partition sizes\n");
        } else {
            printf("Edge Case: Created partitioned arena with uneven "
                   "partition sizes\n");
            for(size_t i = 0; i < nparts; ++i) {
                printf("    Partition %zu: Capacity %zu bytes\n",
                       i,
                       parts[i]->capacity);
            }
            marfree(mar);
        }
    }

    // Edge case: Requesting 1 partition
    {
        size_t  nparts       = 1;
        size_t  request_size = KiB(1); // 1 KiB total
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Failed to create partitioned arena with 1 "
                   "partition\n");
        } else {
            printf("Edge Case: Created partitioned arena with 1 partition\n");
            printf("    Partition 0: Capacity %zu bytes\n", parts[0]->capacity);
            marfree(mar);
        }
    }

    // Edge case: Requesting zero partitions
    {
        size_t   nparts       = 0;
        size_t   request_size = KiB(1); // 1 KiB total
        marena** parts        = NULL;
        marena*  mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Correctly handled zero partitions request\n");
        } else {
            printf("Edge Case: Unexpectedly created arena with zero "
                   "partitions\n");
            marfree(mar);
        }
    }

    // Edge case: Requesting a very large number of partitions
    {
        size_t  nparts       = 1024;
        size_t  request_size = MiB(1); // 1 MiB total
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Failed to create partitioned arena with "
                   "many partitions\n");
        } else {
            printf("Edge Case: Created partitioned arena with many "
                   "partitions\n");
            marfree(mar);
        }
    }

    // Edge case: Requesting partitions with size zero
    {
        size_t  nparts       = 4;
        size_t  request_size = 0; // 0 bytes total
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf("Edge Case: Correctly handled zero total size request\n");
        } else {
            printf("Edge Case: Unexpectedly created arena with zero total "
                   "size\n");
            marfree(mar);
        }
    }

    // Edge case: Requesting partitions with negative size (invalid case)
    {
        size_t  nparts       = 4;
        int64_t request_size = -KiB(1); // -1 KiB total (invalid)
        marena* parts[nparts];
        marena* mar = marmalloc_partition(request_size, parts, nparts);

        if(!mar) {
            printf(
              "Edge Case: Correctly handled negative total size request\n");
        } else {
            printf("Edge Case: Unexpectedly created arena with negative total "
                   "size\n");
            marfree(mar);
        }
    }

    // Test case(s) involving an existing arena and marlloc_partition
    {
        size_t  nparts       = 4;
        size_t  request_size = MiB(1); // 1 MiB total
        marena* base_arena   = marmalloc(request_size);
        if(!base_arena) {
            printf("Failed to create base arena for marlloc_partition test\n");
        } else {
            marena* parts[nparts];
            marena* marena =
              marlloc_partition(base_arena, request_size, parts, nparts);

            if(!marena) {
                printf("Failed to create partitioned arena from existing "
                       "arena\n");
            } else {
                printf("Created partitioned arena from existing arena\n");
                for(size_t i = 0; i < nparts; ++i) {
                    printf("    Partition %zu: Capacity %zu bytes\n",
                           i,
                           parts[i]->capacity);
                }
                // THIS IS WRONG, THERE IS ONLY ONE MALLOC, NOT MULTIPLE ONLY
                // EVER FREE WHAT IS RETURNED BY A FUNCTION WITH MALLOC IN THE
                // NAME marfree(arena);
            }

            // Sample data. We can use urandom from common.h
            // it writes random uint8ts to a buffer

            // We can use the arena to create the test data to test the arena
            // ironically.
            uint8_t* buffer = marlloc(base_arena, 128, 8, false);
            urandom(buffer, 4096);
            printf("Random data in base arena:\n");
            for(size_t i = 0; i < 128; ++i) {
                printf("%02X ", buffer[i]);
            }
            printf("\n");

            // Start spamming it with test cases reusing the "arena" partition
            // which is just a view / subarena of some memory in base_arena

            // code:

            for(size_t i = 0; i < nparts; ++i) {
                size_t alloc_size =
                  parts[i]->capacity / 2; // Allocate half the partition
                void* ptr = marlloc(parts[i], alloc_size, 8, false);
                if(ptr) {
                    printf("    Allocated %zu bytes in partition %zu at %p\n",
                           alloc_size,
                           i,
                           ptr);
                } else {
                    printf(
                      "    Failed to allocate %zu bytes in partition %zu\n",
                      alloc_size,
                      i);
                }
            }

            // try to overflow or create a partition that's invalid because it
            // is  equalot too large;  the numbers compared to ensure it's not a
            // invlid operation are numbers that aren't truly equal to what we
            // assing them t, just close.

            for(size_t i = 0; i < nparts; ++i) {
                size_t alloc_size =
                  parts[i]->capacity + 1; // Allocate more than the partition
                void* ptr = marlloc(parts[i], alloc_size, 8, false);
                if(ptr) {
                    printf("    ERROR: Allocated %zu bytes in partition %zu at "
                           "%p (should have failed)\n",
                           alloc_size,
                           i,
                           ptr);
                } else {
                    printf("    Correctly failed to allocate %zu bytes in "
                           "partition %zu\n",
                           alloc_size,
                           i);
                }
            }

            // not yet, we wat to get multiple partitions of partitions of
            // partitoins with marlloc_partition
            marfree(base_arena);
        }
    }

#endif

    return EXIT_SUCCESS;
}
