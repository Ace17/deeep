// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Unit test framework: test runner

#include "tests.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace
{
struct Test
{
  void (* run)();
  const char* sName;
};

int const MAX_TESTS = 512;

Test tests[MAX_TESTS];
int numTests;
}

int RegisterTest(void (* proc)(), const char* testName)
{
  assert(numTests < MAX_TESTS);
  tests[numTests].run = proc;
  tests[numTests].sName = testName;
  ++numTests;
  return 1;
}

void RunTests(const char* filter)
{
  printf("Running tests.\n");

  for(int i = 0; i < numTests; ++i)
  {
    if(!strstr(tests[i].sName, filter))
      continue;

    printf("[%d] %s\n", i, tests[i].sName);
    tests[i].run();
  }
}

