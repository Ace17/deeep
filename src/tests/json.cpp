// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "misc/json.h"
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
  catch(...)
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
  assertTrue(jsonOk("{}"));

  assertTrue(!jsonOk("{"));
}

unittest("Json parser: members")
{
  assertTrue(jsonOk("{ \"var\": 0 }"));
  assertTrue(jsonOk("{ \"var\": -10 }"));
  assertTrue(jsonOk("{ \"hello\": \"world\" }"));
  assertTrue(jsonOk("{ \"N1\": \"V1\", \"N2\": \"V2\" }"));

  assertTrue(!jsonOk("{ \"N1\" : : \"V2\" }"));
}

unittest("Json parser: booleans")
{
  assertTrue(jsonOk("{ \"var\": true }"));
  assertTrue(jsonOk("{ \"var\": false }"));

  {
    auto o = jsonParse("{ \"isCool\" : true }");
    auto s = o["isCool"];
    assertEquals((int)json::Value::Type::Boolean, (int)s.type);
    assertEquals(true, s.boolValue);
  }

  {
    auto o = jsonParse("{ \"isSlow\" : false }");
    auto s = o["isSlow"];
    assertEquals((int)json::Value::Type::Boolean, (int)s.type);
    assertEquals(false, s.boolValue);
  }
}

unittest("Json parser: non-zero terminated")
{
  json::parse("{ \"isCool\" : true } _invalid_json_token_", 19);
}

unittest("Json parser: arrays")
{
  assertTrue(jsonOk("{ \"A\": [] }"));
  assertTrue(jsonOk("{ \"A\": [ { }, { } ] }"));
  assertTrue(jsonOk("{ \"A\": [ \"hello\", \"world\" ] }"));

  assertTrue(!jsonOk("{ \"A\": [ }"));
  assertTrue(!jsonOk("{ \"A\": ] }"));
}

unittest("Json parser: returned value")
{
  {
    auto o = jsonParse("{}");
    assertEquals(0u, o.members.size());
  }
  {
    auto o = jsonParse("{ \"N\" : \"hello\"}");
    assertEquals(1u, o.members.size());
    auto s = o.members["N"];
    assertEquals(std::string("hello"), s.stringValue);
  }
  {
    auto o = jsonParse("{ \"N\" : -1234 }");
    assertEquals(1u, o.members.size());
    auto s = o.members["N"];
    assertEquals((int)json::Value::Type::Integer, (int)s.type);
    assertEquals(-1234, s.intValue);
  }
}

