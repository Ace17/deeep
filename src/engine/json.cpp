/*
 * Copyright (C) 2016 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "json.h"
#include "tokenizer.h"
#include <fstream>
#include <sstream>

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

  while(getline(fp, line))
    r += line;

  return r;
}

unique_ptr<Object> json::load(string path)
{
  auto const text = read(path);
  return json::parseObject(text.c_str());
}

static unique_ptr<Object> parseObject(Tokenizer& tk);
static unique_ptr<Value> parseValue(Tokenizer& tk);
static unique_ptr<Value> parseArray(Tokenizer& tk);

static
string expect(Tokenizer& tk, Token::Type type)
{
  auto front = tk.front();

  if(front.type != type)
  {
    stringstream msg;

    if(front.type == Token::EOF_)
      msg << "Unexpected end of file found";
    else
      msg << "Unexpected token type: '" + front.lexem + "'";

    msg << " (" << front.type << ")";
    msg << " expected " << type;
    throw runtime_error(msg.str());
  }

  string r = front.lexem;
  tk.popFront();
  return r;
}

unique_ptr<Object> json::parseObject(const char* text)
{
  unique_ptr<Object> r;
  Tokenizer tokenizer(text);
  return parseObject(tokenizer);
}

unique_ptr<Object> parseObject(Tokenizer& tk)
{
  auto r = make_unique<Object>();
  expect(tk, Token::LBRACE);
  int idx = 0;

  while(tk.front().type != Token::RBRACE)
  {
    if(idx > 0)
      expect(tk, Token::COMMA);

    auto const name = expect(tk, Token::STRING);
    expect(tk, Token::COLON);
    r->members[name] = parseValue(tk);
    ++idx;
  }

  expect(tk, Token::RBRACE);
  return r;
}

unique_ptr<Value> parseValue(Tokenizer& tk)
{
  if(tk.front().type == Token::LBRACKET)
  {
    return parseArray(tk);
  }
  else if(tk.front().type == Token::LBRACE)
  {
    return parseObject(tk);
  }
  else
  {
    auto r = make_unique<String>();
    r->value = expect(tk, Token::STRING);
    return unique_ptr<Value>(r.release());
  }
}

unique_ptr<Value> parseArray(Tokenizer& tk)
{
  auto r = make_unique<Array>();
  expect(tk, Token::LBRACKET);
  int idx = 0;

  while(tk.front().type != Token::RBRACKET)
  {
    if(idx > 0)
      expect(tk, Token::COMMA);

    r->elements.push_back(parseValue(tk));
    ++idx;
  }

  expect(tk, Token::RBRACKET);
  return unique_ptr<Value>(r.release());
}

