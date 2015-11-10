#pragma once

class Spawner : public Entity
{
public:
  Spawner(Entity* toSpawn, int delay = 0) :
    m_delay(delay),
    m_child(toSpawn)
  {
  }

  virtual void tick() override
  {
    if(lifetime > m_delay && !dead)
    {
      game->spawn(m_child);
      dead = true;
    }
  }

  virtual Actor getActor() const override
  {
    return Actor(Vector2f(-1000, 0)); // HACK: invisible entity
  }

private:
  const int m_delay;
  Entity* const m_child;
};

