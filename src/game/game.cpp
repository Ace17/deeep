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
    m_player = new Player;
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
      { SND_BEEP, "res/beep.ogg" },
      { SND_LAND, "res/land.ogg" },
      { SND_SWITCH, "res/switch.ogg" },
      { 0, nullptr },
    };

    return sounds;
  }

  virtual const Resource* getModels() const override
  {
    static const Resource models[] =
    {
      { MDL_BASE, "res/base.png" },
      { MDL_SWITCH, "res/switch.mdl" },
      { MDL_TILES, "res/tiles.mdl" },
      { 0, nullptr },
    };

    return models;
  }

  vector<Actor> getActors() const override
  {
    vector<Actor> r;

    auto const limit = 3.5f;
    Vector2f pov;
    pov.x = clamp(m_player->pos.x, limit, m_tiles.getWidth() - limit - 1);
    pov.y = clamp(m_player->pos.y, limit, m_tiles.getHeight() - limit - 1);

    auto onCell = [&] (int x, int y, int tile)
                  {
                    if(!tile)
                      return;

                    if(abs(x - pov.x) > 8)
                      return;

                    if(abs(y - pov.y) > 8)
                      return;

                    auto composition = computeTileFor(m_tiles, x, y);

                    for(int subTile = 0; subTile < 4; ++subTile)
                    {
                      auto const ts = 1.0;
                      auto const posX = (x + (subTile % 2) * 0.5) * ts;
                      auto const posY = (y + (subTile / 2) * 0.5) * ts;
                      auto actor = Actor(Vector2f(posX, posY), MDL_TILES);
                      actor.frame = (tile - 1) * 16 + composition[subTile];
                      actor.scale = Vector2f(0.5, 0.5);
                      r.push_back(actor);
                    }
                  };

    m_tiles.scan(onCell);

    for(auto& enemy : m_entities)
      r.push_back(enemy->getActor());

    for(auto& actor : r)
    {
      actor.pos.x -= pov.x;
      actor.pos.y -= pov.y;
    }

    return r;
  }

  vector<SOUND> readSounds() override
  {
    return std::move(m_sounds);
  }

private:
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
    auto const x = (int)pos.x;
    auto const y = (int)pos.y;

    if(!m_tiles.isInside(x, y))
      return false;

    return m_tiles.get(x, y) != 0;
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

