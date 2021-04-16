#pragma once

struct StatVal
{
  const char* name;
  float val;
};

void Stat(const char* name, float value);

int getStatCount();
StatVal getStat(int idx);

