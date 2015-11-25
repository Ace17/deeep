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
#include "engine/scene.h"
#include "engine/util.h"
#include "entities/player.h"
#include "entities/wheel.h"
#include "game.h"
#include "sounds.h"
#include "models.h"

using namespace std;

array<int, 4> computeTileFor(Matrix<int> const& m, int x, int y);
void loadLevel(Matrix<int>& tiles, Vector2i& start, IGame* game, int number);

class Game : public Scene, public IGame
{
public:
  Game() : m_tiles(32 * 2, 32 * 2)
  {
    m_player = createRockman();
    m_player->pos = Vector2f(8, m_tiles.getHeight() - 2);
    spawn(m_player);

    auto onCell = [&] (int, int, int& tile)
                  {
                    tile = 1;
                  };

    m_tiles.scan(onCell);

    Vector2i start;
    loadLevel(m_tiles, start, this, 1);
    m_player->pos = Vector2f(start.x, start.y);

    {
      auto w = new Wheel;
      w->pos = Vector2f(17, m_tiles.getHeight() - 4);
      spawn(w);
    }

    addRandomWidgets();
  }

  void addRandomWidgets()
  {
    auto rect = [&] (Vector2i pos, Vector2i size, int tile)
                {
                  for(int dy = 0; dy < size.y; ++dy)
                    for(int dx = 0; dx < size.x; ++dx)
                      m_tiles.set(dx + pos.x, dy + pos.y, tile);
                };

    auto isFull = [&] (Vector2i pos, Vector2i size) -> bool
                  {
                    for(int dy = 0; dy < size.y; ++dy)
                      for(int dx = 0; dx < size.x; ++dx)
                        if(m_tiles.get(dx + pos.x, dy + pos.y) == 0)
                          return false;

                    return true;
                  };

    for(int i = 0; i < 1000; ++i)
    {
      auto const maxX = m_tiles.getWidth() - 4;
      auto const maxY = m_tiles.getHeight() - 4;
      auto pos = Vector2i(rand() % maxX + 1, rand() % maxY + 1);
      auto size = Vector2i(2, 2);

      if(isFull(pos + Vector2i(-1, -1), size + Vector2i(2, 2)))
        rect(pos, size, 3);
    }
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control const& c) override
  {
    m_player->think(c);

    for(auto& e : m_entities)
      e->tick();

    checkCollisions();
    removeDeadThings();
  }

  virtual const Resource* getSounds() const override
  {
    static const Resource sounds[] =
    {
      { SND_CHIRP, "res/base.ogg" },
      { SND_JUMP, "res/jump.ogg" },
      { SND_BEEP, "res/beep.ogg" },
      { SND_LAND, "res/land.ogg" },
      { SND_SWITCH, "res/switch.ogg" },
      { SND_HURT, "res/hurt.ogg" },
      { 0, nullptr },
    };

    return sounds;
  }

  virtual const Resource* getModels() const override
  {
    static const Resource models[] =
    {
      { MDL_BASE, "res/base.png" },
      { MDL_SWITCH, "res/switch.json" },
      { MDL_TILES, "res/tiles.mdl" },
      { MDL_ROCKMAN, "res/sprites/x/sprite.json" },
      { MDL_WHEEL, "res/sprites/wheel/sprite.json" },
      { MDL_LIFEBAR, "res/sprites/lifebar/sprite.json" },
      { 0, nullptr },
    };

    return models;
  }

  vector<Actor> getActors() const override
  {
    vector<Actor> r;

    auto const limit = 5.0f;
    auto cameraPos = m_player->pos;
    cameraPos.y += 1.5;
    cameraPos.x = clamp(cameraPos.x, limit, m_tiles.getWidth() - limit - 1);
    cameraPos.y = clamp(cameraPos.y, limit, m_tiles.getHeight() - limit - 1);

    addActorsForTileMap(r, cameraPos);

    for(auto& entity : retro(m_entities))
      r.push_back(entity->getActor());

    for(auto& actor : r)
    {
      actor.pos -= cameraPos;
    }

    {
      Actor lifebar(Vector2f(-4.5, 1), MDL_LIFEBAR);
      lifebar.action = 1;
      lifebar.ratio = 0;
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
                      actor.action = (tile - 1) * 16 + composition[subTile];
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
          other.onCollide(&me);
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
};

Scene* createGame()
{
  return new Game;
}

