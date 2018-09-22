#pragma once

#include <vector>

int decodePNG(std::vector<uint8_t>& out_image, unsigned long& width, unsigned long& height, const uint8_t* in_png, size_t in_size);

