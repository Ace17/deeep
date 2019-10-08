#include "load_quest.h"
#include "preprocess_quest.h"

#include <stdio.h>

void dumpQuest(Quest const& q, const char* filename);

int main(int argc, const char* argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s <quest.json> <packedquest.json>\n", argv[0]);
    return 1;
  }

  auto q = loadQuest(argv[1]);
  preprocessQuest(q);
  dumpQuest(q, argv[2]);
  return 0;
}

void dumpQuest(Quest const& q, const char* filename)
{
  FILE* fp = fopen(filename, "wb");

  if(!fp)
    throw std::runtime_error("Can't open file for writing");

  (void)q;
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
    fprintf(fp, "           \"x\":%d,\n", r.pos.x * 4);
    fprintf(fp, "           \"y\":%d,\n", r.pos.y * 4);
    fprintf(fp, "           \"width\":%d,\n", r.size.width * 4);
    fprintf(fp, "           \"height\":%d,\n", r.size.height * 4);
    fprintf(fp, "           \"name\":\"%s\",\n", r.name.c_str());
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

