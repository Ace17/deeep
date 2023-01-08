#include "logger.h"
#include <cstdarg>
#include <cstdio> // vsnprintf

void logMsg(String fmt, ...)
{
  char buffer[4096];

  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(buffer, sizeof(buffer) - 2, fmt.data, args);
  va_end(args);

  buffer[n] = '\n';

  // Send logs to the console
  fwrite(buffer, 1, n + 1, stdout);
  fflush(stdout);

  // Dump logs to a persistent log file.
  static FILE* fp = fopen("log.txt", "ab");

  if(fp)
  {
    fwrite(buffer, 1, n + 1, fp);
    fflush(fp);
  }
}

