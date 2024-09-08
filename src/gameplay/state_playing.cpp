// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Game logic

#include <cmath>
#include <cstring> // strlen
#include <map>

#include "base/error.h"
#include "base/logger.h"
#include "base/scene.h"
#include "base/util.h"

#include "collision_groups.h"
#include "entity_factory.h"
#include "game.h"
#include "load_quest.h"
#include "minimap_data.h"
#include "models.h" // MDL_TILES_00
#include "physics.h"
#include "player.h"
#include "presenter.h"
#include "quest.h"
#include "state_machine.h"
#include "toggle.h"
#include "variable.h"

extern const Vec2i CELL_SIZE;

namespace
{
Actor getDebugActor(Entity* entity)
{
  auto box = entity->getBox();
  auto r = Actor { box.pos, MDL_RECT };
  r.scale = box.size;
  r.zOrder = 10;
  return r;
}

struct EntityConfigImpl : IEntityConfig
{
  std::string getString(const char* varName, std::string defaultValue) override
  {
    auto i = values.find(varName);

    if(i == values.end())
      return defaultValue;

    return i->second;
  }

  int getInt(const char* varName, int defaultValue) override
  {
    auto i = values.find(varName);

    if(i == values.end())
      return defaultValue;

    return atoi(i->second.c_str());
  }

  std::map<std::string, std::string> values;
};

static
void spawnEntities(Room const& room, IGame* game)
{
  for(auto& spawner : room.spawners)
  {
    EntityConfigImpl config;
    config.values = spawner.config;

    auto entity = createEntity(spawner.name, &config);

    if(spawner.id)
      entity->id = spawner.id;

    entity->pos = spawner.pos;
    game->spawn(entity.release());
  }
}

struct InGameScene : Scene, private IGame
{
  InGameScene(IPresenter* view) :
    m_view(view)
  {
    m_shouldLoadLevel = true;
    m_shouldLoadVars = true;
    m_quest = loadQuest("res/quest.gz");

    {
      Vec2i questMapSize {};

      for(auto& r : m_quest.rooms)
      {
        const Vec2i roomTopRight = r.pos + r.size;
        questMapSize.x = std::max(questMapSize.x, roomTopRight.x);
        questMapSize.y = std::max(questMapSize.y, roomTopRight.y);
      }

      m_savedGame.exploredCells.resize(questMapSize);
    }
  }

  ~InGameScene()
  {
    if(m_player)
      m_player->leaveLevel();
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control c) override
  {
    loadLevelIfNeeded();

    // update explored map cells
    if(m_player)
    {
      Vec2i playerPos = m_quest.rooms[m_level].pos;
      playerPos.x += int(m_player->position().x) / CELL_SIZE.x;
      playerPos.y += int(m_player->position().y) / CELL_SIZE.y;

      m_savedGame.exploredCells.set(playerPos.x, playerPos.y, 2);
    }

    if(startButton.toggle(c.start))
    {
      MinimapData data {};
      data.quest = &m_quest;
      data.level = m_level;
      data.playerPos = m_player->position();
      data.exploredCells = &m_savedGame.exploredCells;
      return createPausedState(m_view, this, data);
    }

    m_player->think(c);

    updateEntities();

    processEvents();
    updateCamera();

    updateDebugFlag(c.debug);

    if(m_gameFinished)
    {
      std::unique_ptr<Scene> deleteMeOnReturn(this);
      return createEndingState(m_view);
    }

    return this;
  }

  void draw() override
  {
    if(!m_player)
      return;

    sendActorsForTileMap();

    std::vector<Actor> actors;

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
      Actor lifebar { Vector(-7, 1.5), MDL_LIFEBAR };
      lifebar.action = 0;
      lifebar.ratio = m_player->health();
      lifebar.scale = Size(1, 5);
      lifebar.screenRefFrame = true;
      lifebar.zOrder = 9;
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
      m_player->setPosition(m_player->position() + m_transform);
      m_shouldLoadLevel = false;
      setAmbientLight(0);
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

  void processEvents()
  {
    auto events = std::move(m_eventQueue);

    for(auto& event : events)
    {
      if(auto levelBoundaryEvent = event->as<TouchLevelBoundary>())
        onTouchLevelBoundary(levelBoundaryEvent);
      else if(event->as<SaveEvent>())
        onSaveEvent();
      else if(event->as<FinishGameEvent>())
        m_gameFinished = true;
    }
  }

  void updateCamera()
  {
    auto cameraPos = m_player->position();
    cameraPos.y += 1.5;

    // prevent camera from going outside the level
    auto const margin = Vec2f(8, 8);
    cameraPos.x = ::clamp(cameraPos.x, margin.x, m_currRoomSize.x * CELL_SIZE.x - margin.x);
    cameraPos.y = ::clamp(cameraPos.y, margin.y, m_currRoomSize.y * CELL_SIZE.y - margin.y);

    m_view->setCameraPos(cameraPos);
  }

  void sendActorsForTileMap() const
  {
    auto const model = MDL_TILES_00 + m_theme % 8;

    auto onCell =
      [&] (int x, int y, int tile)
      {
        if(tile == -1)
          return;

        {
          const float posX = x;
          const float posY = y;
          auto actor = Actor { Vector(posX, posY), model };
          actor.action = tile;
          actor.scale = UnitSize;
          actor.zOrder = -1;
          m_view->sendActor(actor);
        }
      };

    m_tilesForDisplay->scan(onCell);
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
      m_entities.push_back(std::move(spawned));
    }

    m_spawned.clear();
  }

