// Copyright (C) 2020 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// User-code API

#include <string>

#define unittest(name) \
  unittestWithCounter(__COUNTER__, name)

#define assertTrue(expr) \
  assertTrueFunc(__FILE__, __LINE__, # expr, expr)

#define assertEquals(expected, actual) \
  assertEqualsFunc(__FILE__, __LINE__, # actual, expected, actual)

#define assertThrown(expr) \
  do { \
    try \
    { \
      expr; \
      failUnitTest(__FILE__, __LINE__, "No exception was thrown: " # expr); \
    } \
    catch(...){} \
  } while (0)

void runTests(const char* filter);

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
  static auto g_registration ## counter = registerTest(g_myTestInfo ## counter); \
  static void g_myTest ## counter()

struct Registration {};
Registration registerTest(Test& test);

void failUnitTest(char const* file, int line, const char* msg);

template<typename U, typename dummy = void>
struct ToStringImpl;

template<typename T>
std::string testValueToString(const T& val)
{
  return ToStringImpl<T>::call(val);
}

// match std::vector
template<typename T>
struct ToStringImpl < T, decltype(((T*)nullptr)->push_back((typename T::value_type) {})) >
{
  static std::string call(const T& val)
  {
    std::string r;

    bool first = true;
    r += "[";

    for(auto& element : val)
    {
      if(!first)
        r += ", ";

      r += testValueToString(element);
      first = false;
    }

    r += "]";

    return r;
  }
};

// match std::string
template<>
struct ToStringImpl<std::string>
{
  static std::string call(const std::string& val) { return val; }
};

template<>
struct ToStringImpl<char>
{
  static std::string call(const char& val) { return std::to_string(val); }
};

template<>
struct ToStringImpl<float>
{
  static std::string call(const float& val) { return std::to_string(val); }
};

template<>
struct ToStringImpl<unsigned char>
{
  static std::string call(const unsigned char& val) { return std::to_string(val); }
};

template<>
struct ToStringImpl<unsigned int>
{
  static std::string call(const unsigned int& val) { return std::to_string(val); }
};

template<>
struct ToStringImpl<bool>
{
  static std::string call(const bool& val) { return val ? "true" : "false"; }
};

template<>
struct ToStringImpl<int>
{
  static std::string call(const int& val) { return std::to_string(val); }
};

template<>
struct ToStringImpl<size_t>
{
  static std::string call(const size_t& val) { return std::to_string(val); }
};

inline void assertTrueFunc(char const* file, int line, const char* caption, bool expr)
{
  if(!expr)
    failUnitTest(file, line, caption);
}

template<typename T, typename U>
inline void assertEqualsFunc(const char* file, int line, const char* caption, T const& expected, U const& actual)
{
  if(expected == actual)
    return;

  std::string ss;
  ss += "'" + std::string(caption) + "' has an invalid value\n";
  ss += "  Expect: '" + testValueToString(expected) + "'\n";
  ss += "  Actual: '" + testValueToString(actual) + "'\n";
  failUnitTest(file, line, ss.c_str());
}

