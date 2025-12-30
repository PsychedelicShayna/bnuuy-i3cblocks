#!/bin/bash

set -eou pipefail

clock_glyph() {
  local hour
  hour=$(date '+%H')
  case $hour in
      "00") printf '%s' 󱑖 ;;
      "01") printf '%s' 󱑋 ;;
      "02") printf '%s' 󱑌 ;;
      "03") printf '%s' 󱑍 ;;
      "04") printf '%s' 󱑎 ;;
      "05") printf '%s' 󱑏 ;;
      "06") printf '%s' 󱑐 ;;
      "07") printf '%s' 󱑑 ;;
      "08") printf '%s' 󱑒 ;;
      "09") printf '%s' 󱑓 ;;
      "10") printf '%s' 󱑔 ;;
      "11") printf '%s' 󱑕 ;;
      "12") printf '%s' 󱑊 ;;
      "13") printf '%s' 󱐿 ;; 
      "14") printf '%s' 󱑀 ;; 
      "15") printf '%s' 󱑁 ;; 
      "16") printf '%s' 󱑂 ;; 
      "17") printf '%s' 󱑃 ;; 
      "18") printf '%s' 󱑄 ;; 
      "19") printf '%s' 󱑅 ;;
      "20") printf '%s' 󱑆 ;;
      "21") printf '%s' 󱑇 ;;
      "22") printf '%s' 󱑈 ;;
      "23") printf '%s' 󱑉 ;;
  esac
}

season_glyph() {
  local hemisphere=$1

  local month
  month=$(date '+%m')

  if [[ $hemisphere == north ]]; then
    case $month in
      12|01|02) printf '' ;;  # Winter
      03|04|05) printf '󰉊' ;;  # Spring
      06|07|08) printf '' ;;  # Summer
      09|10|11) printf '󰲓' ;;  # Autumn
    esac
  else
    case $month in
      06|07|08) printf '' ;;  # Winter
      09|10|11) printf '󰉊' ;;  # Spring
      12|01|02) printf '' ;;  # Summer
      03|04|05) printf '󰲓' ;;  # Autumn
    esac
  fi
}

clock=$(clock_glyph)
season=$(season_glyph south)

out=$(date "+ $season  %a %d %b $clock  %H:%M ")
echo "$out"
