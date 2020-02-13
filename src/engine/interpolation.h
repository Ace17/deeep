#pragma once

#include "base/view.h"
#include <vector>

using namespace std;

struct ActorFrame
{
  int date;
  vector<Actor> actors;
};

ActorFrame interpolate(const ActorFrame& a, const ActorFrame& b, int dateNow);

