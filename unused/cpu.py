#!/usr/bin/env python3
import os
import sys
import fcntl
import time

PREV = "/tmp/.i3blocks_cpu"

# Fast /proc/stat read - direct seek to cpu line (faster than readline loop)
with open("/proc/stat", "rb", buffering=0) as f:
    while True:
        line = f.readline()
        if not line:
            sys.exit(1)
        if line.startswith(b'cpu '):
            current = line.decode().rstrip('\n')
            break

# Atomic file read/write with flock (eliminates race conditions)
flags = os.O_RDWR | os.O_CREAT
prev_fd = os.open(PREV, flags, 0o644)
fcntl.flock(prev_fd, fcntl.LOCK_EX)

try:
    prev_data = os.read(prev_fd, 1024).decode().rstrip('\n')
except OSError:
    prev_data = ""

# Truncate and write current
os.lseek(prev_fd, 0, os.SEEK_SET)
os.ftruncate(prev_fd, 0)
os.write(prev_fd, current.encode())
os.close(prev_fd)

if not prev_data:
    print("CPU: ...")
    sys.exit(0)


# Parse fields (skip cpu label, take first 9 values)
curr_fields = [int(x) for x in current.split()[1:9]]
prev_fields = [int(x) for x in prev_data.split()[1:9]]

idle_now = curr_fields[3]      # field 5 (0-indexed)
total_now = sum(curr_fields)
idle_prev = prev_fields[3]
total_prev = sum(prev_fields)

diff_idle = idle_now - idle_prev
diff_total = total_now - total_prev
diff_usage = ((1000 * (diff_total - diff_idle) // diff_total) + 5) // 10

# Direct sysfs read (faster than subprocess)
try:
    with open('/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq', 'r') as f:
        freq_raw = int(f.read().strip())
        freq_mhz = freq_raw / 1000.0
        freq = f"{freq_mhz / 1000:.2f}GHz"
except:
    freq = "N/A"

print(f"{diff_usage}% {freq}")
print("")

# Color hinting (uncommented from Perl)
if diff_usage > 80:
    print("#cc241d")
elif diff_usage > 50:
    print("#d65d0e")
