// immediate-mode gathering of named statistics.
// Values are smoothed over AVERAGE_PERIOD.
#include "stats.h"
#include "time.h"
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

std::vector<StatTrack>& g_Tracks()
{
  static std::vector<StatTrack> tracks;
  return tracks;
}
}

int getStatCount()
{
  return (int)g_Tracks().size();
}

StatVal getStat(int idx)
{
  return g_Tracks().at(idx).getCurrValue();
}

static const Gauge* gAllGauges;

Gauge::Gauge(const char* name) :
  name(name),
  next(gAllGauges),
  index(g_Tracks().size())
{
  g_Tracks().push_back({});

  gAllGauges = this;
}

void Gauge::operator = (float value)
{
  auto timeNow = GetSteadyClockMs() / 1000.0;
  auto& track = g_Tracks()[index];

  track.name = name;
  track.addValue(timeNow, value);
}

