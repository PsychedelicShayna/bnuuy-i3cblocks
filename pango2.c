#include "pango2.h"
#include <locale.h>
#include <wchar.h>

int main(void)
{
    setlocale(LC_ALL, "");

    panspan ps;
    panspan_init(&ps);
    ps.foreground = "#00FF00";
    ps.background = "#00FFFF";

    wchar_t buf[4096];
    memset(buf, 0, 4096 * sizeof(wchar_t));
    pango_format(buf,
                 4096,
                 L"This should not be possible",
                 PSBT_ITAL | PSBT_BOLD | PSBT_BIG,
                 &ps);
    wprintf(L"Wtf '%ls' is this of len %zu\n", buf, wcslen(buf));

    // fprintf(stderr, "%zu\n%ls\n", wcslen(buf), buf);

    // int wrote = swprintf(buf, 4096, L"<%ls>%ls<%ls!", L"A", L"B", L"C");

    // wprintf(L"Wrote: %d\n, C:%lx", wrote, buf[wrote]);

    return 0;
}
