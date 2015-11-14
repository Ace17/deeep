#include "engine/json.h"
#include "tests/tests.h"
#include <vector>
using namespace std;

static
bool jsonOk(string text)
{
  try
  {
    json::parseObject(text.c_str());
    return true;
  }
  catch(exception const&)
  {
    return false;
  }
}

unittest("Json parser: empty")
{
  assert(jsonOk("{}"));

  assert(!jsonOk("{"));
}

unittest("Json parser: members")
{
  assert(jsonOk("{ \"hello\": \"world\" }"));
  assert(jsonOk("{ \"N1\": \"V1\", \"N2\": \"V2\" }"));

  assert(!jsonOk("{ \"N1\" : : \"V2\" }"));
}

unittest("Json parser: arrays")
{
  assert(jsonOk("{ \"A\": [] }"));
  assert(jsonOk("{ \"A\": [ { }, { } ] }"));

  assert(!jsonOk("{ \"A\": [ }"));
}

