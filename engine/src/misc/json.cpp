// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Simplistic standalone JSON-parser

#include "json.h"

struct Token
{
  enum Type
  {
    EOF_ = 0,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    STRING,
    NUMBER,
    BOOLEAN,
    COLON,
    COMMA,
  };

  string lexem;
  Type type;
};

class Tokenizer
{
public:
  Tokenizer(const char* text_, size_t len)
  {
    text = text_;
    textEnd = text_ + len;
    decodeToken();
  }

  const Token& front() const
  {
    return curr;
  }

  bool empty() const
  {
    return curr.type == Token::EOF_;
  }

  void popFront()
  {
    decodeToken();
  }

private:
  void decodeToken()
  {
    while(whitespace(frontChar()))
      ++text;

    curr.lexem = "";
    switch(frontChar())
    {
    case '\0':
      curr.type = Token::EOF_;
      break;
    case '[':
      accept();
      curr.type = Token::LBRACKET;
      break;
    case ']':
      accept();
      curr.type = Token::RBRACKET;
      break;
    case '{':
      accept();
      curr.type = Token::LBRACE;
      break;
    case '}':
      accept();
      curr.type = Token::RBRACE;
      break;
    case ':':
      accept();
      curr.type = Token::COLON;
      break;
    case ',':
      accept();
      curr.type = Token::COMMA;
      break;
    case '"':
      accept();

      while(frontChar() != '"' && frontChar() != '\0')
      {
        if(frontChar() == '\\')
        {
          accept();
          curr.lexem.pop_back();

          // escape sequence
          accept();
        }
        else
        {
          accept();
        }
      }

      accept();

      curr.type = Token::STRING;
      curr.lexem = curr.lexem.substr(1, curr.lexem.size() - 2);
      break;
    case 't':
      curr.type = Token::BOOLEAN;
      expect('t');
      expect('r');
      expect('u');
      expect('e');
      break;

    case 'f':
      curr.type = Token::BOOLEAN;
      expect('f');
      expect('a');
      expect('l');
      expect('s');
      expect('e');
      break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
        curr.type = Token::NUMBER;

        if(frontChar() == '-')
          accept();

        while(isdigit(frontChar()))
          accept();

        break;
      }
    default:
      {
        string msg = "Unknown char '";
        msg += frontChar();
        msg += "'";
        throw runtime_error(msg);
      }
    }
  }

  void expect(char c)
  {
    if(frontChar() != c)
      throw runtime_error("Unexpected character");

    accept();
  }

  void accept()
  {
    curr.lexem += frontChar();
    ++text;
  }

  char frontChar() const
  {
    if(text >= textEnd)
      return 0;

    return *text;
  }

  static bool whitespace(char c)
  {
    return c == ' ' || c == '\n';
  }

  const char* text;
  const char* textEnd;
  Token curr;
};

using namespace json;

static unique_ptr<Object> parseObject(Tokenizer& tk);
static unique_ptr<Value> parseValue(Tokenizer& tk);
static unique_ptr<Value> parseArray(Tokenizer& tk);
static string expect(Tokenizer& tk, Token::Type type);

unique_ptr<Object> json::parse(const char* text, size_t len)
{
  unique_ptr<Object> r;
  Tokenizer tokenizer(text, len);
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
  else if(tk.front().type == Token::BOOLEAN)
  {
    auto r = make_unique<Boolean>();
    r->value = expect(tk, Token::BOOLEAN) == "true";
    return unique_ptr<Value>(r.release());
  }
  else if(tk.front().type == Token::NUMBER)
  {
    auto r = make_unique<Number>();
    r->value = atoi(expect(tk, Token::NUMBER).c_str());
    return unique_ptr<Value>(r.release());
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

static
string expect(Tokenizer& tk, Token::Type type)
{
  auto front = tk.front();

  if(front.type != type)
  {
    string msg;

    if(front.type == Token::EOF_)
      msg += "Unexpected end of file found";
    else
    {
      msg += "Unexpected token '" + front.lexem + "'";
      msg += " of type " + to_string(front.type);
      msg += " instead of " + to_string(type);
    }

    throw runtime_error(msg);
  }

  string r = front.lexem;
  tk.popFront();
  return r;
}

