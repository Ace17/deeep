/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "engine/src/tokenizer.h"
#include "tests.h"
#include <vector>
using namespace std;

namespace
{
vector<Token::Type> tokenize(string s)
{
  Tokenizer tokenizer(s.c_str());
  vector<Token::Type> r;

  while(!tokenizer.empty())
  {
    r.push_back(tokenizer.front().type);
    tokenizer.popFront();
  }

  return r;
}

template<typename T>
vector<T> V(initializer_list<T> elements)
{
  return vector<T>(elements);
}
}

unittest("Tokenizer: empty")
{
  Tokenizer t("");
  assert(t.empty());
}

unittest("Tokenizer: simple")
{
  assertEquals(
    V({ Token::STRING, Token::COMMA, Token::STRING, Token::COLON }),
    tokenize("\"Hello\", \"world\":")
    );

  assertEquals(
    V({ Token::LBRACE, Token::RBRACE, Token::LBRACKET, Token::RBRACKET, Token::COMMA, Token::COLON }),
    tokenize("{}[],:")
    );

  {
    Tokenizer t("\"Hello\"");
    assert(!t.empty());
    assertEquals("Hello", t.front().lexem);
    t.popFront();
    assert(t.empty());
  }
}

unittest("Tokenizer: escaping")
{
  {
    auto tk = Tokenizer("\"hello/world\"");
    assertEquals("hello/world", tk.front().lexem);
  }

  {
    auto tk = Tokenizer("\"hello\\/world\"");
    assertEquals("hello/world", tk.front().lexem);
  }

  {
    auto tk = Tokenizer("\"hello\\/\\/world\"");
    assertEquals("hello//world", tk.front().lexem);
  }
}

