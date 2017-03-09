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
  { MDL_TILES, "res/tiles.mdl" },
  { MDL_DOOR, "res/sprites/door.json" },
  { MDL_RECT, "res/sprites/rect.json" },
  { MDL_SWITCH, "res/sprites/switch.json" },
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

