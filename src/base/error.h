// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Our base exception type.

#pragma once

#include "string.h"
#include <string>

struct Error
{
  Error(String msg_)
  {
    for(int i = 0; i < msg_.len; ++i)
      buffer[i] = msg_[i];

    msg.data = buffer;
    msg.len = msg_.len;
  }

  String message() const { return msg; }

  String msg;
  char buffer[4096];
};

