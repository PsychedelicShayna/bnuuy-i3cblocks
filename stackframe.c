#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistdio.h>
#include <unistr.h>

static inline char* test1(const char* data) {
    size_t s   = strlen(data) + 1;
    char*  buf = alloca(s);
    // char buf[s];
    memset(buf, 0, s);
    memcpy(buf, data, s);

    for(size_t i = 0; i < s; ++i) {
        if(buf[i] >= 'a' && buf[i] <= 'z') {

            if(buf[i] < 'a' + 13 && buf[i] + 13 <= 'z') {
                buf[i] += 13;
            } else if(buf[i] >= 'a' + 13 && buf[i] - 13 >= 'a') {
                buf[i] -= 13;
            }
        }
    }

    return buf;
}

int main(void) {
    const char* msg = "for i have plucked the rose, tea earl grey hot, longing "
                      "still for that which further nurses the disease!";

    char* x = test1(msg);

    printf("X:%s\n", x);
}
