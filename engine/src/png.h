#pragma once

#include <cstdint>
#include <vector>
#include "base/span.h"

std::vector<uint8_t> decodePng(Span<const uint8_t> buffer, int& width, int& height);

