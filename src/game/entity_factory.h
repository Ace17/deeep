#pragma once

#include "game/entity.h"
#include <memory>

static auto const ENTITY_TELEPORTER = "teleporter";
static auto const ENTITY_FRAGILE_DOOR = "fragile_door";

static auto const ENTITY_ENEMY_WHEEL = "wheel";
static auto const ENTITY_ENEMY_SPIDER = "spider";
static auto const ENTITY_ENEMY_HOPPER = "hopper";

static auto const ENTITY_BONUS_LIFE = "bonus_life";
static auto const ENTITY_UPGRADE_CLIMB = "upgrade_climb";
static auto const ENTITY_UPGRADE_SHOOT = "upgrade_shoot";
static auto const ENTITY_UPGRADE_DASH = "upgrade_dash";
static auto const ENTITY_UPGRADE_DJUMP = "upgrade_djump";

// e.g:
// createEntity("spider");
// createEntity("door(4)");
std::unique_ptr<Entity> createEntity(string name);

