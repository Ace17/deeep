// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Pluggable entity factory, registration side: game-specific part

#include "entity_factory.h"

#include "switch.h"
#include "wheel.h"
#include "hopper.h"
#include "sweeper.h"
#include "detector.h"
#include "bonus.h"
#include "player.h"
#include "spikes.h"
#include "blocks.h"
#include "hatch.h"
#include "savepoint.h"
#include "moving_platform.h"
#include "conveyor.h"

using namespace std;

map<string, CreationFunc> getRegistry()
{
  map<string, CreationFunc> r;

  r["upgrade_climb"] = [] (EntityConfig &) { return makeBonus(4, UPGRADE_CLIMB, "jump while against wall"); };
  r["upgrade_shoot"] = [] (EntityConfig &) { return makeBonus(3, UPGRADE_SHOOT, "press Z"); };
  r["upgrade_dash"] = [] (EntityConfig &) { return makeBonus(5, UPGRADE_DASH, "press C while walking"); };
  r["upgrade_djump"] = [] (EntityConfig &) { return makeBonus(6, UPGRADE_DJUMP, "jump while airborne"); };
  r["upgrade_ball"] = [] (EntityConfig &) { return makeBonus(7, UPGRADE_BALL, "press down"); };
  r["upgrade_slide"] = [] (EntityConfig &) { return makeBonus(8, UPGRADE_SLIDE, "go against wall while falling"); };
  r["bonus_life"] = [] (EntityConfig &) { return makeBonus(0, 0, "life up"); };
  r["wheel"] = [] (EntityConfig &) { return make_unique<Wheel>(); };
  r["hopper"] = [] (EntityConfig &) { return make_unique<Hopper>(); };
  r["sweeper"] = [] (EntityConfig &) { return make_unique<Sweeper>(); };
  r["spider"] = [] (EntityConfig &) { extern unique_ptr<Entity> makeSpider(); return makeSpider(); };
  r["spikes"] = [] (EntityConfig &) { return make_unique<Spikes>(); };
  r["fragile_door"] = [] (EntityConfig &) { return makeBreakableDoor(); };
  r["fragile_block"] = [] (EntityConfig &) { return make_unique<FragileBlock>(); };
  r["hatch"] = [] (EntityConfig &) { return make_unique<Hatch>(); };
  r["savepoint"] = [] (EntityConfig &) { return make_unique<SavePoint>(); };
  r["crumble_block"] = [] (EntityConfig &) { return make_unique<Hatch>(); };
  r["door"] = [] (EntityConfig& args) { auto arg = atoi(args[0].c_str()); return makeDoor(arg); };
  r["switch"] = [] (EntityConfig& args) { auto arg = atoi(args[0].c_str()); return makeSwitch(arg); };
  r["moving_platform"] = [] (EntityConfig& args) { auto arg = atoi(args[0].c_str()); return make_unique<MovingPlatform>(arg); };
  r["elevator"] = [] (EntityConfig &) { return make_unique<Elevator>(); };
  r["conveyor"] = [] (EntityConfig &) { return make_unique<Conveyor>(); };
  r["blocker"] = [] (EntityConfig &) { return make_unique<RoomBoundaryBlocker>(-1); };
  r["room_boundary_detector"] = [] (EntityConfig& args) { int targetLevel = atoi(args[0].c_str()); Vector transform; transform.x = atoi(args[1].c_str()); transform.y = atoi(args[2].c_str()); return make_unique<RoomBoundaryDetector>(targetLevel, transform); };

  return r;
}

