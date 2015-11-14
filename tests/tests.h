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

#pragma once

#include <cassert>

int RegisterTest(void (*f)(), const char* testName);
void RunTests();

struct Registrator
{
  Registrator(void (*f)(), char const* name)
  {
    RegisterTest(f, name);
  }
};

#define unittest(name) \
  unittest2(__LINE__, name)

#define unittest2(line, name) \
  unittest3(testFunction, line, name)

#define unittest3(prefix, line, name) \
  static void prefix##line(); \
  static Registrator g_Registrator##line(&prefix##line, name); \
  static void prefix##line()

