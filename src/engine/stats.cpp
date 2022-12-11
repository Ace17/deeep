// immediate-mode gathering of named statistics.
// Values are smoothed over AVERAGE_PERIOD.
#include "stats.h"

#include "misc/time.h"
#include <map>
#include <vector>

namespace
{
constexpr double AVERAGE_PERIOD = 1.0;

struct StatTrack
{
  const char* name;
  std::map<double, double> timedValues;

  StatVal getCurrValue() const
  {
    if(timedValues.empty())
    {
      return { name, {} };
    }

    double average = 0;
    int count = 0;

    for(auto& i : timedValues)
    {
      average += i.second;
      ++count;
    }

    average /= count;
    return { name, (float)average };
  }

  void addValue(double time, double value)
  {
    auto lowerBound = time - AVERAGE_PERIOD;
    timedValues[time] = value;
    auto i = timedValues.begin();

    while(i != timedValues.end())
    {
      if(i->first > lowerBound)
        break;

      i = timedValues.erase(i);
    }
  }
};

std::vector<StatTrack> g_Tracks;
std::map<const char*, int> g_NameToTrackId;
}

void Stat(const char* name, float value)
{
  auto i = g_NameToTrackId.find(name);

  if(i == g_NameToTrackId.end())
  {
    g_NameToTrackId[name] = (int)g_Tracks.size();
    g_Tracks.push_back({});
    i = g_NameToTrackId.find(name);
  }

  auto timeNow = GetSteadyClockMs() / 1000.0;
  auto& track = g_Tracks[i->second];

  track.name = name;
  track.addValue(timeNow, value);
}

int getStatCount()
{
  return (int)g_Tracks.size();
}

StatVal getStat(int idx)
{
  return g_Tracks.at(idx).getCurrValue();
}

