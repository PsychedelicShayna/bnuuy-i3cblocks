#!/bin/dash
case $BLOCK_BUTTON in
  # 1) alacritty -e btop & ;;
  1) xfce4-taskmanager & ;;
esac

PREV="/tmp/.i3blocks_cpu"
CURRENT=$(grep '^cpu ' /proc/stat)
PREV_DATA=$(cat "$PREV" 2>/dev/null)

echo "$CURRENT" > "$PREV"

if [ -z "$PREV_DATA" ]; then echo "CPU: ..."; exit; fi

set -- $CURRENT
IDLE_NOW=$5
TOTAL_NOW=$((${1}+${2}+${3}+${4}+${5}+${6}+${7}+${8}))

set -- $PREV_DATA
IDLE_BEFORE=$5
TOTAL_BEFORE=$((${1}+${2}+${3}+${4}+${5}+${6}+${7}+${8}))

# How this reaches a value is basically by checking how much of the total
# CPU time did not consist of idle time, but it does so not on the current
# values, but against the *delta* of the previous and current ones.

# It asks "how much of the difference in CPU time between two samples, 
# is a result of the CPU working, not idling?" this is the most important
# part: (DIFF_TOTAL - DIFF_IDLE) / DIFF_TOTAL
#
# That's your percentage. After a fashion.
# 
# Why diff and not operate directly? Because CPU time *always* increasees, so
# you only want to know how much of the increase is not because of the time itself
# but because of the relative activity betwen the two samples. In other words,
# it filters out the increase due to total CPU time increasing, by getting delta
# then once you have two values that are different, past and present, of total
# and idle time, and then get the ratio, you're getting the ratio of how much
# the increase was non-IDLE CPU time. If you don't get two samples, the value
# you get as a result is interesting but not irrelevant here; it always trends
# upwards, and the true percentage is lost in the noise because two sources
# product a +delta: overall increase in CPU time, and lower idle time resulting
# in higher total-idle value. When these two sources of value mix, it becomes 
# very difficult (although there probably is a mathemtical way, maybe signal
# to noise isolation algorithms) to isolate the difference that actually matters.
# The only way to know whta was the result of CPU time increasing, is to know
# the invariants necessary for the word "increasing/increase"'s definition to
# be coherent: a reference point. Increase is a relative concept.
 
DIFF_IDLE=$((IDLE_NOW - IDLE_BEFORE))
DIFF_TOTAL=$((TOTAL_NOW - TOTAL_BEFORE))
DIFF_USAGE=$(( (1000 * (DIFF_TOTAL - DIFF_IDLE) / DIFF_TOTAL + 5) / 10 ))

FREQ=$(./cpufreq.sh)
# echo "CPU: ${DIFF_USAGE}% @ ${FREQ} "
echo "${DIFF_USAGE}% ${FREQ} "
echo ""

# color hinting
if [ $DIFF_USAGE -gt 80 ]; then
  echo "#cc241d"
elif [ $DIFF_USAE -gt 50 ]; then 
  echo "#d65d0e"
fi
