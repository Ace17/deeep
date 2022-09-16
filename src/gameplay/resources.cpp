/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "base/resource.h"
#include "base/span.h"
#include "models.h"
#include "sounds.h"

static const Resource resources[] =
{
  { ResourceType::Sound, SND_PAUSE, "res/sounds/pause.ogg" },
  { ResourceType::Sound, SND_CHIRP, "res/sounds/base.ogg" },
  { ResourceType::Sound, SND_DOOR, "res/sounds/door.ogg" },
  { ResourceType::Sound, SND_FIRE, "res/sounds/fire.ogg" },
  { ResourceType::Sound, SND_JUMP, "res/sounds/jump.ogg" },
  { ResourceType::Sound, SND_BEEP, "res/sounds/beep.ogg" },
  { ResourceType::Sound, SND_LAND, "res/sounds/land.ogg" },
  { ResourceType::Sound, SND_SWITCH, "res/sounds/switch.ogg" },
  { ResourceType::Sound, SND_HURT, "res/sounds/hurt.ogg" },
  { ResourceType::Sound, SND_DIE, "res/sounds/die.ogg" },
  { ResourceType::Sound, SND_BONUS, "res/sounds/bonus.ogg" },
  { ResourceType::Sound, SND_DAMAGE, "res/sounds/damage.ogg" },
  { ResourceType::Sound, SND_HATCH, "res/sounds/hatch.ogg" },
  { ResourceType::Sound, SND_TELEPORT, "res/sounds/teleport.ogg" },
  { ResourceType::Sound, SND_EXPLODE, "res/sounds/explode.ogg" },
  { ResourceType::Sound, SND_SAVEPOINT, "res/sounds/savepoint.ogg" },
  { ResourceType::Sound, SND_VICTORY, "res/sounds/victory.ogg" },

  { ResourceType::Model, MDL_ENGINE, "res/sprites/engine.model" },

  { ResourceType::Model, MDL_TILES_00, "res/tiles/tiles-00.tiles" },
  { ResourceType::Model, MDL_TILES_01, "res/tiles/tiles-01.tiles" },
  { ResourceType::Model, MDL_TILES_02, "res/tiles/tiles-02.tiles" },
  { ResourceType::Model, MDL_TILES_03, "res/tiles/tiles-03.tiles" },
  { ResourceType::Model, MDL_TILES_04, "res/tiles/tiles-04.tiles" },
  { ResourceType::Model, MDL_TILES_05, "res/tiles/tiles-05.tiles" },
  { ResourceType::Model, MDL_TILES_06, "res/tiles/tiles-06.tiles" },
  { ResourceType::Model, MDL_TILES_07, "res/tiles/tiles-07.tiles" },

  { ResourceType::Model, MDL_DOOR, "res/sprites/door.model" },
  { ResourceType::Model, MDL_SAVEPOINT, "res/sprites/savepoint.model" },
  { ResourceType::Model, MDL_BLOCK, "res/sprites/block.model" },
  { ResourceType::Model, MDL_BOOTUP, "res/sprites/bootup_logo.model" },
  { ResourceType::Model, MDL_SPLASH, "res/sprites/splash.model" },
  { ResourceType::Model, MDL_ENDING, "res/sprites/ending.model" },
  { ResourceType::Model, MDL_PAUSED, "res/sprites/pause_menu.model" },
  { ResourceType::Model, MDL_ELEVATOR, "res/sprites/elevator.model" },
  { ResourceType::Model, MDL_RECT, "res/sprites/rect.model" },
  { ResourceType::Model, MDL_SWITCH, "res/sprites/switch.model" },
  { ResourceType::Model, MDL_SWEEPER, "res/sprites/sweeper.model" },
  { ResourceType::Model, MDL_SPIDER, "res/sprites/spider.model" },
  { ResourceType::Model, MDL_ROCKMAN, "res/sprites/rockman.model" },
  { ResourceType::Model, MDL_WHEEL, "res/sprites/wheel.model" },
  { ResourceType::Model, MDL_LIFEBAR, "res/sprites/lifebar.model" },
  { ResourceType::Model, MDL_LADDER, "res/sprites/ladder.model" },
  { ResourceType::Model, MDL_TELEPORTER, "res/sprites/teleporter.model" },
  { ResourceType::Model, MDL_BONUS, "res/sprites/bonus.model" },
  { ResourceType::Model, MDL_BULLET, "res/sprites/bullet.model" },
  { ResourceType::Model, MDL_EXPLOSION, "res/sprites/explosion.model" },
  { ResourceType::Model, MDL_SPIKES, "res/sprites/spikes.model" },
  { ResourceType::Model, MDL_HOPPER, "res/sprites/hopper.model" },
  { ResourceType::Model, MDL_MINIMAP_BG, "res/sprites/minimap_menu.model" },
  { ResourceType::Model, MDL_MINIMAP_TILES, "res/sprites/minimap_tiles.model" },
};

Span<const Resource> getResources()
{
  return resources;
}

