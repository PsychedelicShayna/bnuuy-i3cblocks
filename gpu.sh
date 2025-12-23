#!/bin/dash
case $BLOCK_BUTTON in
  1) termite -e nvidia-smi & ;;
esac

TEMP=$(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits | head -n1)
UTIL=$(nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits | head -n1)
# echo "GPU: ${UTIL}% ${TEMP}°C "
echo "${UTIL}% ${TEMP}°C "
echo ""

if [ $UTIL -gt 75 ]; then 
  echo "#cc241d"
elif [ $UTIL -gt 50 ]; then
  echo "#d65d0e"
fi


