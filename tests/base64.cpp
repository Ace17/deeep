#include "engine/base64.h"
#include "tests/tests.h"
#include <vector>
using namespace std;

#include <iostream>
unittest("Base64: simple")
{
  assertEquals(vector<uint8_t>({ }), decodeBase64(""));
  assertEquals(vector<uint8_t>({ 'a' }), decodeBase64("YQ=="));
  assertEquals(vector<uint8_t>({ 'C', 'o', 'o', 'l' }), decodeBase64("Q29vbA=="));
  assertEquals(vector<uint8_t>({ 'H', 'e', 'l', 'l', 'o' }), decodeBase64("SGVsbG8="));
}

