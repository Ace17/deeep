/**
 * @brief Unit test framework
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include "tests.h"

namespace
{
  struct Test
  {
    void (*run)();
    const char* sName;
  };

  int const MAX_TESTS = 512;

  Test tests[MAX_TESTS];
  int numTests;
}

int RegisterTest(void (*proc)(), const char* testName)
{
  assert(numTests < MAX_TESTS);
  tests[numTests].run = proc;
  tests[numTests].sName = testName;
  ++numTests;
  return 1;
}

void RunTests()
{
  std::cout << "Running " << numTests << " tests." << std::endl;
  bool success = true;
  for(int i=0;i < numTests;++i)
  {
    try
    {
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Test: " << tests[i].sName << std::endl;
      tests[i].run();
      std::cout << std::endl;
    }
    catch(std::exception const& e)
    {
      std::cout << "Test aborted on exception: " << e.what() << std::endl;
      success = false;
    }
    catch(...)
    {
      std::cout << "Test aborted on exception" << std::endl;
      success = false;
    }
  }

  if(!success)
    throw std::runtime_error("At least one test failed");
}

