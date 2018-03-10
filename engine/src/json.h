// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;

namespace json
{
template<typename To, typename From>
To* cast(From* from)
{
  auto to = dynamic_cast<To*>(from);

  if(from && !to)
    throw runtime_error("Type error");

  return to;
}

struct Value
{
  virtual ~Value()
  {
  }
};

struct Object : Value
{
  map<string, unique_ptr<Value>> members;

  template<typename T>
  T* getMember(string name)
  {
    auto it = members.find(name);

    if(it == members.end())
      throw runtime_error("Member '" + name + "' was not found");

    auto pValue = it->second.get();
    return cast<T>(pValue);
  }
};

struct Array : Value
{
  vector<unique_ptr<Value>> elements;
};

struct String : Value
{
  string value;
};

struct Number : Value
{
  int value;
};

struct Boolean : Value
{
  bool value;
};

unique_ptr<Object> parse(const char* text);
}

