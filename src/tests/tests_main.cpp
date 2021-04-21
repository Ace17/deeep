// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Unit test framework: entry point

#include "base/error.h"
#include "tests.h"
#include <cstdio>

using namespace std;

int main(int argc, char* argv[])
{
  char const* filter = "";

  if(argc == 2)
    filter = argv[1];

  try
  {
    runTests(filter);
    return 0;
  }
  catch(const Error& e)
  {
    fprintf(stderr, "Fatal: %.*s\n", e.message().len, e.message().data);
    return 1;
  }
}

