#!/bin/bash
DEVICE=$(lsblk -ndo NAME,TYPE | awk '$2=="disk" {print $1; exit}')
OLD=($(cat /sys/block/$DEVICE/stat | awk '{print $3, $7}'))
sleep 1
NEW=($(cat /sys/block/$DEVICE/stat | awk '{print $3, $7}'))
R=$(( (${NEW[0]} - ${OLD[0]}) * 512 / 1024 ))
W=$(( (${NEW[1]} - ${OLD[1]}) * 512 / 1024 ))
echo "${R}K↓ ${W}K↑"
