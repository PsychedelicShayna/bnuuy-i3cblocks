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

CFLAGS += -Isrc/include

OUTDIR=blocks
SRCDIR=src
STATIC=-lm

all: cpu gpu netip memory datetime disk

cpu: $(SRCDIR)/cpu.c
	$(CC) $(CFLAGS) $(STATIC) $(SRCDIR)/cpu.c -o $(OUTDIR)/cpu -DUSLEEPFOR=1000000  

gpu: $(SRCDIR)/gpu.c
	$(CC) $(CFLAGS) $(STATIC) \
		$(SRCDIR)/gpu.c -o $(OUTDIR)/gpu \
		-DUSLEEPFOR=1000000 \
		$(shell pkg-config --cflags nvidia-ml) \
		$(shell pkg-config --libs nvidia-ml)  \
		-lnvidia-ml

netip: $(SRCDIR)/netip.c
	$(CC) $(CFLAGS) $(STATIC) \
		$(SRCDIR)/netip.c -o $(OUTDIR)/netip \
		-DUSLEEPFOR=1000000 \
		-lcurl

memory: $(SRCDIR)/memory.c
	$(CC) $(CFLAGS) $(STATIC) \
		$(SRCDIR)/memory.c -o $(OUTDIR)/memory \
		-DUSLEEPFOR=1000000

datetime: $(SRCDIR)/datetime.c
	$(CC) $(CFLAGS) $(STATIC) \
		$(SRCDIR)/datetime.c -o $(OUTDIR)/datetime \
		-DUSLEEPFOR=1000000  \
		-ljson-c \
		-lcurl

disk: $(SRCDIR)/disk.c
	$(CC) $(CFLAGS) $(STATIC) \
		$(SRCDIR)/disk.c -o $(OUTDIR)/disk \
		-DUSLEEPFOR=1000000

clean:
	rm \
		./$(OUTDIR)/cpu \
		./$(OUTDIR)/gpu \
		./$(OUTDIR)/netip \
		./$(OUTDIR)/memory \
		./$(OUTDIR)/datetime \
		./$(OUTDIR)/disk \
		./$(OUTDIR)/*_test \
		2>&1>/dev/null

