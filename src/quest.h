#pragma once

#include <vector>
#include <string>

using namespace std;

struct Quest
{
  vector<Room> rooms;
};

Quest loadQuest(string path);

