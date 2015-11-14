#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

namespace json
{
struct Value
{
  virtual ~Value()
  {
  }
};

struct Object : Value
{
  map<string, unique_ptr<Value>> members;
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

