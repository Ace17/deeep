#pragma once

#include <string>
#include <stdexcept>
using namespace std;

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
    COLON,
    COMMA,
  };

  string lexem;
  Type type;
};

class Tokenizer
{
public:
  Tokenizer(const char* text_)
  {
    text = text_;
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
    while(whitespace(*text))
      ++text;

    curr.lexem = "";
    switch(*text)
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

      while(*text != '"' && *text != '\0')
        accept();

      accept();

      curr.type = Token::STRING;
      break;
    default:
      {
        string msg = "Unknown char '";
        msg += *text;
        msg += "'";
        throw runtime_error(msg);
      }
    }
  }

  void accept()
  {
    curr.lexem += *text;
    ++text;
  }

  static bool whitespace(char c)
  {
    return c == ' ' || c == '\n';
  }

  const char* text;
  Token curr;
};

