#!/bin/bash

readonly input=$1
readonly output=$2
readonly name=$3

function main
{
  echo "// generated file"
  echo "#include <stdlib.h>"
  echo "unsigned char $name[] = "
  echo "{"
  cat $input | xxd -i
  echo "};"
  echo "size_t ${name}_size = sizeof($name);"
}

main > $output