  static bool isDead(std::unique_ptr<Entity> const& e)
  {
    return e->dead;
  }

  void loadLevel(int levelIdx)
  {
    ///////////////////////////////////////////////////////////////////////////
    // destroy current game arena
    ///////////////////////////////////////////////////////////////////////////
    if(m_player)
      m_player->leaveLevel();

    m_physics.reset();

    m_entities.clear();
    m_spawned.clear();

    if(m_shouldLoadVars)
    {
      m_vars.clear();

      for(auto& savedVar : m_savedGame.varValues)
        getVariable(savedVar.first)->set(savedVar.second);

      m_shouldLoadVars = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // create new game arena
    ///////////////////////////////////////////////////////////////////////////

    m_physics.reset(createPhysics());

    if(levelIdx < 0 || levelIdx >= (int)m_quest.rooms.size())
      throw Error("No such level");

    auto& level = m_quest.rooms[levelIdx];

    m_tilemapBody.solid = true;
    m_tilemapBody.collisionGroup = CG_WALLS;
    m_tilemapBody.shape = &m_tilemapShape;
    m_tilemapShape.tiles = &level.tiles;
    m_physics->addBody(&m_tilemapBody);

    spawnEntities(level, this);
    removeDeadThings();

    m_tilesForDisplay = &level.tilesForDisplay;
    m_currRoomSize = level.size;
    m_theme = level.theme;
    m_view->playMusic(level.theme);

    // load new background
    {
      char buffer[256];
      String path = format(buffer, "res/sprites/background-%02d.model", level.theme);
      m_view->preload({ ResourceType::Model, MDL_BACKGROUND, path });
    }

    if(!m_player)
    {
      EntityConfigImpl config;
      m_player = createHeroPlayer(this);
      m_player->setPosition(Vector(level.start.x, level.start.y));
      postEvent(std::make_unique<SaveEvent>());
    }

    m_player->enterLevel();
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

  std::map<int, std::unique_ptr<IVariable>> m_vars;
  std::vector<std::unique_ptr<Event>> m_eventQueue;

  ////////////////////////////////////////////////////////////////
  // IGame: game, as seen by the entities

  void playSound(SOUND sound) override
  {
    m_view->playSound(sound);
  }

  void stopMusic() override
  {
    m_view->stopMusic();
  }

  void spawn(Entity* e) override
  {
    m_spawned.push_back(unique(e));
  }

  void detach(Entity* e) override
  {
    auto releaseFromVector = [] (std::vector<std::unique_ptr<Entity>>& container, Entity* e)
      {
        for(auto it = container.begin(); it != container.end();)
        {
          if(it->get() == e)
          {
            it->release();
            it = container.erase(it);
          }
          else
          {
            ++it;
          }
        }
      };

    m_physics->removeBody(e);
    releaseFromVector(m_spawned, e);
    releaseFromVector(m_entities, e);
  }

  IVariable* getVariable(int name) override
  {
    if(!m_vars[name])
      m_vars[name] = std::make_unique<Variable>();

    return m_vars[name].get();
  }

  void postEvent(std::unique_ptr<Event> event) override
  {
    m_eventQueue.push_back(std::move(event));
  }

  Vector getPlayerPosition() override
  {
    return m_player->position();
  }

  struct SavedGame
  {
    int level = 0;
    Vector position = NullVector;
    std::map<int, int> varValues;
    Matrix2<int> exploredCells; // 0 unknown, 1 known, 2 explored
  };

  void onSaveEvent()
  {
    m_savedGame.level = m_level;
    m_savedGame.position = m_player->position();
    m_savedGame.position.y = round(m_savedGame.position.y) + 0.02;
    m_savedGame.varValues.clear();

    for(auto& var : m_vars)
      m_savedGame.varValues[var.first] = var.second->get();
  }

  void respawn() override
  {
    logMsg("Respawning!");
    m_level = m_savedGame.level;
    m_transform = m_savedGame.position - m_player->position();
    m_shouldLoadLevel = true;
    m_shouldLoadVars = true;
  }

  void textBox(char const* msg) override
  {
    String s;
    s.data = msg;
    s.len = strlen(msg);
    m_view->textBox(s);
  }

  void setAmbientLight(float light) override
  {
    m_view->setAmbientLight(light);
  }

  SavedGame m_savedGame;
  Quest m_quest;
  Vec2i m_currRoomSize {};
  Player* m_player = nullptr;
  IPresenter* const m_view;
  std::unique_ptr<IPhysics> m_physics;
  bool m_gameFinished = false;
  Body m_tilemapBody {};
  ShapeTilemap m_tilemapShape {};

  const Matrix2<int>* m_tilesForDisplay;
  bool m_debug;
  bool m_debugFirstTime = true;
  Toggle startButton;

  std::vector<std::unique_ptr<Entity>> m_entities;
  std::vector<std::unique_ptr<Entity>> m_spawned;
};
}

Scene* createPlayingStateAtLevel(IPresenter* view, int level)
{
  auto gameState = std::make_unique<InGameScene>(view);
  gameState->m_level = level;
  return gameState.release();
}

Scene* createPlayingState(IPresenter* view)
{
  return createPlayingStateAtLevel(view, 1);
}

