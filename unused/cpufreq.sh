#!/bin/dash

echo "$(cat '/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq' | awk '{ l=length($1); x=sprintf("%."l"f", $1/1000000); printf("%sGHz", substr(x, 1, 4)) }')"
