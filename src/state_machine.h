#pragma once

#include "base/scene.h"
#include <map>
#include <memory>

struct StateMachine : Scene
{
  void next()
  {
    currIdx = (currIdx + 1) % states.size();
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

