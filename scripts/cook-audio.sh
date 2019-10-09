#!/usr/bin/env bash
set -euo pipefail

readonly output="$1"
readonly input="$2"

args=()
args+=(-ar 22050 -ac 2)

readonly descFile="$2.desc"
if test -f "$2.desc" ; then
  vol=$(cat $descFile)
  args+=(-filter:a "volume=${vol}dB")
fi

ffmpeg -loglevel 1 -y -i "$input" "${args[@]}" "$output" </dev/null
