#pragma once

#include <memory>
#include "body.h"

struct IPhysics : IPhysicsProbe
{
  // called by game
  virtual void addBody(Body* body) = 0;
  virtual void removeBody(Body* body) = 0;
  virtual void checkForOverlaps() = 0;
  virtual void setEdifice(function<bool(IntBox)> isSolid) = 0;
};

unique_ptr<IPhysics> createPhysics();

