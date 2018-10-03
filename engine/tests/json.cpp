// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "engine/src/json.h"
#include "tests.h"
#include <vector>
using namespace std;

static
bool jsonOk(string text)
{
  try
  {
    json::parse(text.c_str(), text.size());
    return true;
  }
  catch(exception const &)
  {
    return false;
  }
}

static
auto jsonParse(string text)
{
  return json::parse(text.data(), text.size());
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

unittest("Json parser: booleans")
{
  assert(jsonOk("{ \"var\": true }"));
  assert(jsonOk("{ \"var\": false }"));

  {
    auto o = jsonParse("{ \"isCool\" : true }");
    auto s = o->getMember<json::Boolean>("isCool");
    assertEquals(true, s->value);
  }

  {
    auto o = jsonParse("{ \"isSlow\" : false }");
    auto s = o->getMember<json::Boolean>("isSlow");
    assertEquals(false, s->value);
  }
}

unittest("Json parser: non-zero terminated")
{
  json::parse("{ \"isCool\" : true } _invalid_json_token_", 19);
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
    auto o = jsonParse("{}");
    assert(o);
    assertEquals(0u, o->members.size());
  }
  {
    auto o = jsonParse("{ \"N\" : \"hello\"}");
    assertEquals(1u, o->members.size());
    auto s = dynamic_cast<json::String*>(o->members["N"].get());
    assert(s);
    assert(s->value == "hello");
  }
  {
    auto o = jsonParse("{ \"N\" : -1234 }");
    assertEquals(1u, o->members.size());
    auto s = dynamic_cast<json::Number*>(o->members["N"].get());
    assert(s);
    assertEquals(-1234, s->value);
  }
}

