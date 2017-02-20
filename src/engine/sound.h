#pragma once

#include <string>

struct Audio
{
  virtual ~Audio()
  {
  };
  virtual void loadSound(int id, std::string path) = 0;
  virtual void playSound(int id) = 0;
};

