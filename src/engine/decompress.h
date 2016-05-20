#pragma once

#include <cstdint>
#include <vector>
using namespace std;

#include "base/util.h"

vector<uint8_t> decompress(Slice<const uint8_t> buffer);

