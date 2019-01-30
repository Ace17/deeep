// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Game logic

#include <array>
#include <list>
#include <map>

#include "base/scene.h"
#include "base/view.h"
#include "base/util.h"

#include "entity_factory.h"
#include "entities/player.h"
#include "entities/rockman.h"
#include "game.h"
#include "models.h" // MDL_TILES_00
#include "physics.h"
#include "quest.h"
#include "load_quest.h"
#include "preprocess_quest.h"
#include "variable.h"
#include "state_machine.h"

using namespace std;

// from smarttiles
array<int, 4> computeTileFor(Matrix2<int> const& m, int x, int y);

static
void spawnEntities(Room const& room, IGame* game, int levelIdx)
{
  // avoid collisions between static entities from different rooms
  int id = levelIdx * 1000;

  for(auto& spawner : room.spawners)
  {
    auto entity = createEntity(spawner.name);
    entity->id = id;
    entity->pos = spawner.pos;
    game->spawn(entity.release());

    ++id;
  }
}

struct GameState : Scene, private IGame
{
  GameState(View* view) :
    m_view(view)
  {
    m_shouldLoadLevel = true;
    m_shouldLoadVars = true;
    m_quest = loadQuest("res/quest.json");
    preprocessQuest(m_quest);
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control c) override
  {
    loadLevelIfNeeded();

    m_player->think(c);

    for(int i = 0; i < 10; ++i)
      updateEntities();

    updateCamera();

    updateDebugFlag(c.debug);
    sendActors();
  }

  void sendActors() const
  {
    sendActorsForTileMap();

    vector<Actor> actors;

    for(auto& entity : m_entities)
    {
      actors.clear();
      entity->addActors(actors);

      for(auto actor : actors)
        m_view->sendActor(actor);

      if(m_debug)
        m_view->sendActor(getDebugActor(entity.get()));
    }

    {
      Actor lifebar { Vector(-7, 3.5), MDL_LIFEBAR };
      lifebar.action = 0;
      lifebar.ratio = m_player->health();
      lifebar.scale = Size(0.7, 3);
      lifebar.screenRefFrame = true;
      lifebar.zOrder = 10;
      m_view->sendActor(lifebar);
    }

    {
      Actor background = { Vector(-8, -8), MDL_BACKGROUND };
      background.scale = Size(16, 16);
      background.screenRefFrame = true;
      background.zOrder = -2;
      m_view->sendActor(background);
    }
  }

  ////////////////////////////////////////////////////////////////
  // internals

  void loadLevelIfNeeded()
  {
    if(m_shouldLoadLevel)
    {
      loadLevel(m_level);
      m_player->pos += m_transform;
      m_shouldLoadLevel = false;
    }
  }

  void updateDebugFlag(float debugFlag)
  {
    m_debug = debugFlag;

    if(debugFlag && m_debugFirstTime)
    {
      m_debugFirstTime = false;
      m_player->addUpgrade(-1);
    }
  }

  void updateEntities()
  {
    for(auto& e : m_entities)
      e->tick();

    m_physics->checkForOverlaps();
    removeDeadThings();
  }

  void updateCamera()
  {
    auto cameraPos = m_player->pos;
    cameraPos.y += 1.5;

    // prevent camera from going outside the level
    auto const limit = 8.0f;
    cameraPos.x = clamp(cameraPos.x, limit, m_tiles->size.width - limit);
    cameraPos.y = clamp(cameraPos.y, limit, m_tiles->size.height - limit);

    m_view->setCameraPos(cameraPos);
  }

  void sendActorsForTileMap() const
  {
    auto const model = MDL_TILES_00 + m_theme % 8;

    auto onCell =
      [&] (int x, int y, int tile)
      {
        if(!tile)
          return;

        auto composition = computeTileFor(*m_tiles, x, y);

        for(int subTile = 0; subTile < 4; ++subTile)
        {
          auto const ts = 1.0;
          auto const posX = (x + (subTile % 2) * 0.5) * ts;
          auto const posY = (y + (subTile / 2) * 0.5) * ts;
          auto actor = Actor { Vector(posX, posY), model };
          actor.action = composition[subTile];
          actor.scale = Size(0.5, 0.5);
          actor.zOrder = -1;
          m_view->sendActor(actor);
        }
      };

    m_tiles->scan(onCell);
  }

  void removeDeadThings()
  {
    for(auto& entity : m_entities)
    {
      if(entity->dead)
      {
        entity->leave();
        m_physics->removeBody(entity.get());
      }
    }

    unstableRemove(m_entities, &isDead);

    for(auto& spawned : m_spawned)
    {
      spawned->game = this;
      spawned->physics = m_physics.get();
      spawned->enter();

      m_physics->addBody(spawned.get());
      m_entities.push_back(move(spawned));
    }

    m_spawned.clear();
  }

  static bool isDead(unique_ptr<Entity> const& e)
  {
    return e->dead;
  }

