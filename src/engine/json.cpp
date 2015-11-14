#include "json.h"
#include "tokenizer.h"
#include <fstream>

using namespace json;

static
ifstream openInput(string path)
{
  ifstream fp(path);

  if(!fp.is_open())
    throw runtime_error("Can't open file '" + path + "'");

  return fp;
}

static
string read(string path)
{
  auto fp = openInput(path);

  string r;
  string line;

  while(getline(fp, r))
    r += line;

  return r;
}

unique_ptr<Object> json::load(string path)
{
  auto const text = read(path);
  return json::parseObject(text.c_str());
}

unique_ptr<Object> json::parseObject(const char* text)
{
  unique_ptr<Object> r;
  Tokenizer tokenizer(text);
  return r;
}

