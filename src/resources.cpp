/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "base/resource.h"
#include "sounds.h"
#include "models.h"

static const Resource sounds[] =
{
  { SND_PAUSE, "res/sounds/pause.ogg" },
  { SND_CHIRP, "res/sounds/base.ogg" },
  { SND_FIRE, "res/sounds/fire.ogg" },
  { SND_JUMP, "res/sounds/jump.ogg" },
  { SND_BEEP, "res/sounds/beep.ogg" },
  { SND_LAND, "res/sounds/land.ogg" },
  { SND_SWITCH, "res/sounds/switch.ogg" },
  { SND_HURT, "res/sounds/hurt.ogg" },
  { SND_DIE, "res/sounds/die.ogg" },
  { SND_BONUS, "res/sounds/bonus.ogg" },
  { SND_DAMAGE, "res/sounds/damage.ogg" },
  { SND_TELEPORT, "res/sounds/teleport.ogg" },
  { SND_EXPLODE, "res/sounds/explode.ogg" },
};

static const Resource models[] =
{
  { MDL_TILES_00, "res/tiles/tiles-00.mdl" },
  { MDL_TILES_01, "res/tiles/tiles-01.mdl" },
  { MDL_TILES_02, "res/tiles/tiles-02.mdl" },
  { MDL_TILES_03, "res/tiles/tiles-03.mdl" },
  { MDL_TILES_04, "res/tiles/tiles-04.mdl" },
  { MDL_TILES_05, "res/tiles/tiles-05.mdl" },
  { MDL_TILES_06, "res/tiles/tiles-06.mdl" },
  { MDL_TILES_07, "res/tiles/tiles-07.mdl" },

  { MDL_DOOR, "res/sprites/door.json" },
  { MDL_BLOCK, "res/sprites/block.json" },
  { MDL_ELEVATOR, "res/sprites/elevator.json" },
  { MDL_RECT, "res/sprites/rect.json" },
  { MDL_SWITCH, "res/sprites/switch.json" },
  { MDL_SWEEPER, "res/sprites/sweeper.json" },
  { MDL_SPIDER, "res/sprites/spider.json" },
  { MDL_ROCKMAN, "res/sprites/rockman.json" },
  { MDL_WHEEL, "res/sprites/wheel.json" },
  { MDL_LIFEBAR, "res/sprites/lifebar.json" },
  { MDL_TELEPORTER, "res/sprites/teleporter.json" },
  { MDL_BONUS, "res/sprites/bonus.json" },
  { MDL_BULLET, "res/sprites/bullet.json" },
  { MDL_EXPLOSION, "res/sprites/explosion.json" },
  { MDL_SPIKES, "res/sprites/spikes.json" },
};

Span<const Resource> getSounds()
{
  return makeSpan(sounds);
}

Span<const Resource> getModels()
{
  return makeSpan(models);
}

