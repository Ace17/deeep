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
  assert(jsonOk("{ \"var\": 0 }"));
  assert(jsonOk("{ \"var\": -10 }"));
  assert(jsonOk("{ \"hello\": \"world\" }"));
  assert(jsonOk("{ \"N1\": \"V1\", \"N2\": \"V2\" }"));

  assert(!jsonOk("{ \"N1\" : : \"V2\" }"));
}

unittest("Json parser: arrays")
{
  assert(jsonOk("{ \"A\": [] }"));
  assert(jsonOk("{ \"A\": [ { }, { } ] }"));
  assert(jsonOk("{ \"A\": [ \"hello\", \"world\" ] }"));

  assert(!jsonOk("{ \"A\": [ }"));
  assert(!jsonOk("{ \"A\": ] }"));
}

unittest("Json parser: returned value")
{
  {
    auto o = json::parseObject("{}");
    assert(o);
    assertEquals(0u, o->members.size());
  }
  {
    auto o = json::parseObject("{ \"N\" : \"hello\"}");
    assertEquals(1u, o->members.size());
    auto s = dynamic_cast<json::String*>(o->members["N"].get());
    assert(s);
    assert(s->value == "hello");
  }
  {
    auto o = json::parseObject("{ \"N\" : -1234 }");
    assertEquals(1u, o->members.size());
    auto s = dynamic_cast<json::Number*>(o->members["N"].get());
    assert(s);
    assertEquals(-1234, s->value);
  }
}

