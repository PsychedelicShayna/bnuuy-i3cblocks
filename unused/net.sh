#!/bin/dash
IFACE=$(ip route get 8.8.8.8 | awk -- '{print $5; exit}')
R1=$(cat /sys/class/net/$IFACE/statistics/rx_bytes)
T1=$(cat /sys/class/net/$IFACE/statistics/tx_bytes)
sleep 1
R2=$(cat /sys/class/net/$IFACE/statistics/rx_bytes)
T2=$(cat /sys/class/net/$IFACE/statistics/tx_bytes)
RB=$(( (R2 - R1) / 1024 ))
TB=$(( (T2 - T1) / 1024 ))
# echo "Net: ${RB}K↓ ${TB}K↑"
echo "${RB}K↓ ${TB}K↑"
