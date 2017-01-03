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
#include "base/scene.h"
#include "base/util.h"
#include "entities/player.h"
#include "game.h"
#include "sounds.h"
#include "models.h"

using namespace std;

// from smarttiles
array<int, 4> computeTileFor(Matrix<int> const& m, int x, int y);

class Game : public Scene, public IGame
{
public:
  Game() : m_tiles(Size2i(1, 1))
  {
    m_level = -1;
    m_levelFinished = true;
  }

  void addRandomWidgets()
  {
    auto rect = [&] (Vector2i pos, Size2i size, int tile)
                {
                  for(int dy = 0; dy < size.height; ++dy)
                    for(int dx = 0; dx < size.width; ++dx)
                      m_tiles.set(dx + pos.x, dy + pos.y, tile);
                };

    auto isFull = [&] (Vector2i pos, Size2i size) -> bool
                  {
                    for(int dy = 0; dy < size.height; ++dy)
                      for(int dx = 0; dx < size.width; ++dx)
                        if(m_tiles.get(dx + pos.x, dy + pos.y) == 0)
                          return false;

                    return true;
                  };

    auto const maxX = m_tiles.size.width - 4;
    auto const maxY = m_tiles.size.height - 4;

    for(int i = 0; i < (maxX * maxY) / 100; ++i)
    {
      auto pos = Vector2i(rand() % maxX + 1, rand() % maxY + 1);
      auto size = Size2i(2, 2);

      if(isFull(pos + Vector2i(-1, -1), Size2i(size.width + 2, size.height + 2)))
        rect(pos, size, 3);
    }
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control const& c) override
  {
    if(m_levelFinished)
    {
      m_level++;
      loadLevel();
      m_levelFinished = false;
    }

    m_player->think(c);

    for(auto& e : m_entities)
      e->tick();

    checkCollisions();
    removeDeadThings();

    m_upgrades = m_player->getUpgrades();

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
      cameraPos.x = clamp(cameraPos.x, limit, m_tiles.size.width - limit - 1);
      cameraPos.y = clamp(cameraPos.y, limit, m_tiles.size.height - limit - 1);
    }

    addActorsForTileMap(r, cameraPos);

    for(auto& entity : retro(m_entities))
    {
      auto delta = entity->pos - cameraPos;

      if(abs(delta.x) > 8 || abs(delta.y) > 8)
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
      Actor lifebar(Vector2f(-4.5, 1), MDL_LIFEBAR);
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

private:
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
                      actor.action = (m_level % 2) * 16 + composition[subTile];
                      actor.scale = Vector2f(0.5, 0.5);
                      r.push_back(actor);
                    }
                  };

    m_tiles.scan(onCell);
  }

  void checkCollisions()
  {
    for(size_t i = 0; i < m_entities.size(); ++i)
    {
      auto& me = *m_entities[i];

      for(size_t j = i + 1; j < m_entities.size(); ++j)
      {
        auto& other = *m_entities[j];

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
  }

  void removeDeadThings()
  {
    removeDeadEntities(m_entities);

    for(auto& spawned : m_spawned)
      m_entities.push_back(move(spawned));

    m_spawned.clear();
  }

  void loadLevel()
  {
    m_entities.clear();
    m_spawned.clear();
    m_listeners.clear();
    m_player = nullptr;

    extern void loadLevel1(Matrix<int> &tiles, Vector2i & start, IGame* game);
    extern void loadLevel2(Matrix<int> &tiles, Vector2i & start, IGame* game);
    extern void loadLevel3(Matrix<int> &tiles, Vector2i & start, IGame* game);
    extern void loadLevel4(Matrix<int> &tiles, Vector2i & start, IGame* game);

    auto levels = vector<decltype(loadLevel1)*>(
    {
      &loadLevel4,
      &loadLevel3,
      &loadLevel1,
      // &loadLevel2,
    });

    auto setToOne = [&] (int, int, int& tile) { tile = 1; };

    m_tiles.scan(setToOne);

    Vector2i start;
    levels[m_level] (m_tiles, start, this);

    m_player = createRockman();
    m_player->addUpgrade(m_upgrades);
    m_player->pos = Vector2f(start.x, start.y);
    spawn(m_player);

    m_ender.game = this;
    listen(-1, &m_ender);

    addRandomWidgets();
  }

  struct LevelEnder : ITriggerable
  {
    Game* game;
    void trigger()
    {
      game->m_levelFinished = true;
    }
  };

  Int m_upgrades;
  Int m_level;
  Bool m_levelFinished;
  LevelEnder m_ender;

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

  void trigger(int triggerIdx) override
  {
    if(!exists(m_listeners, triggerIdx))
      return;

    for(auto& listener : m_listeners[triggerIdx])
      listener->trigger();
  }

  void listen(int triggerIdx, ITriggerable* triggerable) override
  {
    m_listeners[triggerIdx].push_back(triggerable);
  }

  Player* m_player;
  uvector<Entity> m_entities;
  uvector<Entity> m_spawned;

  map<int, vector<ITriggerable*>> m_listeners;

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

Scene* createGame()
{
  return new Game;
}

