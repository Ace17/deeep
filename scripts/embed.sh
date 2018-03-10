#!/usr/bin/env bash

set -euo pipefail

readonly input=$1
readonly output=$2
readonly name=$3

function main
{
  echo "// generated file"
  echo "#include <stdlib.h>"
  echo "#include \"base/span.h\""
  echo "unsigned char ${name}_data[] = "
  echo "{"
  cat $input | xxd -i
  echo "};"
  echo "extern auto const $name = makeSpan(${name}_data);"
}

main > $output