  void loadLevel(int levelIdx)
  {
    ///////////////////////////////////////////////////////////////////////////
    // destroy current game arena
    ///////////////////////////////////////////////////////////////////////////
    if(m_player)
    {
      for(auto& entity : m_entities)
        if(m_player == entity.get())
          entity.release();
    }

    m_physics.reset();

    m_entities.clear();
    m_spawned.clear();

    if(m_shouldLoadVars)
    {
      m_vars.clear();

      for(auto& savedVar : m_savedVars)
        getVariable(savedVar.first)->set(savedVar.second);

      m_shouldLoadVars = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // create new game arena
    ///////////////////////////////////////////////////////////////////////////

    m_physics = createPhysics();
    m_physics->setEdifice(bind(&GameState::isBoxSolid, this, placeholders::_1));

    if(levelIdx < 0 || levelIdx >= (int)m_quest.rooms.size())
      throw runtime_error("No such level");

    auto& level = m_quest.rooms[levelIdx];
    spawnEntities(level, this, levelIdx);
    m_tiles = &level.tiles;
    m_theme = level.theme;
    m_view->playMusic(level.theme);

    // load new background
    {
      char buffer[256];
      sprintf(buffer, "res/sprites/background-%02d.model", level.theme);
      m_view->preload({ ResourceType::Model, MDL_BACKGROUND, buffer });
    }

    if(!m_player)
    {
      m_player = makeRockman().release();
      m_player->pos = Vector(level.start.x, level.start.y);
      savepoint();
    }

    spawn(m_player);
  }

  void onTouchLevelBoundary(const TouchLevelBoundary* event)
  {
    m_shouldLoadLevel = true;
    m_transform = event->transform;
    m_level = event->targetLevel;
  }

  int m_level = 1;
  int m_theme = 0;
  Vector m_transform;
  bool m_shouldLoadLevel = false;
  bool m_shouldLoadVars = false;

  map<int, unique_ptr<IVariable>> m_vars;

  ////////////////////////////////////////////////////////////////
  // IGame: game, as seen by the entities

  void playSound(SOUND sound) override
  {
    m_view->playSound(sound);
  }

  void spawn(Entity* e) override
  {
    m_spawned.push_back(unique(e));
  }

  IVariable* getVariable(int name) override
  {
    if(!m_vars[name])
      m_vars[name] = make_unique<Variable>();

    return m_vars[name].get();
  }

  void postEvent(unique_ptr<Event> event) override
  {
    if(auto levelBoundaryEvent = event->as<TouchLevelBoundary>())
      onTouchLevelBoundary(levelBoundaryEvent);
  }

  Vector getPlayerPosition() override
  {
    return m_player->pos;
  }

  int m_savedLevel = 0;
  Vector m_savedPos = NullVector;
  map<int, int> m_savedVars;

  void savepoint() override
  {
    m_savedLevel = m_level;
    m_savedPos = m_player->pos;
    m_savedVars.clear();

    for(auto& var : m_vars)
      m_savedVars[var.first] = var.second->get();
  }

  void respawn() override
  {
    printf("Respawning!\n");
    m_level = m_savedLevel;
    m_transform = m_savedPos - m_player->pos + Vector(0, 0.01);
    m_shouldLoadLevel = true;
    m_shouldLoadVars = true;
  }

  void textBox(char const* msg) override
  {
    m_view->textBox(msg);
  }

  void setAmbientLight(float light) override
  {
    m_view->setAmbientLight(light);
  }

  Quest m_quest;
  Player* m_player = nullptr;
  View* const m_view;
  unique_ptr<IPhysics> m_physics;

  const Matrix2<int>* m_tiles;
  bool m_debug;
  bool m_debugFirstTime = true;

  vector<unique_ptr<Entity>> m_entities;
  vector<unique_ptr<Entity>> m_spawned;

  bool isBoxSolid(IntBox box)
  {
    auto const x1 = box.pos.x;
    auto const y1 = box.pos.y;
    auto const x2 = box.pos.x + box.size.width;
    auto const y2 = box.pos.y + box.size.height;

    auto const col1 = x1 / PRECISION;
    auto const col2 = x2 / PRECISION;
    auto const row1 = y1 / PRECISION;
    auto const row2 = y2 / PRECISION;

    for(int row = row1; row <= row2; row++)
      for(int col = col1; col <= col2; col++)
        if(m_tiles->isInside(col, row) && m_tiles->get(col, row))
          return true;

    return false;
  }

  // static stuff

  static Actor getDebugActor(Entity* entity)
  {
    auto box = entity->getFBox();
    auto r = Actor { box.pos, MDL_RECT };
    r.scale = box.size;
    r.zOrder = 10;
    return r;
  }
};

unique_ptr<Scene> createPlayingState(StateMachine* fsm, View* view, int level)
{
  (void)fsm;
  auto gameState = make_unique<GameState>(view);
  gameState->m_level = level;
  return gameState;
}
