// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"

struct Conveyor : Entity
{
  Conveyor()
  {
    size = UnitSize;
    collisionGroup = CG_WALLS;
    collidesWith = CG_PLAYER;
    solid = 1;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_RECT };
    r.action = 2;
    r.scale = size;
    r.scale.width *= -1;
    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    // avoid infinite recursion
    // (if the conveyor pushes the player towards the conveyor)
    if(noRecurse)
      return;

    noRecurse = true;
    physics->moveBody(other, Vector(-0.004, 0));
    noRecurse = false;
  }

  bool noRecurse = false;
};

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("conveyor", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<Conveyor>(); });

