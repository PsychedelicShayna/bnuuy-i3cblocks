#!/bin/dash
# df -h / | awk 'NR==2 {print "Disk: " $4 " free"}'
# drive1="$(df -h / | awk 'NR==2 { print $5, $4  }')"
# drive2="$(df -h '/mnt/alias/m2/' | awk 'NR==2 { print $5, $4  }')"

drive1="$(df -h / | awk 'NR==2 { print $5, $4  }')"
drive2="$(df -h /mnt/alias/m2/ | awk 'NR==2 { print $5, $4  }')"
printf "%s %s" "$drive1" "$drive2"



