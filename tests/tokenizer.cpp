#include "engine/tokenizer.h"
#include "tests/tests.h"
#include <vector>
using namespace std;

static
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

