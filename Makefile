CC=gcc
# CFLAGS += -Werror
CFLAGS += -Wall -Wextra -Wpedantic
# CFLAGS += -Wuninitialized -Wshadow -Warray-bounds=2 -Wnull-dereference \
# 	  -Wstringop-overflow=2 -Wstrict-prototypes -Wconversion \
# 	  -Wsign-conversion -Wfloat-equal -Wformat=2 \
# 	  -Wundef -Wcast-align
# clang
# CFLAGS += -Wuninitialized -Wshadow -Warray-bounds -Wnull-dereference \
# 	  -Wshift-overflow -Wstrict-prototypes -Wconversion \
# 	  -Wsign-conversion -Wfloat-equal -Wformat \
# 	  -Wundef -Wcast-align

ifeq ($(CC), clang)
	CFLAGS += -Wall -Wextra -Wpedantic
	CFLAGS += -Wshadow -Warray-bounds -Wnull-dereference
	CFLAGS += -Wconversion -Wsign-conversion -Wfloat-equal
	CFLAGS += -Wformat -Wundef -Wcast-align
	CFLAGS += -Wvla -Wimplicit-fallthrough
	CFLAGS += -fsanitize=address,undefined,object-size#,bounds
	# CFLAGS += -fno-omit-frame-pointer

	# debug
	CFLAGS += -Og
	CFLAGS += -g
else
	CFLAGS += -Wuninitialized -Wshadow -Warray-bounds=2 -Wnull-dereference
	CFLAGS += -Wstringop-overflow=2 -Wstrict-prototypes -Wconversion
	CFLAGS += -Wsign-conversion -Wfloat-equal -Wformat=2
	CFLAGS += -Wundef -Wcast-align
	# debug
	CFLAGS += -O0
	CFLAGS += -g
endif


# CFLAGS += -Wall -Wextra -Wpedantic
# CFLAGS += -Wshadow -Warray-bounds -Wnull-dereference
# CFLAGS += -Wconversion -Wsign-conversion -Wfloat-equal
# CFLAGS += -Wformat -Wundef -Wcast-align
# CFLAGS += -Wvla -Wimplicit-fallthrough
CFLAGS += -fsanitize=address,undefined


pango: pango2.c
	$(CC) $(CFLAGS) pango2.c -o pango_test

all: cpu gpu memory datetime

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


clean:
	rm ./blocks/cpu  ./blocks/gpu ./blocks/datetime ./blocks/*_test 2>&1>/dev/null

