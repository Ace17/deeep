#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

namespace json
{
struct Base
{
  virtual ~Base()
  {
  }
};

struct Object : Base
{
  map<string, unique_ptr<Base>> members;
};

struct Array : Base
{
  vector<unique_ptr<Base>> elements;
};

struct String : Base
{
  string value;
};

unique_ptr<Object> load(string path);
unique_ptr<Object> parseObject(const char* text);
}

