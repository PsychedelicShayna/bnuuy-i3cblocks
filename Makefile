CC=gcc
# CFLAGS += -Werror
CFLAGS += -Wall -Wextra -Wpedantic
BUILD := debug

ifeq ($(CC),clang)
	CFLAGS += -Wall -Wextra -Wpedantic
	CFLAGS += -Wshadow -Warray-bounds -Wnull-dereference
	CFLAGS += -Wconversion -Wsign-conversion -Wfloat-equal
	CFLAGS += -Wformat -Wundef -Wcast-align
	CFLAGS += -Wvla -Wimplicit-fallthrough
ifeq ($(BUILD),release)
	CFLAGS += -O3
else
	CFLAGS += -Og
	CFLAGS += -g
	# CFLAGS += -fsanitize=address,undefined,object-size
endif
else
	CFLAGS += -Wuninitialized -Wshadow -Warray-bounds=2 -Wnull-dereference
	CFLAGS += -Wstringop-overflow=2 -Wstrict-prototypes -Wconversion
	CFLAGS += -Wsign-conversion -Wfloat-equal -Wformat=2
	CFLAGS += -Wundef -Wcast-align
ifeq ($(BUILD),release)
	CFLAGS += -O3
else
	CFLAGS += -O0
	CFLAGS += -g
	# CFLAGS += -fsanitize=address,undefined
endif
endif


pango: pango2.c
	$(CC) $(CFLAGS) pango2.c -o pango_test

all: cpu gpu netip memory datetime

cpu: cpu.c
	$(CC) cpu.c  -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/cpu

gpu: gpu.c
	$(CC) gpu.c  -DUSLEEPFOR=1000000 $(CFLAGS) $(shell pkg-config --cflags nvidia-ml) $(shell pkg-config --libs nvidia-ml) -lnvidia-ml -o blocks/gpu

netip: netip.c
	$(CC) netip.c  -DUSLEEPFOR=1000000 $(CFLAGS) -lcurl -o blocks/netip

memory: memory.c
	$(CC) memory.c  -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/memory

datetime: datetime.c
	$(CC) datetime.c  -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/datetime -lcurl -lm -ljson-c

disk: disk.c
	$(CC) disk.c  -DUSLEEPFOR=1000000 $(CFLAGS) -o blocks/disk -lm


clean:
	rm ./blocks/cpu  ./blocks/gpu ./blocks/datetime ./blocks/memory ./blocks/netip ./blocks/*_test 2>&1>/dev/null

