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
#include <stdexcept>
#include <sstream>
#include <vector>

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

#define assertEquals(u, v) \
  assertEqualsFunc(u, v, __FILE__, __LINE__)

template<typename T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& v)
{
  int idx=0;
  o << "[";
  for(auto& element : v)
  {
    if(idx > 0)
      o << ", ";
    o << element;
    ++idx;
  }
  o << "]";
  return o;
}

template<typename U, typename V>
void assertEqualsFunc(
    U const& expected,
    V const& actual,
    const char* file,
    int line)
{
  if(expected != actual)
  {
    using namespace std;
    stringstream ss;
    ss << "Assertion failure" << endl;
    ss << file << "(" << line << ")" << endl;
    ss << "Expected " << expected << ", got " << actual << endl;
    throw logic_error(ss.str());
  }
}

