#!/bin/dash
printf "%s\n" "$(ip route get 8.8.8.8 2>/dev/null | awk '{print $7; exit}')"
