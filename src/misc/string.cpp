#include "base/string.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

String format(Span<char> buf, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf.data, buf.len - 1, fmt, args);
  va_end(args);
  const int len = strlen(buf.data);
  buf.data[len] = 0;

  String r;
  r.data = buf.data;
  r.len = len;
  return r;
}

