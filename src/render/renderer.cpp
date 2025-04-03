// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <array>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "base/box.h"
#include "base/error.h"
#include "base/geom.h"
#include "base/logger.h"
#include "base/my_algorithm.h" // sort
#include "base/renderer.h"
#include "base/scene.h"
#include "base/span.h"
#include "base/util.h" // clamp
#include "engine/graphics_backend.h"
#include "matrix3.h"
#include "misc/file.h"
#include "misc/math.h"
#include "misc/stats.h"
#include "misc/util.h"
#include "model.h"
#include "picture.h"

namespace
{
Gauge ggVboCap("VBO cap");
Gauge ggSpriteCount("sprites");
Gauge ggBatchCount("batches");

const int MAX_QUADS = 32678;
const auto TILE_SIZE = 16.0f;
const float SCALE = 0.1;
const Vec2i AtlasSize = { 2048, 2048 };

Vec2f multiplyMatrix(const Matrix3f& mat, float v0, float v1, float v2)
{
  Vec2f r;
  r.x = mat[0][0] * v0 + mat[0][1] * v1 + mat[0][2] * v2;
  r.y = mat[1][0] * v0 + mat[1][1] * v1 + mat[1][2] * v2;
  return r;
}

struct Camera
{
  Vec2f pos = Vec2f(0, 0);
  float angle = 0;
};

// VBO format
struct Vertex
{
  float x, y, u, v;
  float r = 1, g = 1, b = 1, a = 1;
};

const Vertex quadVertices[] =
{
  { -1, -1, 0, 0 },
  { +1, -1, 1, 0 },
  { +1, +1, 1, 1 },

  { -1, -1, 0, 0 },
  { +1, +1, 1, 1 },
  { -1, +1, 0, 1 },
};

struct Renderer : IRenderer
{
  Renderer(IGraphicsBackend* backend_, Vec2i internalResolution)
    : backend(backend_)
    , m_internalResolution(internalResolution)
  {
    m_quadShader = backend->createGpuProgram("standard", false);
    m_lineShader = backend->createGpuProgram("line", false);
    m_batchVbo = backend->createVertexBuffer();
    m_fb = backend->createFrameBuffer(m_internalResolution, false);

    m_quadVbo = backend->createVertexBuffer();
    m_quadVbo->upload(quadVertices, sizeof quadVertices);

    m_atlasTexture = backend->createTexture();
    m_atlasTexture->create(AtlasSize);

    loadModel(-1, "res/font.model");
  }

  void loadModel(int id, String path) override
  {
    auto loadTexFunc = [&] (String path, Rect2f frect) { return loadTexture(path, frect); };
    m_Models[id] = ::loadModel(path, loadTexFunc);
  }

  void setCamera(Vec2f pos) override
  {
    auto cam = Camera{ pos, 0 };

    if(!m_cameraValid)
    {
      m_camera = cam;
      m_cameraValid = true;
    }

    // avoid big camera jumps
    {
      auto delta = m_camera.pos - pos;

      if(abs(delta.x) > 2 || abs(delta.y) > 2)
        m_camera = cam;
    }

    m_camera.pos = lerp(m_camera.pos, cam.pos, 0.3f);
    m_camera.angle = cam.angle;
  }

  void setAmbientLight(float ambientLight) override
  {
    m_ambientLight = ambientLight;
  }

  void beginDraw() override
  {
    m_frameCount++;
  }

  struct MyUniformBlock
  {
    float fragOffset[4];
  };

  void endDraw() override
  {
    // draw to internal framebuffer, with fixed resolution
    backend->setRenderTarget(m_fb.get());
    backend->clear();

    processCommands();

    // draw to screen
    backend->setRenderTarget(nullptr);
    backend->clear();

    backend->useGpuProgram(m_quadShader.get());
    backend->useVertexBuffer(m_quadVbo.get());
    m_fb->getColorTexture()->bind(0);
    backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
    backend->enableVertexAttribute(1 /* uvLoc       */, 2, sizeof(Vertex), offsetof(Vertex, u));
    MyUniformBlock block {};
    backend->setUniformBlock(&block, sizeof block);
    backend->draw(6);

    backend->swap();
  }

