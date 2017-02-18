/**
 * @brief Game logic
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
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
#include "entities/detector.h"
#include "game.h"
#include "sounds.h"
#include "models.h"
#include "room.h"

using namespace std;

static int const DETECTOR_ID_BOUNDARY = -2;

// from smarttiles
array<int, 4> computeTileFor(Matrix<int> const& m, int x, int y);

class Game : public Scene, public IGame
{
public:
  Game() : m_tiles(Size2i(1, 1))
  {
    m_shouldLoadLevel = true;
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

    checkCollisions();
    removeDeadThings();

    m_debug = c.debug;
  }

  virtual Span<const Resource> getSounds() const override
  {
    static const Resource sounds[] =
    {
      { SND_PAUSE, "res/sounds/pause.ogg" },
      { SND_CHIRP, "res/sounds/base.ogg" },
      { SND_FIRE, "res/sounds/fire.ogg" },
      { SND_JUMP, "res/sounds/jump.ogg" },
      { SND_BEEP, "res/sounds/beep.ogg" },
      { SND_LAND, "res/sounds/land.ogg" },
      { SND_SWITCH, "res/sounds/switch.ogg" },
      { SND_HURT, "res/sounds/hurt.ogg" },
      { SND_DIE, "res/sounds/die.ogg" },
      { SND_BONUS, "res/sounds/bonus.ogg" },
      { SND_DAMAGE, "res/sounds/damage.ogg" },
      { SND_TELEPORT, "res/sounds/teleport.ogg" },
      { SND_EXPLODE, "res/sounds/explode.ogg" },
    };

    return makeSpan(sounds);
  }

  virtual Span<const Resource> getModels() const override
  {
    static const Resource models[] =
    {
      { MDL_TILES, "res/tiles.mdl" },
      { MDL_DOOR, "res/sprites/door.json" },
      { MDL_RECT, "res/sprites/rect.json" },
      { MDL_SWITCH, "res/sprites/switch.json" },
      { MDL_ROCKMAN, "res/sprites/rockman.json" },
      { MDL_WHEEL, "res/sprites/wheel.json" },
      { MDL_LIFEBAR, "res/sprites/lifebar.json" },
      { MDL_TELEPORTER, "res/sprites/teleporter.json" },
      { MDL_BONUS, "res/sprites/bonus.json" },
      { MDL_BULLET, "res/sprites/bullet.json" },
      { MDL_EXPLOSION, "res/sprites/explosion.json" },
    };

    return makeSpan(models);
  }

  vector<Actor> getActors() const override
  {
    vector<Actor> r;

    auto cameraPos = m_player->pos;
    cameraPos.y += 1.5;

    {
      // prevent camera from going outside the level
      auto const limit = 7.0f;
      cameraPos.x = clamp(cameraPos.x, limit, m_tiles.size.width - limit);
      cameraPos.y = clamp(cameraPos.y, limit, m_tiles.size.height - limit);
    }

    addActorsForTileMap(r, cameraPos);

    Rect2f cameraRect;
    cameraRect.width = 16;
    cameraRect.height = 16;
    cameraRect.x = cameraPos.x - cameraRect.width / 2;
    cameraRect.y = cameraPos.y - cameraRect.height / 2;

    for(auto& entity : m_entities)
    {
      if(!overlaps(entity->getRect(), cameraRect))
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
      lifebar.scale = Vector2f(0.7, 3);
      r.push_back(lifebar);
    }

    return r;
  }

  vector<SOUND> readSounds() override
  {
    return std::move(m_sounds);
  }

  void addActorsForTileMap(vector<Actor>& r, Vector2f cameraPos) const
  {
    auto onCell = [&] (int x, int y, int tile)
                  {
                    if(!tile)
                      return;

                    if(abs(x - cameraPos.x) > 8)
                      return;

                    if(abs(y - cameraPos.y) > 8)
                      return;

                    auto composition = computeTileFor(m_tiles, x, y);

                    for(int subTile = 0; subTile < 4; ++subTile)
                    {
                      auto const ts = 1.0;
                      auto const posX = (x + (subTile % 2) * 0.5) * ts;
                      auto const posY = (y + (subTile / 2) * 0.5) * ts;
                      auto actor = Actor(Vector2f(posX, posY), MDL_TILES);
                      // actor.action = 0 * (tile - 1) * 16 + composition[subTile];
                      actor.action = ((m_level + 1) % 8) * 16 + composition[subTile];
                      actor.scale = Vector2f(0.5, 0.5);
                      r.push_back(actor);
                    }
                  };

    m_tiles.scan(onCell);
  }

  void checkCollisions()
  {
    for(auto p : allPairs((int)m_entities.size()))
    {
      auto& me = *m_entities[p.first];
      auto& other = *m_entities[p.second];

      auto bulletRect = me.getRect();
      auto enemyRect = other.getRect();

      if(overlaps(bulletRect, enemyRect))
      {
        if(other.collidesWith & me.collisionGroup)
          other.onCollide(&me);

        if(me.collidesWith & other.collisionGroup)
          me.onCollide(&other);
      }
    }
  }

  void removeDeadThings()
  {
    removeDeadEntities(m_entities);

    for(auto& spawned : m_spawned)
      m_entities.push_back(move(spawned));

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

    m_entities.clear();
    m_spawned.clear();
    m_listeners.clear();

    auto level = Graph_loadRoom(levelIdx, this);
    m_tiles = move(level.tiles);

    Vector2f nextPos;

    if(!m_player)
    {
      m_player = makeRockman().release();
      m_player->pos = Vector2f(level.start.x, level.start.y);
    }

    spawn(m_player);

    {
      auto f = bind(&Game::onEndLevel, this, std::placeholders::_1);
      m_ender = makeDelegator<EndLevelEvent>(f);
      subscribeForEvents(&m_ender);
    }

    {
      auto f = bind(&Game::onTouchLevelBoundary, this, std::placeholders::_1);
      m_levelBoundary = makeDelegator<TouchLevelBoundary>(f);
      subscribeForEvents(&m_levelBoundary);
    }

    {
      auto detector = make_unique<Detector>();
      detector->size = Size2f(m_tiles.size.width, 1);
      detector->pos = Vector2f(0, -1);
      detector->id = DETECTOR_ID_BOUNDARY;
      spawn(detector.release());

      m_oobDelegator = makeDelegator<TouchDetectorEvent>(&onTouchDetector);
      subscribeForEvents(&m_oobDelegator);
    }
  }

  static void onTouchDetector(const TouchDetectorEvent* event)
  {
    (void)event;

    if(event->whichOne == DETECTOR_ID_BOUNDARY)
      printf("out of bounds\n");
  }

  void onEndLevel(const EndLevelEvent* event)
  {
    (void)event;
    m_shouldLoadLevel = true;
    m_transform = Vector2f(0, 0);
    m_level++;
  }

  void onTouchLevelBoundary(const TouchLevelBoundary* event)
  {
    (void)event;
    m_shouldLoadLevel = true;
    m_transform = event->transform;
    m_level = event->targetLevel;
  }

  Int m_level = 1;
  Vector2f m_transform;
  Bool m_shouldLoadLevel;

  EventDelegator m_ender, m_levelBoundary;
  EventDelegator m_oobDelegator;

  ////////////////////////////////////////////////////////////////
  // IGame: game, as seen by the entities

  void playSound(SOUND sound) override
  {
    m_sounds.push_back(sound);
  }

  void spawn(Entity* e) override
  {
    e->game = this;
    m_spawned.push_back(unique(e));
  }

  bool isSolid(Vector2f pos) override
  {
    // entities
    {
      auto myRect = Rect2f(pos.x, pos.y, 0, 0);

      for(auto& entity : m_entities)
      {
        if(!entity->solid)
          continue;

        auto rect = entity->getRect();

        if(overlaps(rect, myRect))
          return true;
      }
    }

    // tiles
    {
      auto const x = (int)pos.x;
      auto const y = (int)pos.y;

      if(m_tiles.isInside(x, y))
      {
        if(m_tiles.get(x, y) != 0)
          return true;
      }
    }

    return false;
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

  Vector2f getPlayerPosition() override
  {
    return m_player->pos;
  }

  Player* m_player = nullptr;
  uvector<Entity> m_entities;
  uvector<Entity> m_spawned;

  set<IEventSink*> m_listeners;

  Matrix<int> m_tiles;
  vector<SOUND> m_sounds;
  bool m_debug;

  // static stuff

  static void removeDeadEntities(uvector<Entity>& entities)
  {
    auto oldEntities = std::move(entities);

    for(auto& entity : oldEntities)
    {
      if(!entity->dead)
        entities.push_back(move(entity));
    }
  }

  static Actor getDebugActor(Entity* entity)
  {
    auto rect = entity->getRect();
    auto r = Actor(Vector2f(rect.x, rect.y), MDL_RECT);
    r.scale.x = rect.width;
    r.scale.y = rect.height;
    return r;
  }
};

Scene* createGame(vector<string> args)
{
  auto r = make_unique<Game>();

  if(args.size() == 1)
    r->m_level = atoi(args[0].c_str());

  return r.release();
}

