/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// main FSM: dispatching between various game states
#include "state_machine.h"
#include "state_splash.h"
#include "state_game.h"

Scene* createGame(View* view, vector<string> args)
{
  auto fsm = make_unique<StateMachine>();

  auto gameState = make_unique<GameState>(view);

  if(args.size() == 1)
    gameState->m_level = atoi(args[0].c_str());

  auto splashState = make_unique<SplashState>(fsm.get());

  fsm->states.push_back(move(splashState));
  fsm->states.push_back(move(gameState));

  return fsm.release();
}

