#pragma once

#include "game/entity.h"
#include <memory>

std::unique_ptr<Entity> makeBonus(int action, int upgradeType);

