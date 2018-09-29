#pragma once

#include "base/scene.h"
#include <vector>
#include <memory>

using namespace std;

struct StateMachine : Scene
{
  void next()
  {
    currIdx = (currIdx + 1) % states.size();
    auto current = states[currIdx].get();
    current->init();
    tick(Control {});
  }

  void tick(Control c) override
  {
    auto current = states[currIdx].get();
    current->tick(c);
  }

  vector<unique_ptr<Scene>> states;
  int currIdx = 0;
};