  void processCommands()
  {
    batchCount = 0;

    auto byPriority = [&] (Quad const& a, Quad const& b)
      {
        if(a.zOrder != b.zOrder)
          return a.zOrder < b.zOrder;

        if(m_tiles[a.tile].texture != m_tiles[b.tile].texture)
          return m_tiles[a.tile].texture < m_tiles[b.tile].texture;

        if(a.light != b.light)
          return a.light < b.light;

        return true;
      };

    my::sort<Quad>(m_quads, byPriority);

    auto linesByZ = [] (const RenderLine& a, const RenderLine& b)
      {
        return a.zOrder < b.zOrder;
      };

    my::sort<RenderLine>(m_lines, linesByZ);

    vboData.clear();

    ITexture* currTexture = nullptr;
    std::array<float, 3> currLight {};
    currLight[0] = 0.0f / 0.0f;
    IGpuProgram* currShader = nullptr;

    backend->useVertexBuffer(m_batchVbo.get());

    Span<RenderLine> lines(m_lines);
    Span<RenderCircle> circles(m_circles);
    Span<Quad> quads(m_quads);

    const auto cameraTransform = getCameraMatrix(m_camera);
    const auto identity = translate({});

    auto addOneLine = [&] (const RenderLine& line)
      {
        if(currShader != m_lineShader.get())
        {
          flushBatch();

          backend->useGpuProgram(m_lineShader.get());
          backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
          backend->enableVertexAttribute(1 /* uv          */, 2, sizeof(Vertex), offsetof(Vertex, u));
          backend->enableVertexAttribute(2 /* color       */, 4, sizeof(Vertex), offsetof(Vertex, r));

          currShader = m_lineShader.get();
        }

        auto const transform = identity;

        auto const a = multiplyMatrix(transform, line.a.x, line.a.y, 1);
        auto const b = multiplyMatrix(transform, line.b.x, line.b.y, 1);

        auto const n = normalize(b - a);
        auto const t = Vec2f(-n.y, n.x);

        auto const p0 = a + t * line.thicknessMax * SCALE;
        auto const p1 = a - t * line.thicknessMax * SCALE;
        auto const p2 = b - t * line.thicknessMax * SCALE;
        auto const p3 = b + t * line.thicknessMax * SCALE;

        const auto u = line.thicknessMin;

        vboData.push_back(Vertex{ p0.x, p0.y, -u, 0, line.color.r, line.color.g, line.color.b, line.color.a });
        vboData.push_back(Vertex{ p1.x, p1.y, +u, 0, line.color.r, line.color.g, line.color.b, line.color.a });
        vboData.push_back(Vertex{ p2.x, p2.y, +u, 0, line.color.r, line.color.g, line.color.b, line.color.a });

        vboData.push_back(Vertex{ p0.x, p0.y, -u, 0, line.color.r, line.color.g, line.color.b, line.color.a });
        vboData.push_back(Vertex{ p2.x, p2.y, +u, 0, line.color.r, line.color.g, line.color.b, line.color.a });
        vboData.push_back(Vertex{ p3.x, p3.y, -u, 0, line.color.r, line.color.g, line.color.b, line.color.a });
      };

    auto addOneCircle = [&] (const RenderCircle& circle)
      {
        if(currShader != m_lineShader.get())
        {
          flushBatch();

          backend->useGpuProgram(m_lineShader.get());
          backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
          backend->enableVertexAttribute(1 /* uv          */, 2, sizeof(Vertex), offsetof(Vertex, u));
          backend->enableVertexAttribute(2 /* color       */, 4, sizeof(Vertex), offsetof(Vertex, r));

          currShader = m_lineShader.get();
        }

        auto transform = circle.useWorldRefFrame ? cameraTransform : identity;
        const int N = 48;

        for(int i = 0; i < N; ++i)
        {
          float angle0 = (i + 0) * 2 * PI / N;
          float angle1 = (i + 1) * 2 * PI / N;

          const float cos0 = cos(angle0);
          const float sin0 = sin(angle0);
          const float cos1 = cos(angle1);
          const float sin1 = sin(angle1);

          const float thickness = 1.0 * SCALE;

          const float innerRadius = circle.radius - thickness * 0.5;
          const float outerRadius = circle.radius + thickness * 0.5;

          Vec2f p0 = circle.pos + Vec2f(cos0, sin0) * innerRadius;
          Vec2f p1 = circle.pos + Vec2f(cos0, sin0) * outerRadius;
          Vec2f p2 = circle.pos + Vec2f(cos1, sin1) * outerRadius;
          Vec2f p3 = circle.pos + Vec2f(cos1, sin1) * innerRadius;

          p0 = multiplyMatrix(transform, p0.x, p0.y, 1);
          p1 = multiplyMatrix(transform, p1.x, p1.y, 1);
          p2 = multiplyMatrix(transform, p2.x, p2.y, 1);
          p3 = multiplyMatrix(transform, p3.x, p3.y, 1);

          const auto u = 1;

          vboData.push_back(Vertex{ p0.x, p0.y, -u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });
          vboData.push_back(Vertex{ p1.x, p1.y, +u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });
          vboData.push_back(Vertex{ p2.x, p2.y, +u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });

          vboData.push_back(Vertex{ p0.x, p0.y, -u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });
          vboData.push_back(Vertex{ p2.x, p2.y, +u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });
          vboData.push_back(Vertex{ p3.x, p3.y, -u, 0, circle.color.r, circle.color.g, circle.color.b, circle.color.a });
        }
      };

    auto addOneQuad = [&] (const Quad& quad)
      {
        if(currShader != m_quadShader.get())
        {
          flushBatch();

          backend->useGpuProgram(m_quadShader.get());
          backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
          backend->enableVertexAttribute(1 /* uvLoc       */, 2, sizeof(Vertex), offsetof(Vertex, u));

          currShader = m_quadShader.get();
        }

        if(m_tiles[quad.tile].texture != currTexture)
        {
          flushBatch();

          currTexture = m_tiles[quad.tile].texture;
          currTexture->bind(0); // Bind our diffuse texture in Texture Unit 0
        }

        if(quad.light != currLight)
        {
          flushBatch();

          MyUniformBlock block { quad.light[0], quad.light[1], quad.light[2], 0 };
          backend->setUniformBlock(&block, sizeof block);
          currLight = quad.light;
        }

        if(vboData.size() * 6 >= MAX_QUADS)
          flushBatch();

        const auto& tile = m_tiles[quad.tile];
        const float u0 = tile.uv[0].x;
        const float v0 = 1 - tile.uv[1].y;
        const float u1 = tile.uv[1].x;
        const float v1 = 1 - tile.uv[0].y;

        vboData.push_back({ quad.pos[0].x, quad.pos[0].y, u0, v0 });
        vboData.push_back({ quad.pos[1].x, quad.pos[1].y, u0, v1 });
        vboData.push_back({ quad.pos[2].x, quad.pos[2].y, u1, v1 });

        vboData.push_back({ quad.pos[0].x, quad.pos[0].y, u0, v0 });
        vboData.push_back({ quad.pos[2].x, quad.pos[2].y, u1, v1 });
        vboData.push_back({ quad.pos[3].x, quad.pos[3].y, u1, v0 });
      };

    while(lines.len || circles.len || quads.len)
    {
      float minZ = 1.0 / 0.0;

      if(lines.len && lines[0].zOrder < minZ)
        minZ = lines[0].zOrder;

      if(circles.len && circles[0].zOrder < minZ)
        minZ = circles[0].zOrder;

      if(quads.len && quads[0].zOrder < minZ)
        minZ = quads[0].zOrder;

      while(lines.len > 0 && lines[0].zOrder == minZ)
      {
        addOneLine(lines[0]);
        lines += 1;
      }

      while(circles.len > 0 && circles[0].zOrder == minZ)
      {
        addOneCircle(circles[0]);
        circles += 1;
      }

      while(quads.len > 0 && quads[0].zOrder == minZ)
      {
        addOneQuad(quads[0]);
        quads += 1;
      }
    }

    flushBatch();

    ggSpriteCount = m_quads.size();
    ggBatchCount = batchCount;
    ggVboCap = vboData.capacity();

    m_quads.clear();
    m_circles.clear();
    m_lines.clear();
  }

