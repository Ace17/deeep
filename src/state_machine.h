#pragma once

#include "base/scene.h"
#include <vector>
#include <memory>

struct StateMachine : Scene
{
  void next()
  {
    currIdx = (currIdx + 1) % states.size();
    auto current = states[currIdx].get();
    current->init();
    tick(Control {});
  }

  void tick(Control const& c) override
  {
    auto current = states[currIdx].get();
    current->tick(c);
  }

  vector<Actor> getActors() const override
  {
    auto current = states[currIdx].get();
    return current->getActors();
  }

  vector<unique_ptr<Scene>> states;
  int currIdx = 0;
};

