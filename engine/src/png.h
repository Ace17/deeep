#pragma once

#include "base/span.h"
#include <vector>

std::vector<uint8_t> decodePng(Span<const uint8_t> buffer, int& width, int& height);

