#pragma once

#include "base/scene.h"
#include "base/view.h"

struct Quest;

Scene* createSplashState(View* view);
Scene* createPausedState(View* view, Scene* sub, Quest* quest, int room);
Scene* createPlayingState(View* view);
Scene* createEndingState(View* view);
Scene* createPlayingStateAtLevel(View* view, int level);

