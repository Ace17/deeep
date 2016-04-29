#include "decompress.h"

#define MINIZ_HEADER_FILE_ONLY 1
#include "extra/miniz.c"

#include <cstring>
#include <vector>

using namespace std;
vector<uint8_t> decompress(Slice<const uint8_t> buffer)
{
  vector<uint8_t> r;

  z_stream s;
  memset(&s, 0, sizeof s);
  s.next_in = buffer.data;
  s.avail_in = buffer.len;
  inflateInit(&s);

  for(;;)
  {
    uint8_t out[32];
    s.avail_out = sizeof out;
    s.next_out = out;
    inflate(&s, MZ_SYNC_FLUSH);
    const auto bytes = int((sizeof out) - s.avail_out);

    if(!bytes)
      break;

    for(int i = 0; i < bytes; ++i)
      r.push_back(out[i]);
  }

  inflateEnd(&s);
  return r;
}

