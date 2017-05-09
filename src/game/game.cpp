/**
 * @brief Game logic
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <functional>
#include "base/scene.h"
#include "base/util.h"
#include "entities/player.h"
#include "entities/rockman.h"
#include "game.h"
#include "models.h" // MDL_LIFEBAR
#include "room.h"

using namespace std;

// from smarttiles
array<int, 4> computeTileFor(Matrix2<int> const& m, int x, int y);

// from physics.cpp
unique_ptr<IPhysics> createPhysics();

struct Game : Scene, IGame
{
  Game() : m_tiles(Size2i(1, 1))
  {
    m_shouldLoadLevel = true;
    m_physics = createPhysics();
    m_physics->setEdifice(bind(&Game::isRectSolid, this, placeholders::_1));
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control const& c) override
  {
    if(m_shouldLoadLevel)
    {
      loadLevel(m_level);
      m_player->pos += m_transform;
      m_shouldLoadLevel = false;
    }

    m_player->think(c);

    for(auto& e : m_entities)
      e->tick();

    m_physics->checkForOverlaps();
    removeDeadThings();

    m_debug = c.debug;

    if(c.debug && m_debugFirstTime)
    {
      m_debugFirstTime = false;
      m_player->addUpgrade(-1);
    }
  }

  vector<Actor> getActors() const override
  {
    vector<Actor> r;

    auto cameraPos = m_player->pos;
    cameraPos.y += 1.5;

    {
      // prevent camera from going outside the level
      auto const limit = 8.0f;
      cameraPos.x = clamp(cameraPos.x, limit, m_tiles.size.width - limit);
      cameraPos.y = clamp(cameraPos.y, limit, m_tiles.size.height - limit);
    }

    addActorsForTileMap(r, cameraPos);

    Box cameraRect;
    cameraRect.width = 16;
    cameraRect.height = 16;
    cameraRect.x = cameraPos.x - cameraRect.width / 2;
    cameraRect.y = cameraPos.y - cameraRect.height / 2;

    for(auto& entity : m_entities)
    {
      if(!overlaps(entity->getBox(), cameraRect))
        continue;

      r.push_back(entity->getActor());

      if(m_debug)
        r.push_back(getDebugActor(entity.get()));
    }

    for(auto& actor : r)
    {
      actor.pos -= cameraPos;
    }

    {
      Actor lifebar(Vector2f(-5.5, 1), MDL_LIFEBAR);
      lifebar.action = 0;
      lifebar.ratio = m_player->health();
      lifebar.scale = Size2f(0.7, 3);
      r.push_back(lifebar);
    }

    return r;
  }

  vector<SOUND> readSounds() override
  {
    return std::move(m_sounds);
  }

  int getMusic() const override
  {
    return m_theme;
  }

  void addActorsForTileMap(vector<Actor>& r, Vector2f cameraPos) const
  {
    auto onCell =
      [&] (int x, int y, int tile)
      {
        if(!tile)
          return;

        if(abs(x - cameraPos.x) > 9)
          return;

        if(abs(y - cameraPos.y) > 9)
          return;

        auto composition = computeTileFor(m_tiles, x, y);

        for(int subTile = 0; subTile < 4; ++subTile)
        {
          auto const ts = 1.0;
          auto const posX = (x + (subTile % 2) * 0.5) * ts;
          auto const posY = (y + (subTile / 2) * 0.5) * ts;
          auto actor = Actor(Vector2f(posX, posY), MDL_TILES);
          actor.action = (m_theme % 8) * 16 + composition[subTile];
          actor.scale = Size2f(0.5, 0.5);
          r.push_back(actor);
        }
      };

    m_tiles.scan(onCell);
  }

  void removeDeadThings()
  {
    removeDeadEntities(m_entities);

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

  void loadLevel(int levelIdx)
  {
    if(m_player)
    {
      for(auto& entity : m_entities)
        if(m_player == entity.get())
          entity.release();
    }

    m_physics->clearBodies();

    m_entities.clear();
    m_spawned.clear();
    m_listeners.clear();

    auto level = Graph_loadRoom(levelIdx, this);
    m_tiles = move(level.tiles);
    m_theme = level.theme;
    printf("Now in: %s\n", level.name.c_str());

    Vector2f nextPos;

    if(!m_player)
    {
      m_player = makeRockman().release();
      m_player->pos = Vector2f(level.start.x, level.start.y);
    }

    spawn(m_player);

    {
      auto f = bind(&Game::onTouchLevelBoundary, this, std::placeholders::_1);
      m_levelBoundary = makeDelegator<TouchLevelBoundary>(f);
      subscribeForEvents(&m_levelBoundary);
    }
  }

  void onTouchLevelBoundary(const TouchLevelBoundary* event)
  {
    (void)event;
    m_shouldLoadLevel = true;
    m_transform = event->transform;
    m_level = event->targetLevel;
  }

  int m_level = 1;
  int m_theme = 0;
  Vector2f m_transform;
  bool m_shouldLoadLevel = false;

  EventDelegator m_levelBoundary;

  ////////////////////////////////////////////////////////////////
  // IGame: game, as seen by the entities

  void playSound(SOUND sound) override
  {
    m_sounds.push_back(sound);
  }

  void spawn(Entity* e) override
  {
    m_spawned.push_back(unique(e));
  }

  void postEvent(unique_ptr<Event> event) override
  {
    for(auto& listener : m_listeners)
      listener->notify(event.get());
  }

  void subscribeForEvents(IEventSink* sink) override
  {
    m_listeners.insert(sink);
  }

  void unsubscribeForEvents(IEventSink* sink) override
  {
    m_listeners.erase(sink);
  }

  Vector2f getPlayerPosition() override
  {
    return m_player->pos;
  }

  Player* m_player = nullptr;
  uvector<Entity> m_entities;
  uvector<Entity> m_spawned;
  unique_ptr<IPhysics> m_physics;

  set<IEventSink*> m_listeners;

  Matrix2<int> m_tiles;
  vector<SOUND> m_sounds;
  bool m_debug;
  bool m_debugFirstTime = true;

  // static stuff

  void removeDeadEntities(uvector<Entity>& entities)
  {
    auto oldEntities = std::move(entities);

    for(auto& entity : oldEntities)
    {
      if(!entity->dead)
        entities.push_back(move(entity));
      else
      {
        entity->leave();
        m_physics->removeBody(entity.get());
      }
    }
  }

  static Actor getDebugActor(Entity* entity)
  {
    auto box = entity->getBox();
    auto r = Actor(Vector2f(box.x, box.y), MDL_RECT);
    r.scale = box;
    return r;
  }

  bool isRectSolid(Box box)
  {
    if(isPointSolid(Vector2f(box.x, box.y)))
      return true;

    if(isPointSolid(Vector2f(box.x, box.y + box.height)))
      return true;

    if(isPointSolid(Vector2f(box.x + box.width, box.y)))
      return true;

    if(isPointSolid(Vector2f(box.x + box.width, box.y + box.height)))
      return true;

    if(isPointSolid(Vector2f(box.x, box.y + box.height / 2)))
      return true;

    if(isPointSolid(Vector2f(box.x + box.width, box.y + box.height / 2)))
      return true;

    return false;
  }

  bool isPointSolid(Vector2f pos)
  {
    auto const x = (int)pos.x;
    auto const y = (int)pos.y;

    if(!m_tiles.isInside(x, y))
      return false;

    if(m_tiles.get(x, y) == 0)
      return false;

    return true;
  }
};

Scene* createGame(vector<string> args)
{
  auto r = make_unique<Game>();

  if(args.size() == 1)
    r->m_level = atoi(args[0].c_str());

  return r.release();
}

