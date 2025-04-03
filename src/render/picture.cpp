#include "base/box.h"
#include "base/error.h"
#include "base/logger.h"
#include "jpg.h"
#include "misc/file.h"
#include "misc/util.h"
#include "picture.h"
#include "png.h"

namespace
{
Picture loadPng(String path)
{
  Picture pic;
  auto pngDataBuf = File::read(path);
  auto pngData = Span<const uint8_t>((uint8_t*)pngDataBuf.data(), (int)pngDataBuf.size());

  auto decoderFunc = endsWith(path, ".png") ? decodePng : decodeJpg;
  pic.pixels = decoderFunc(pngData, pic.dim.x, pic.dim.y);
  pic.stride = pic.dim.x;

  return pic;
}
}

Picture loadPicture(String path)
{
  try
  {
    return loadPng(path);
  }
  catch(const Error& e)
  {
    logMsg("[display] can't load texture '%.*s': %.*s. Falling back on generated one", path.len, path.data, e.message().len, e.message().data);

    Picture r;
    r.dim = Vec2i(32, 32);
    r.stride = r.dim.x;
    r.pixels.resize(r.dim.x * r.dim.y * 4);

    for(int y = 0; y < r.dim.y; ++y)
    {
      for(int x = 0; x < r.dim.x; ++x)
      {
        r.pixels[(x + y * r.dim.x) * 4 + 0] = 0xff;
        r.pixels[(x + y * r.dim.x) * 4 + 1] = x < 16 ? 0xff : 0x00;
        r.pixels[(x + y * r.dim.x) * 4 + 2] = y < 16 ? 0xff : 0x00;
        r.pixels[(x + y * r.dim.x) * 4 + 3] = 0xff;
      }
    }

    return r;
  }
}

