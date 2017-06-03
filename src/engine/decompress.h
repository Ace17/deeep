#pragma once

#include <cstdint>
#include <vector>
using namespace std;

#include "base/span.h"

vector<uint8_t> decompress(Span<const uint8_t> buffer);

