#ifndef _ARENA_H
#define _ARENA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

typedef int8_t  bool8_t;
typedef int32_t bool32_t;

#define KiB(n) ((uint64_t)(n) << 10)
#define MiB(n) ((uint64_t)(n) << 20)
#define GiB(n) ((uint64_t)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ALIGN_UP_POW2(n, p) \
    (((uint64_t)(n) + ((uint64_t)(p) - 1)) & (~((uint64_t)(p) - 1)))

#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN    (sizeof(void*))

#define PUSH_STRUCT(arena, T)    (T*)arena_push((arena), sizeof(T), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)

#define PUSH_ARRAY(arena, T, count) \
    (T*)arena_push((arena), sizeof(T) * (count), false)

#define PUSH_ARRAY_NZ(arena, T, count) \
    (T*)arena_push((arena), sizeof(T) * (count), true)

#define SWAP_PTRS(a, b)       \
    {                         \
        wchar_t* temp = a;    \
        a             = b;    \
        b             = temp; \
    }

typedef struct {
    uint64_t capacity;
    uint64_t pos;
} mem_arena;

typedef struct {
    mem_arena* arena;
    uint64_t   start_pos;
} mem_arena_temp;

mem_arena_temp arena_scratch_get(mem_arena** conflicts, uint32_t num_conflicts);
void           arena_scratch_release(mem_arena_temp scratch);

mem_arena* arena_create(uint64_t capacity);
void       arena_destroy(mem_arena* arena);
void*      arena_push(mem_arena* arena, uint64_t size, bool32_t non_zero);
void       arena_pop(mem_arena* arena, uint64_t size);
void       arena_pop_to(mem_arena* arena, uint64_t pos);
void       arena_clear(mem_arena* arena);

mem_arena_temp arena_temp_begin(mem_arena* arena);
void           arena_temp_end(mem_arena_temp arena);

static __thread mem_arena* _scratch_arena[2] = { NULL, NULL };
static __thread mem_arena* _global_arena     = NULL;

#define _global_arena_cap MiB(64)
#define GlobalArena       (arena_global())

static inline mem_arena* arena_global(void)
{
    if(_global_arena == NULL) {
        _global_arena = arena_create(_global_arena_cap);
    }

    return _global_arena;
}

mem_arena_temp arena_scratch_get(mem_arena** conflicts, uint32_t num_conflicts)
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
        return (mem_arena_temp) { 0 };
    }

    mem_arena** selected = &_scratch_arena[scratch_index];

    if(*selected == NULL) {
        *selected = arena_create(MiB(64));
    }

    return arena_temp_begin(*selected);
}

void arena_scratch_release(mem_arena_temp scratch)
{
    arena_temp_end(scratch);
}

mem_arena* arena_create(uint64_t capacity)
{

    mem_arena* arena = (mem_arena*)malloc(ARENA_BASE_POS + capacity);
    if(!arena) {
        return NULL;
    }
    arena->capacity = capacity;
    arena->pos      = ARENA_BASE_POS;
    return arena;
}

void arena_destroy(mem_arena* arena)
{
    free(arena);
}

void* arena_push(mem_arena* arena, uint64_t size, bool32_t non_zero)
{
    uint64_t aligned_pos = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    uint64_t new_pos     = aligned_pos + size;

    if(new_pos > arena->capacity) {
        return NULL;
    }

    arena->pos = new_pos;
    void* ptr  = (uint8_t*)arena + aligned_pos;

    if(!non_zero) {
        memset(ptr, 0, size);
    }

    return ptr;
}

void arena_pop(mem_arena* arena, uint64_t size)
{
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}

void arena_pop_to(mem_arena* arena, uint64_t pos)
{
    uint64_t size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(arena, size);
}

void arena_clear(mem_arena* arena)
{
    arena_pop_to(arena, ARENA_BASE_POS);
}

mem_arena_temp arena_temp_begin(mem_arena* arena)
{
    return (mem_arena_temp) { .arena = arena, .start_pos = arena->pos };
}

void arena_temp_end(mem_arena_temp temp)
{
    arena_pop_to(temp.arena, temp.start_pos);
}

#endif // !_ARENA_H
