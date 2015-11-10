#!/bin/bash

set -euo pipefail

readonly scriptDir=$(dirname $0)
readonly dir=$1

function list_files
{
  find $dir -name "*.d" -or -name "*.cpp" -or -name "*.c" -or -name "*.h"
}

list_files | while read f; do
	uncrustify -c "$scriptDir/uncrustify.cfg" -f "$f" -o "$f.tmp" -q
  if ! diff -Naur "$f" "$f.tmp" ; then
    echo "Formatting $f"
    mv "$f.tmp" "$f"
  else
    rm "$f.tmp"
  fi
done
