// Copyright (C) 2020 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Unit test framework: test runner

#include "tests.h"
#include <cassert>
#include <cstdio>
#include <cstring> // strstr
#include <stdlib.h> // abort

static Test* g_first;
bool g_sorted;

void failUnitTest(char const* file, int line, const char* msg)
{
  fprintf(stderr, "[%s:%d] %s\n", file, line, msg);
  abort();
}

Registration registerTest(Test& test)
{
  test.next = g_first;
  g_first = &test;
  return {};
}

void runTests(const char* filter)
{
  if(!g_sorted) // reverse list
  {
    Test* curr = g_first;
    g_first = nullptr;

    while(curr)
    {
      auto next = curr->next;
      curr->next = g_first;
      g_first = curr;

      curr = next;
    }
  }

  printf("Running tests.\n");

  auto test = g_first;

  int i = 0;

  while(test)
  {
    if(strstr(test->name, filter))
    {
      printf("[%d] %s\n", i, test->name);
      test->func();
    }

    ++i;
    test = test->next;
  }
}

