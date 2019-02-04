#pragma once

#include "base/scene.h"
#include "base/view.h"

Scene* createSplashState(View* view);
Scene* createPausedState(View* view, Scene* sub);
Scene* createPlayingState(View* view);
Scene* createPlayingStateAtLevel(View* view, int level);

