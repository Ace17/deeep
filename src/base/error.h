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

