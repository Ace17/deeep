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

unique_ptr<Object> load(string path);
unique_ptr<Object> parseObject(const char* text);
}

