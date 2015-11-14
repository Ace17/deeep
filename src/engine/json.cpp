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

  while(getline(fp, r))
    r += line;

  return r;
}

unique_ptr<Object> json::load(string path)
{
  auto const text = read(path);
  return json::parseObject(text.c_str());
}

static
unique_ptr<Object> parseObject(Tokenizer& tk);

static
unique_ptr<Object> parseMember(Tokenizer& tk);

static
string expect(Tokenizer& tk, Token::Type type)
{
  auto front = tk.front();

  if(front.type != type)
  {
    stringstream msg;
    msg << "Unexpected token type: '" + front.lexem + "'";
    msg << " (" << front.type << ")";
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
  expect(tk, Token::LBRACE);
  int idx = 0;

  while(tk.front().type != Token::RBRACE)
  {
    if(idx > 0)
      expect(tk, Token::COMMA);

    parseMember(tk);
    ++idx;
  }

  expect(tk, Token::RBRACE);
  return nullptr;
}

unique_ptr<Object> parseMember(Tokenizer& tk)
{
  expect(tk, Token::STRING);
  expect(tk, Token::COLON);
  expect(tk, Token::STRING);
  return nullptr;
}

