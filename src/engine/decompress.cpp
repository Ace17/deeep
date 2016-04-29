#include "decompress.h"

#define MINIZ_HEADER_FILE_ONLY 1
#include "extra/miniz.c"

#include <cstring>
#include <vector>

using namespace std;
vector<uint8_t> decompress(Slice<const uint8_t> buffer)
{
  vector<uint8_t> r;

  int ret;
  z_stream s;
  memset(&s, 0, sizeof s);
  s.next_in = buffer.data;
  s.avail_in = buffer.len;
  ret = inflateInit(&s);

  if(ret < 0)
    throw runtime_error(string("inflateInit failed: ") + mz_error(ret));

  for(;;)
  {
    uint8_t out[32];
    s.avail_out = sizeof out;
    s.next_out = out;
    ret = inflate(&s, MZ_SYNC_FLUSH);

    if(ret < 0)
      throw runtime_error(string("inflate failed: ") + mz_error(ret));

    const auto bytes = int((sizeof out) - s.avail_out);

    if(!bytes)
      break;

    for(int i = 0; i < bytes; ++i)
      r.push_back(out[i]);
  }

  if(s.avail_in != 0)
    throw runtime_error("incompletely read input");

  ret = inflateEnd(&s);

  if(ret < 0)
    throw runtime_error("inflateEnd failed");

  return r;
}

