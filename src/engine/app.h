// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/span.h"
#include <memory>

struct IApp
{
  virtual ~IApp() = default;
  virtual bool tick() = 0;
};

std::unique_ptr<IApp> createApp(Span<char*> args);

