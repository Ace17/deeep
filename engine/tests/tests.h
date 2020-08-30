// Copyright (C) 2020 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Unit test framework: API

#include <sstream>

#define unittest(name) \
  unittestWithCounter(__COUNTER__, name)

#define assertTrue(expr) \
  assertTrueFunc(__FILE__, __LINE__, # expr, expr)

#define assertEquals(expected, actual) \
  assertEqualsFunc(__FILE__, __LINE__, # actual, expected, actual)

#define assertThrown(expr) \
  do { try{ expr; failUnitTest(__FILE__, __LINE__, "No exception was thrown: " # expr); }catch(...){} \
  } while (0)

int RegisterTest(void (* f)(), const char* testName);
void RunTests(const char* filter);

struct Registrator
{
  Registrator(void(*f)(), char const* name)
  {
    RegisterTest(f, name);
  }
};

static inline
std::ostream& operator << (std::ostream& o, const std::pair<int, int>& p)
{
  o << "(";
  o << p.first;
  o << ", ";
  o << p.second;
  o << ")";
  return o;
}

// match std::vector
template<typename T, typename = decltype(((T*)nullptr)->emplace({}))>
std::ostream& operator << (std::ostream& o, const T& container)
{
  bool comma = false;
  o << "[";

  for(auto& element : container)
  {
    if(comma)
      o << ", ";

    o << element;
    comma = true;
  }

  o << "]";

  return o;
}

///////////////////////////////////////////////////////////////////////////////
// implementation details

struct Test
{
  void (* func)();
  const char* name;
  Test* next = nullptr;
};

#define unittestWithCounter(counter, name) \
  unittest2(counter, name)

#define unittest2(counter, name) \
  static void g_myTest ## counter(); \
  static Test g_myTestInfo ## counter = { &g_myTest ## counter, name }; \
  static auto g_registration ## counter = RegisterTest(g_myTestInfo ## counter); \
  static void g_myTest ## counter()

struct Registration {};
Registration RegisterTest(Test& test);

void failUnitTest(char const* file, int line, const char* msg);

template<typename T>
inline void assertTrueFunc(char const* file, int line, const char* caption, T const& expr)
{
  if(expr)
    return;

  failUnitTest(file, line, caption);
}

template<typename T, typename U>
inline void assertEqualsFunc(const char* file, int line, const char* caption, T const& expected, U const& actual)
{
  if(expected == actual)
    return;

  std::stringstream ss;
  ss << "'" << caption << "' has an invalid value\n";
  ss << "  Expect: '" << expected << "'\n";
  ss << "  Actual: '" << actual << "'\n";
  failUnitTest(file, line, ss.str().c_str());
}

