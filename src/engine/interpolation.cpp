#include "interpolation.h"
#include <math.h>
#include <stdio.h>

namespace
{
template<typename T>
T lerp(T a, T b, float alpha)
{
  return a * (1 - alpha) + b * alpha;
}

Actor lerp(const Actor& a, const Actor& b, float lerpRatio)
{
  assert(a.id == b.id);
  Actor result = b;
  result.pos = lerp(a.pos, b.pos, lerpRatio);
  result.ratio = lerp(a.ratio, b.ratio, lerpRatio);
  return result;
}
}

ActorFrame interpolate(const ActorFrame& a, const ActorFrame& b, int dateNow)
{
  assert(a.date <= b.date);
  ActorFrame result;
  result.date = dateNow;
  const float lerpRatio = float(dateNow - a.date) / float(b.date - a.date);

  if(0)
    fprintf(stderr, "lerpRatio=%.2f (dateNow=%d, a.date=%d, b.date=%d\n", lerpRatio, dateNow, a.date, b.date);

  // assume that both actor lists are sorted by id, try to match them

  int idxA = 0;
  int idxB = 0;

  while(idxA < (int)a.actors.size() || idxB < (int)b.actors.size())
  {
    if(idxB >= (int)b.actors.size())
      result.actors.push_back(a.actors[idxA++]);
    else if(idxA >= (int)a.actors.size())
      result.actors.push_back(b.actors[idxB++]);
    else
    {
      if(a.actors[idxA].id == b.actors[idxB].id)
      {
        result.actors.push_back(lerp(a.actors[idxA], b.actors[idxB], lerpRatio));
        ++idxA;
        ++idxB;
      }
      else if(a.actors[idxA].id < b.actors[idxB].id)
      {
        result.actors.push_back(a.actors[idxA]);
        ++idxA;
      }
      else // assert(a.actors[idxA].id > b.actors[idxB].id)
      {
        result.actors.push_back(b.actors[idxB]);
        ++idxB;
      }
    }
  }

  return result;
}