  int batchCount = 0;

  void flushBatch()
  {
    if(vboData.empty())
      return;

    m_batchVbo->upload(vboData.data(), vboData.size() * sizeof(vboData[0]));
    backend->useVertexBuffer(m_batchVbo.get());
    backend->draw(vboData.size());

    vboData.clear();

    ++batchCount;
  }

  void drawText(const RenderText& text) override
  {
    Vec2f size = { 0.5, 0.5 };

    auto pos = text.pos;

    pos.x = pos.x - text.text.len * size.x * 0.5;
    pos.y = pos.y;

    for(auto& c : text.text)
    {
      RenderSprite s {};
      s.pos = pos;
      s.halfSize = size;
      s.modelId = -1;
      s.zOrder = 100;
      s.actionIdx = c;
      s.frame = 0;

      drawSprite(s);
      pos.x += size.x;
    }
  }

  void drawCircle(const RenderCircle& circle) override
  {
    auto cam = circle.useWorldRefFrame ? m_camera : Camera();
    const auto transform = getCameraMatrix(cam);

    RenderCircle c = circle;
    Vec2f p = circle.pos + Vec2f(circle.radius, 0);
    c.pos = multiplyMatrix(transform, circle.pos.x, circle.pos.y, 1);
    c.radius = (c.pos - multiplyMatrix(transform, p.x, p.y, 1)).x;

    m_circles.push_back(c);
  }

