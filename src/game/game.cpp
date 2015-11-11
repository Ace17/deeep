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
#include "engine/scene.h"
#include "engine/raii.h"
#include "entities/player.h"
#include "game.h"
#include "sounds.h"
#include "models.h"

using namespace std;

MODEL g_beepModel;

array<int, 4> computeTileFor(Matrix<int> const& m, int x, int y);

class Game : public Scene, public IGame
{
public:
  Game() : m_tiles(16, 16)
  {
    m_player = new Player;
    m_player->game = this;
    m_entities.push_back(unique(m_player));

    auto fillCell = [] (int x, int y, int& tile)
                    {
                      tile = ((x / 2) * 31 + (y * y / 3) * 37) % 4;
                    };

    m_tiles.scan(fillCell);

    for(int y = 0; y < 4; ++y)
      for(int x = 0; x < 4; ++x)
        m_tiles.set(x, y, 1);

    for(int y = 0; y < 4; ++y)
      for(int x = 0; x < 4; ++x)
        m_tiles.set(12 + x, 12 + y, 2);
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control const& c) override
  {
    m_player->think(c);
    removeDeadThings();
  }

  virtual const Resource* getSounds() const override
  {
    static const Resource sounds[] =
    {
      { SND_CHIRP, "res/base.ogg" },
      { SND_BEEP, "res/beep.ogg" },
      { 0, nullptr },
    };

    return sounds;
  }

  virtual const Resource* getModels() const override
  {
    static const Resource models[] =
    {
      { MDL_BASE, "res/base.png" },
      { MDL_TILES, "res/tiles.mdl" },
      { 0, nullptr },
    };

    return models;
  }

  vector<Actor> getActors() const override
  {
    vector<Actor> r;

    auto onCell = [&] (int x, int y, int tile)
                  {
                    if(!tile)
                      return;

                    auto composition = computeTileFor(m_tiles, x, y);

                    for(int subTile = 0; subTile < 4; ++subTile)
                    {
                      auto const ts = 1.0;
                      auto const posX = (x * 2 + subTile % 2) * ts - 16;
                      auto const posY = (y * 2 + subTile / 2) * ts - 16;
                      auto actor = Actor(Vector2f(posX, posY), MDL_TILES);
                      actor.frame = (tile - 1) * 16 + composition[subTile];
                      actor.scale = Vector2f(0.5, 0.5);
                      r.push_back(actor);
                    }
                  };

    m_tiles.scan(onCell);

    for(auto& enemy : m_entities)
      r.push_back(enemy->getActor());

    return r;
  }

  vector<SOUND> readSounds() override
  {
    return std::move(m_sounds);
  }

private:
  void checkCollisions()
  {
    for(auto& pMe : m_entities)
    {
      auto& me = *pMe;

      for(auto& pOther : m_entities)
      {
        auto& other = *pOther;

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

  Player* m_player;
  uvector<Entity> m_entities;
  uvector<Entity> m_spawned;

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

