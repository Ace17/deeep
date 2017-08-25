/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Game logic

#include <cmath>
#include <algorithm>
#include <array>
#include <set>
#include "base/scene.h"
#include "base/util.h"
#include "entities/player.h"
#include "entities/rockman.h"
#include "game.h"
#include "models.h" // MDL_TILES_00
#include "physics.h"
#include "room.h"

using namespace std;

// from smarttiles
array<int, 4> computeTileFor(Matrix2<int> const& m, int x, int y);

struct Game : Scene, IGame
{
  Game(View* view) :
    m_view(view),
    m_tiles(Size2i(1, 1))
  {
    m_shouldLoadLevel = true;
    resetPhysics();
  }

  void resetPhysics()
  {
    m_physics = createPhysics();
    m_physics->setEdifice(bind(&Game::isBoxSolid, this, placeholders::_1));
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

    Box cameraBox;
    cameraBox.size.width = 16;
    cameraBox.size.height = 16;
    cameraBox.pos.x = cameraPos.x - cameraBox.size.width / 2;
    cameraBox.pos.y = cameraPos.y - cameraBox.size.height / 2;

    for(auto& entity : m_entities)
    {
      if(!overlaps(entity->getFBox(), cameraBox))
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
      Actor lifebar(Vector(-5.5, 1), MDL_LIFEBAR);
      lifebar.action = 0;
      lifebar.ratio = m_player->health();
      lifebar.scale = Size(0.7, 3);
      r.push_back(lifebar);
    }

    return r;
  }

  void addActorsForTileMap(vector<Actor>& r, Vector cameraPos) const
  {
    auto const model = MDL_TILES_00 + m_theme % 8;

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
          auto actor = Actor(Vector(posX, posY), model);
          actor.action = composition[subTile];
          actor.scale = Size(0.5, 0.5);
          r.push_back(actor);
        }
      };

    m_tiles.scan(onCell);
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
    if(m_player)
    {
      for(auto& entity : m_entities)
        if(m_player == entity.get())
          entity.release();
    }

    resetPhysics();

    m_entities.clear();
    m_spawned.clear();
    m_listeners.clear();

    auto level = Graph_loadRoom(levelIdx, this);
    m_tiles = move(level.tiles);
    m_theme = level.theme;
    m_view->playMusic(level.theme);

    if(!m_player)
    {
      m_player = makeRockman().release();
      m_player->pos = Vector(level.start.x, level.start.y);
    }

    spawn(m_player);

    {
      auto f = bind(&Game::onTouchLevelBoundary, this, std::placeholders::_1);
      m_levelBoundary = makeDelegator<TouchLevelBoundary>(f);
      m_levelBoundarySubscription.reset();
      m_levelBoundarySubscription = subscribeForEvents(&m_levelBoundary);
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
  Vector m_transform;
  bool m_shouldLoadLevel = false;

  EventDelegator m_levelBoundary;
  unique_ptr<Handle> m_levelBoundarySubscription;

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

  void postEvent(unique_ptr<Event> event) override
  {
    for(auto& listener : m_listeners)
      listener->notify(event.get());
  }

  unique_ptr<Handle> subscribeForEvents(IEventSink* sink) override
  {
    auto unsubscribe = [&] () { m_listeners.erase(sink); };

    struct DestroyableHandle : Handle
    {
      ~DestroyableHandle() { f(); }

      function<void(void)> f;
    };

    auto r = make_unique<DestroyableHandle>();
    r->f = unsubscribe;
    m_listeners.insert(sink);
    return r;
  }

  Vector getPlayerPosition() override
  {
    return m_player->pos;
  }

  void textBox(char const* msg) override
  {
    m_view->textBox(msg);
  }

  void setAmbientLight(float light) override
  {
    ambientLight = light;
  }

  Player* m_player = nullptr;
  uvector<Entity> m_entities;
  uvector<Entity> m_spawned;
  View* const m_view;
  unique_ptr<IPhysics> m_physics;

  set<IEventSink*> m_listeners;

  Matrix2<int> m_tiles;
  bool m_debug;
  bool m_debugFirstTime = true;

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
        if(m_tiles.isInside(col, row) && m_tiles.get(col, row))
          return true;

    return false;
  }

  // static stuff

  static Actor getDebugActor(Entity* entity)
  {
    auto box = entity->getFBox();
    auto r = Actor(box.pos, MDL_RECT);
    r.scale = box.size;
    return r;
  }
};

Scene* createGame(View* view, vector<string> args)
{
  auto r = make_unique<Game>(view);

  if(args.size() == 1)
    r->m_level = atoi(args[0].c_str());

  return r.release();
}

