# CFLAGS += -Werror
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wuninitialized -Wshadow -Warray-bounds=2 -Wnull-dereference \
	  -Wstringop-overflow=2 -Wstrict-prototypes -Wconversion \
	  -Wsign-conversion -Wfloat-equal -Wformat=2 \
	  -Wundef -Wcast-align

CFLAGS += -fsanitize=address -fsanitize=undefined

all: cpu gpu memory datetime

cpu: cpu.c
	cc cpu.c -O3 -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/cpu

gpu: gpu.c
	cc gpu.c -O3 -DUSLEEPFOR=1000000 $(CFLAGS) $(shell pkg-config --cflags nvidia-ml) $(shell pkg-config --libs nvidia-ml) -lnvidia-ml -o blocks/gpu


memory: memory.c
	cc memory.c -O3 -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/memory

datetime: datetime.c
	cc datetime.c -O3 -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/datetime


clean:
	rm ./blocks/cpu  ./blocks/gpu ./blocks/datetime ./blocks/*_test 2>&1>/dev/null

