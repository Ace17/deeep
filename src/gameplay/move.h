// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "entity.h"

struct Trace
{
  bool horz;
  bool vert;
};

inline
Trace slideMove(Entity* ent, Vector vel)
{
  Trace r;

  r.horz = ent->physics->moveBody(ent, Vector(vel.x, 0)) == 1.0;
  r.vert = ent->physics->moveBody(ent, Vector(0, vel.y)) == 1.0;

  return r;
}

