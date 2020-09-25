// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

struct RateCounter
{
public:
  RateCounter()
  {
    m_numTicks = 0;
    m_lastTime = 0;
    m_currSlope = 0;
  }

  void tick(int timeMs)
  {
    ++m_numTicks;

    if(timeMs - m_lastTime > 1000)
    {
      m_currSlope = m_numTicks;
      m_numTicks = 0;
      m_lastTime = timeMs;
    }
  }

  int slope() const
  {
    return m_currSlope;
  }

private:
  int m_lastTime;
  int m_numTicks;
  int m_currSlope;
};

