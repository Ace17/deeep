#include "base/error.h"
#include "misc/file.h"
#include "picture.h"
#include "png.h"
#include <cstring> // memcpy
#include <map>

namespace
{
map<string, Picture> g_pictureCache;

Picture loadPng(string path)
{
  Picture pic;
  auto pngDataBuf = File::read(path);
  auto pngData = Span<const uint8_t>((uint8_t*)pngDataBuf.data(), (int)pngDataBuf.size());
  pic.pixels = decodePng(pngData, pic.dim.width, pic.dim.height);
  pic.stride = pic.dim.width * 4;

  return pic;
}

Picture* getPicture(string path)
{
  if(g_pictureCache.find(path) == g_pictureCache.end())
    g_pictureCache[path] = loadPng(path);

  return &g_pictureCache.at(path);
}
}

Picture loadPicture(String path, Rect2f frect)
{
  try
  {
    auto surface = getPicture(string(path.data, path.len));

    if(frect.size.width == 0 && frect.size.height == 0)
      frect = Rect2f(0, 0, 1, 1);

    if(frect.pos.x < 0 || frect.pos.y < 0 || frect.pos.x + frect.size.width > 1 || frect.pos.y + frect.size.height > 1)
      throw Error("Invalid boundaries for '" + string(path.data, path.len) + "'");

    auto const bpp = 4;

    Rect2i rect;
    rect.pos.x = frect.pos.x * surface->dim.width;
    rect.pos.y = frect.pos.y * surface->dim.height;
    rect.size.width = frect.size.width * surface->dim.width;
    rect.size.height = frect.size.height * surface->dim.height;

    vector<uint8_t> img(rect.size.width * rect.size.height * bpp);

    auto src = surface->pixels.data() + rect.pos.x * bpp + rect.pos.y * surface->stride;
    auto dst = img.data() + bpp * rect.size.width * rect.size.height;

    // from glTexImage2D doc:
    // "The first element corresponds to the lower left corner of the texture image",
    // (e.g (u,v) = (0,0))
    for(int y = 0; y < rect.size.height; ++y)
    {
      dst -= bpp * rect.size.width;
      memcpy(dst, src, bpp * rect.size.width);
      src += surface->stride;
    }

    Picture r;
    r.dim = rect.size;
    r.stride = rect.size.width;
    r.pixels = std::move(img);

    return r;
  }
  catch(const Error& e)
  {
    printf("[display] can't load texture: %.*s\n", e.message().len, e.message().data);
    printf("[display] falling back on generated texture\n");

    Picture r;
    r.dim = Size2i(32, 32);
    r.stride = r.dim.width;
    r.pixels.resize(r.dim.width * r.dim.height * 4);

    for(int y = 0; y < r.dim.height; ++y)
    {
      for(int x = 0; x < r.dim.width; ++x)
      {
        r.pixels[(x + y * r.dim.width) * 4 + 0] = 0xff;
        r.pixels[(x + y * r.dim.width) * 4 + 1] = x < 16 ? 0xff : 0x00;
        r.pixels[(x + y * r.dim.width) * 4 + 2] = y < 16 ? 0xff : 0x00;
        r.pixels[(x + y * r.dim.width) * 4 + 3] = 0xff;
      }
    }

    return r;
  }
}

