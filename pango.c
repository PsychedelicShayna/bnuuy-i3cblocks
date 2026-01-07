#include "pango.h"
#include "arena.h"
#include <wchar.h>

int main(void)
{
    marena* a = marena_create(MiB(1));

    pangosb* sb = marena_alloc(a, sizeof(pangosb), alignof(pangosb), false);
    pangosb_init(sb, a, 4096);

    printf("Created\n");

    // pangosb_wpush(sb, L"Hello World", PSBT_ITAL | PSBT_BOLD, NULL);
    // pangosb_wpush(sb, L"Goodbye World", PSBT_ITAL | PSBT_BOLD, NULL);

    wchar_t buf[4096];
    memset(buf, 0, 4096 * sizeof(wchar_t));

    pangosb_get_last(sb, buf, 4096);
    printf("Tag: %ls, Len: %zu, Sizeof: %zu\n", buf, wcslen(buf), sizeof(buf));

    wcslcat(buf, L"Testing", 4096);

    printf("Tag: %ls, Len: %zu, Sizeof: %zu\n", buf, wcslen(buf), sizeof(buf));

    // wchar_t* strings = marena_alloc(a, sizeof(wchar_t*) * 4096, ARENA_ALIGN,
    // false); wchar_t* strings2 = marena_alloc(a, sizeof(wchar_t*) * 4096,
    // ARENA_ALIGN, false); wchar_t* strings3 = marena_alloc(a, sizeof(wchar_t*)
    // * 4096, ARENA_ALIGN, false);

    pangosb_fini(sb);
    marena_drop(a, a->count);
    marena_free(a);

    printf("Freed\n");
    return 0;
}
