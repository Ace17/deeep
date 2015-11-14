#include "engine/tokenizer.h"
#include "tests/tests.h"

unittest("Tokenizer: simple")
{
  Tokenizer tokenizer("\"Hello\", \"world\";");
}