  void drawLine(const RenderLine& line) override
  {
    auto cam = line.useWorldRefFrame ? m_camera : Camera();
    const auto transform = getCameraMatrix(cam);

    RenderLine l = line;
    l.a = multiplyMatrix(transform, line.a.x, line.a.y, 1);
    l.b = multiplyMatrix(transform, line.b.x, line.b.y, 1);

    m_lines.push_back(l);
  }

  void drawSprite(const RenderSprite& sprite) override
  {
    auto q = spriteToQuad(sprite);

    // early culling
    {
      BoundingBox box(q.pos[0]);

      for(auto& p : q.pos)
        box.add(p);

      if(box.max.x < -1.0 || box.min.x > 1.0 || box.max.y < -1.0 || box.min.y > 1.0)
        return;
    }

    m_quads.push_back(q);
  }

private:
  Matrix3f getCameraMatrix(const Camera& cam) const
  {
    static const auto half_w = m_internalResolution.x / 2;
    static const auto half_h = m_internalResolution.y / 2;
    static const auto shrink = scale(Vec2f(TILE_SIZE / half_w, TILE_SIZE / half_h));
    return shrink * rotate(-cam.angle) * translate(-1 * cam.pos);
  }

  Camera m_camera;
  bool m_cameraValid = false;

  std::unique_ptr<IGpuProgram> m_quadShader;
  std::unique_ptr<IGpuProgram> m_lineShader;

  struct Quad
  {
    int zOrder;
    std::array<float, 3> light {};
    int tile;
    Vec2f pos[4];
  };

  Quad spriteToQuad(const RenderSprite& sprite) const
  {
    auto cam = sprite.useWorldRefFrame ? m_camera : Camera();
    auto it = m_Models.find(sprite.modelId);

    if(it == m_Models.end())
    {
      char buffer[256];
      throw Error(format(buffer, "No such model: %d", sprite.modelId));
    }

    auto& model = it->second;

    if(model.actions.empty())
    {
      char buffer[256];
      throw Error(format(buffer, "model %d has no actions", sprite.modelId));
    }

    if(sprite.actionIdx < 0 || sprite.actionIdx >= (int)model.actions.size())
      throw Error("invalid action index");

    auto const& action = model.actions[sprite.actionIdx];

    if(action.textures.empty())
      throw Error("action has no textures");

    auto const N = (int)action.textures.size();
    auto const idx = ::clamp<int>(sprite.frame * N, 0, N - 1);

    Vec2f pos = sprite.pos;

    const auto worldTransform = translate(pos) * rotate(sprite.angle) * scale(sprite.halfSize);
    const auto transform = getCameraMatrix(cam) * worldTransform;

    Quad q;
    q.zOrder = sprite.zOrder;
    q.tile = action.textures[idx];

    auto const m0x = -0.5;
    auto const m0y = -0.5;
    auto const m1x = +0.5;
    auto const m1y = +0.5;

    q.pos[0] = multiplyMatrix(transform, m0x, m0y, 1);
    q.pos[1] = multiplyMatrix(transform, m0x, m1y, 1);
    q.pos[2] = multiplyMatrix(transform, m1x, m1y, 1);
    q.pos[3] = multiplyMatrix(transform, m1x, m0y, 1);

    // lighting
    {
      q.light[0] = m_ambientLight;
      q.light[1] = m_ambientLight;
      q.light[2] = m_ambientLight;

      if(sprite.blinking)
      {
        if((m_frameCount / 4) % 2)
        {
          q.light[0] = 0.8;
          q.light[1] = 0.4;
          q.light[2] = 0.4;
        }
      }
    }

    return q;
  }

