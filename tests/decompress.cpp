#include "engine/base64.h"
#include "engine/decompress.h"
#include "tests/tests.h"
#include <vector>
using namespace std;

unittest("Decompress: simple")
{
  auto toString = [] (vector<uint8_t> s)
                  {
                    string r;

                    for(auto c : s)
                      r.push_back(c);

                    return r;
                  };

  const uint8_t input[] =
  {
    0x78, 0x9C, 0xF3, 0x48, 0xCD, 0xC9, 0xC9, 0xD7,
    0x51, 0x28, 0xCF, 0x2F, 0xCA, 0x49, 0x01, 0x00,
    0x1B, 0xD4, 0x04, 0x69,
  };
  assertEquals("Hello, world",
               toString(decompress(makeSpan(input))));
}

unittest("Decompress: big")
{
  // "[AAA...AAAA]" (126 'A')
  const uint8_t input[] =
  {
    0x78, 0x01, 0x8B, 0x76, 0x1C, 0x50, 0x10, 0x0B,
    0x00, 0x3E, 0x54, 0x20, 0xB7,
  };

  auto const output = decompress(makeSpan(input));
  assertEquals(128u, output.size());
  assertEquals('[', output[0]);
  assertEquals('A', output[1]);
  assertEquals('A', output[80]);
  assertEquals('A', output[126]);
  assertEquals(']', output[127]);
}

