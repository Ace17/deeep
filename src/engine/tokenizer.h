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
      {
        if(*text == '\\')
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

        if(*text == '-')
          accept();

        while(isdigit(*text))
          accept();

        break;
      }
    default:
      {
        string msg = "Unknown char '";
        msg += *text;
        msg += "'";
        throw runtime_error(msg);
      }
    }
  }

  void expect(char c)
  {
    if(*text != c)
      throw runtime_error("Unexpected character");

    accept();
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

