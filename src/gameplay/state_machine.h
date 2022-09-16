#pragma once

struct IPresenter;
struct Scene;
struct Quest;

Scene* createBootupState(IPresenter* view);
Scene* createSplashState(IPresenter* view);
Scene* createPausedState(IPresenter* view, Scene* sub, Quest* quest, int room);
Scene* createPlayingState(IPresenter* view);
Scene* createEndingState(IPresenter* view);
Scene* createPlayingStateAtLevel(IPresenter* view, int level);

