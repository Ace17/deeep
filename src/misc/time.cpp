#include "time.h"

#include <chrono>

int64_t GetSteadyClockMs()
{
  using namespace std::chrono;
  auto elapsedTime = steady_clock::now().time_since_epoch();
  return duration_cast<milliseconds>(elapsedTime).count();
}

