#pragma once

struct StatVal
{
  const char* name;
  float val;
};

int getStatCount();
StatVal getStat(int idx);

struct Gauge
{
  Gauge(const char* name);

  const char* const name = "?";
  const Gauge* const next = nullptr;
  const int index;

  void operator = (float value);
};

