#pragma once

#include "base/span.h"
#include <cstdint>
#include <vector>

std::vector<uint8_t> decodeJpg(Span<const uint8_t> buffer, int& width, int& height);

