// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <cstdint>
#include <vector>
using namespace std;

#include "base/span.h"

vector<uint8_t> decompress(Span<const uint8_t> buffer);

