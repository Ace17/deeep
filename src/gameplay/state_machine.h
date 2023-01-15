#pragma once

struct IPresenter;
struct Scene;
struct MinimapData;

Scene* createBootupState(IPresenter* view);
Scene* createSplashState(IPresenter* view);
Scene* createPausedState(IPresenter* view, Scene* sub, const MinimapData& minimapData);
Scene* createPlayingState(IPresenter* view);
Scene* createEndingState(IPresenter* view);
Scene* createPlayingStateAtLevel(IPresenter* view, int level);