  struct Tile
  {
    ITexture* texture;
    Vec2f uv[2];
  };

  struct AtlasedTexture
  {
    Vec2i pos;
    Vec2i dim;
  };

  std::vector<RenderCircle> m_circles;
  std::vector<RenderLine> m_lines;
  std::vector<Quad> m_quads;
  std::unique_ptr<IVertexBuffer> m_batchVbo;
  std::unique_ptr<IVertexBuffer> m_quadVbo;
  std::unique_ptr<IFrameBuffer> m_fb;

  std::unordered_map<int, Model> m_Models;
  std::unordered_map<std::string, AtlasedTexture> m_atlasedTextures;
  std::vector<Tile> m_tiles;

  std::vector<Vertex> vboData; // VBO scratch buffer
  std::unique_ptr<ITexture> m_atlasTexture;
  Vec2i m_atlasFreeSpacePointer {};
  int m_atlasMaxHeightOnCurrentLine = 0;
  float m_ambientLight = 0;
  int m_frameCount = 0;

  IGraphicsBackend* const backend;
  const Vec2i m_internalResolution;

  Vec2i findFreePosInAtlas(Vec2i dim)
  {
    if(m_atlasFreeSpacePointer.x + dim.x > AtlasSize.x)
    {
      // the texture doesn't fit on this line, create a new line
      m_atlasFreeSpacePointer.x = 0;
      m_atlasFreeSpacePointer.y += m_atlasMaxHeightOnCurrentLine;
    }

    if(m_atlasFreeSpacePointer.y + dim.y >= AtlasSize.y)
    {
      char buffer[256];
      throw Error(format(buffer, "Atlas is full"));
    }

    const Vec2i pos = m_atlasFreeSpacePointer;
    m_atlasFreeSpacePointer.x += dim.x;
    m_atlasMaxHeightOnCurrentLine = std::max(m_atlasMaxHeightOnCurrentLine, dim.y);
    return pos;
  }

  int loadTexture(String path, Rect2f frect)
  {
    const std::string sPath(path.data, path.len);

    if(m_atlasedTextures.find(sPath) == m_atlasedTextures.end())
    {
      auto pic = loadPicture(path);
      const Vec2i posInAtlas = findFreePosInAtlas(pic.dim);
      m_atlasTexture->upload(pic, posInAtlas);

      m_atlasedTextures[sPath] = { posInAtlas, pic.dim };

      if(0)
        logMsg("[renderer] loaded '%.*s' at (%d;%d)", path.len, path.data, posInAtlas.x, posInAtlas.y);
    }

    const AtlasedTexture& tex = m_atlasedTextures[sPath];

    const float texLocalLeft = frect.pos.x;
    const float texLocalRight = frect.pos.x + frect.size.x;
    const float texLocalTop = 1 - frect.pos.y;
    const float texLocalBottom = 1 - (frect.pos.y + frect.size.y);

    const int leftInPixels = tex.pos.x + (texLocalLeft * tex.dim.x);
    const int rightInPixels = tex.pos.x + (texLocalRight * tex.dim.x);
    const int topInPixels = tex.pos.y + (texLocalTop * tex.dim.y);
    const int bottomInPixels = tex.pos.y + (texLocalBottom * tex.dim.y);

    const float left = leftInPixels / float(AtlasSize.x);
    const float right = rightInPixels / float(AtlasSize.x);
    const float top = 1 - topInPixels / float(AtlasSize.y);
    const float bottom = 1 - bottomInPixels / float(AtlasSize.y);

    const int id = (int)m_tiles.size();
    m_tiles.push_back({ m_atlasTexture.get(), { { left, top }, { right, bottom } } });
    return id;
  }
};
}

IRenderer* createRenderer(IGraphicsBackend* gfxBackend, Vec2i internalResolution)
{
  return new Renderer(gfxBackend, internalResolution);
}

