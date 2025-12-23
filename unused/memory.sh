#!/bin/bash
case $BLOCK_BUTTON in
  # 1) alacritty -e btop & ;;
  1) xfce4-taskmanager & ;;
esac


read total used <<<$(free -m | awk '/Mem:/ {print $2, $3}')
# echo "RAM: ${used}/${total}MB"
PERCENT=$(( used * 100 / total ))

# echo " ${used}m  $PERCENT%"
printf ' %s %0.03fG \n' "$PERCENT%" $(awk "BEGIN { print $used / 1024 }") 




# Optional short_text (skip)
echo

# Color as third line — only one echo, not conditional
if [ "$PERCENT" -gt 75 ]; then
    echo "#cc241d"
elif [ "$PERCENT" -gt 50 ]; then
    echo "#d65d0e"
fi
