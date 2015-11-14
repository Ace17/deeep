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

unittest("Tokenizer: simple")
{
  assertEquals(
      vector<Token::Type>({ Token::STRING, Token::COMMA, Token::STRING, Token::COLON }),
      tokenize("\"Hello\", \"world\":")
      );
}

