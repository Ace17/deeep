#include "base/box.h"
#include "base/error.h"
#include "base/logger.h"
#include "jpg.h"
#include "misc/file.h"
#include "misc/util.h"
#include "picture.h"
#include "png.h"
#include <cstring> // memcpy

namespace
{
Picture loadPng(String path)
{
  Picture pic;
  auto pngDataBuf = File::read(path);
  auto pngData = Span<const uint8_t>((uint8_t*)pngDataBuf.data(), (int)pngDataBuf.size());

  auto decoderFunc = endsWith(path, ".png") ? decodePng : decodeJpg;
  pic.pixels = decoderFunc(pngData, pic.dim.x, pic.dim.y);
  pic.stride = pic.dim.x * 4;

  return pic;
}
}

Picture loadPicture(String path)
{
  try
  {
    auto surface = loadPng(path);

    auto const bpp = 4;

    Rect2i rect;
    rect.pos.x = 0;
    rect.pos.y = 0;
    rect.size.x = surface.dim.x;
    rect.size.y = surface.dim.y;

    std::vector<uint8_t> img(rect.size.x * rect.size.y * bpp);

    auto src = surface.pixels.data() + rect.pos.x * bpp + rect.pos.y * surface.stride;
    auto dst = img.data() + bpp * rect.size.x * rect.size.y;

    // from glTexImage2D doc:
    // "The first element corresponds to the lower left corner of the texture image",
    // (e.g (u,v) = (0,0))
    for(int y = 0; y < rect.size.y; ++y)
    {
      dst -= bpp * rect.size.x;
      memcpy(dst, src, bpp * rect.size.x);
      src += surface.stride;
    }

    Picture r;
    r.dim = rect.size;
    r.stride = rect.size.x;
    r.pixels = std::move(img);

    return r;
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

