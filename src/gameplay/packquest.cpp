#include "base/error.h"
#include "load_quest.h"
#include "preprocess_quest.h"

#include <stdio.h>

Quest loadTmxQuest(string path);
void dumpQuest(Quest const& q, const char* filename);

int main(int argc, const char* argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s <quest.json> <packedquest.json>\n", argv[0]);
    return 1;
  }

  auto q = loadTmxQuest(argv[1]);
  preprocessQuest(q);
  dumpQuest(q, argv[2]);
  return 0;
}

std::string serializeMatrix(Matrix2<int> const& m)
{
  std::string r;

  for(int row = 0; row < m.size.height; ++row)
  {
    for(int col = 0; col < m.size.width; ++col)
    {
      char buffer[256];
      snprintf(buffer, sizeof buffer, "%d, ", m.get(col, row));
      r += buffer;
    }
  }

  return r;
}

void dumpQuest(Quest const& q, const char* filename)
{
  FILE* fp = fopen(filename, "wb");

  if(!fp)
    throw Error("Can't open file for writing");

  fprintf(fp, "{\n");
  fprintf(fp, "  \"layers\":\n");
  fprintf(fp, "  [\n");
  fprintf(fp, "    {\n");
  fprintf(fp, "    \"name\":\"rooms\",\n");
  fprintf(fp, "    \"objects\":\n");
  fprintf(fp, "      [\n");

  for(int i = 0; i < (int)q.rooms.size(); ++i)
  {
    auto& r = q.rooms[i];

    fprintf(fp, "         {\n");
    fprintf(fp, "           \"type\":\"%d\",\n", r.theme);
    fprintf(fp, "           \"x\":%d,\n", r.pos.x);
    fprintf(fp, "           \"y\":%d,\n", r.pos.y);
    fprintf(fp, "           \"start_x\":%d,\n", r.start.x);
    fprintf(fp, "           \"start_y\":%d,\n", r.start.y);
    fprintf(fp, "           \"width\":%d,\n", r.size.width);
    fprintf(fp, "           \"height\":%d,\n", r.size.height);
    fprintf(fp, "           \"name\":\"%s\",\n", r.name.c_str());
    fprintf(fp, "           \"tiles\":\"%s\",\n", serializeMatrix(r.tiles).c_str());
    fprintf(fp, "           \"tilesForDisplay\":\"%s\",\n", serializeMatrix(r.tilesForDisplay).c_str());
    fprintf(fp, "           \"entities\":\n");
    fprintf(fp, "           [\n");

    for(int k = 0; k < (int)r.spawners.size(); ++k)
    {
      auto& s = r.spawners[k];

      fprintf(fp, "             {\n");
      fprintf(fp, "               \"type\": \"%s\",\n", s.name.c_str());
      fprintf(fp, "               \"x\":%d,\n", int(s.pos.x * PRECISION));
      fprintf(fp, "               \"y\":%d,\n", int(s.pos.y * PRECISION));

      if(!s.config.empty())
      {
        fprintf(fp, "               \"props\":\n");
        fprintf(fp, "               {\n");

        {
          bool first = true;

          for(auto& prop : s.config)
          {
            if(!first)
              fprintf(fp, ",\n");

            fprintf(fp, "                 \"%s\": \"%s\"", prop.first.c_str(), prop.second.c_str());
            first = false;
          }

          fprintf(fp, "\n");
        }

        fprintf(fp, "               },\n");
      }

      fprintf(fp, "               \"ender\":0\n");
      fprintf(fp, "             }");

      if(k < int(r.spawners.size() - 1))
        fprintf(fp, ",");

      fprintf(fp, "\n");
    }

    fprintf(fp, "           ],\n");
    fprintf(fp, "           \"ender\":0\n");

    fprintf(fp, "         }");

    if(i < int(q.rooms.size() - 1))
      fprintf(fp, ",");

    fprintf(fp, "\n");
  }

  fprintf(fp, "      ]\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "]\n");
  fprintf(fp, "}\n");
  fclose(fp);
}

