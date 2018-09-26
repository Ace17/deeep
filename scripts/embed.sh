#!/usr/bin/env bash

set -euo pipefail

readonly input=$1
readonly output=$2
readonly name=$3

function main
{
  echo "// generated file"
  echo "#include <stdint.h>"
  echo "#include \"base/span.h\""
  echo "uint8_t ${name}_data[] = "
  echo "{"
  cat $input | xxd -i
  echo "};"
  echo "extern auto const $name = Span<unsigned char>(${name}_data);"
}

main > $output

