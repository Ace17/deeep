#pragma once

#include "game/entity.h"
#include <memory>

static auto const ENTITY_TELEPORTER = "teleporter";
static auto const ENTITY_FRAGILE_DOOR = "fragile_door";

static auto const ENTITY_ENEMY_WHEEL = "enemy_wheel";
static auto const ENTITY_ENEMY_SPIDER = "enemy_spider";

static auto const ENTITY_BONUS_LIFE = "bonus_life";
static auto const ENTITY_UPGRADE_CLIMB = "upgrade_climb";
static auto const ENTITY_UPGRADE_SHOOT = "upgrade_shoot";
static auto const ENTITY_UPGRADE_DASH = "upgrade_dash";

std::unique_ptr<Entity> createEntity(string name);

