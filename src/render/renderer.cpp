// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <array>
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
const auto INTERNAL_RESOLUTION = Vec2i(256, 256);
const auto TILE_SIZE = 16.0f;

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
  Renderer(IGraphicsBackend* backend_)
    : backend(backend_)
  {
    m_quadShader = backend->createGpuProgram("standard", false);
    m_batchVbo = backend->createVertexBuffer();
    m_fb = backend->createFrameBuffer(INTERNAL_RESOLUTION, false);

    m_quadVbo = backend->createVertexBuffer();
    m_quadVbo->upload(quadVertices, sizeof quadVertices);

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

    vboData.clear();

    ITexture* currTexture = nullptr;
    std::array<float, 3> currLight {};
    currLight[0] = 0.0f / 0.0f;
    IGpuProgram* currShader = nullptr;

    backend->useVertexBuffer(m_batchVbo.get());

    for(auto const& q : m_quads)
    {
      if(vboData.size() * 6 >= MAX_QUADS)
        flushBatch();

      if(currShader != m_quadShader.get())
      {
        flushBatch();

        backend->useGpuProgram(m_quadShader.get());
        backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
        backend->enableVertexAttribute(1 /* uvLoc       */, 2, sizeof(Vertex), offsetof(Vertex, u));

        currShader = m_quadShader.get();
      }

      if(m_tiles[q.tile].texture != currTexture)
      {
        flushBatch();

        currTexture = m_tiles[q.tile].texture;
        // Bind our diffuse texture in Texture Unit 0
        currTexture->bind(0);
      }

      if(q.light != currLight)
      {
        flushBatch();

        MyUniformBlock block { q.light[0], q.light[1], q.light[2], 0 };
        backend->setUniformBlock(&block, sizeof block);
        currLight = q.light;
      }

      const auto& tile = m_tiles[q.tile];
      const float u0 = tile.uv[0].x;
      const float v0 = 1 - tile.uv[1].y;
      const float u1 = tile.uv[1].x;
      const float v1 = 1 - tile.uv[0].y;

      vboData.push_back({ q.pos[0].x, q.pos[0].y, u0, v0 });
      vboData.push_back({ q.pos[1].x, q.pos[1].y, u0, v1 });
      vboData.push_back({ q.pos[2].x, q.pos[2].y, u1, v1 });

      vboData.push_back({ q.pos[0].x, q.pos[0].y, u0, v0 });
      vboData.push_back({ q.pos[2].x, q.pos[2].y, u1, v1 });
      vboData.push_back({ q.pos[3].x, q.pos[3].y, u1, v0 });
    }

    flushBatch();

    ggSpriteCount = m_quads.size();
    ggBatchCount = batchCount;
    ggVboCap = vboData.capacity();

    m_quads.clear();
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
  static Matrix3f getCameraMatrix(const Camera& cam)
  {
    static const auto half_w = INTERNAL_RESOLUTION.x / 2;
    static const auto half_h = INTERNAL_RESOLUTION.y / 2;
    static const auto shrink = scale(Vec2f(TILE_SIZE / half_w, TILE_SIZE / half_h));
    return shrink * rotate(-cam.angle) * translate(-1 * cam.pos);
  }

  Camera m_camera;
  bool m_cameraValid = false;

  std::unique_ptr<IGpuProgram> m_quadShader;

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
    auto& model = m_Models.at(sprite.modelId);

    if(model.actions.empty())
      throw Error("model has no actions");

    if(sprite.actionIdx < 0 || sprite.actionIdx >= (int)model.actions.size())
      throw Error("invalid action index");

    auto const& action = model.actions[sprite.actionIdx];

    if(action.textures.empty())
      throw Error("action has no textures");

    auto const N = (int)action.textures.size();
    auto const idx = ::clamp<int>(sprite.frame * N, 0, N - 1);

    Vec2f pos = sprite.pos;

    if(sprite.halfSize.x < 0)
      pos.x -= sprite.halfSize.x;

    if(sprite.halfSize.y < 0)
      pos.y -= sprite.halfSize.y;

    const auto worldTransform = translate(pos) * rotate(sprite.angle) * scale(sprite.halfSize);
    const auto transform = getCameraMatrix(cam) * worldTransform;

    Quad q;
    q.zOrder = sprite.zOrder;
    q.tile = action.textures[idx];

    auto const m0x = 0;
    auto const m0y = 0;
    auto const m1x = 1;
    auto const m1y = 1;

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

  std::vector<Quad> m_quads;
  std::unique_ptr<IVertexBuffer> m_batchVbo;
  std::unique_ptr<IVertexBuffer> m_quadVbo;
  std::unique_ptr<IFrameBuffer> m_fb;

  std::unordered_map<int, Model> m_Models;
  std::unordered_map<std::string, std::unique_ptr<ITexture>> m_textures;
  std::vector<Tile> m_tiles;

  std::vector<Vertex> vboData; // VBO scratch buffer

  float m_ambientLight = 0;
  int m_frameCount = 0;

  IGraphicsBackend* const backend;

public:
  int loadTexture(String path, Rect2f frect)
  {
    const std::string sPath(path.data, path.len);

    if(m_textures.find(sPath) == m_textures.end())
    {
      auto pic = loadPicture(path, Rect2f({ 0, 0 }, { 1, 1 }));
      m_textures[sPath] = backend->createTexture();
      m_textures[sPath]->upload(pic);

      if(0)
        logMsg("[renderer] loaded '%.*s'", path.len, path.data);
    }

    const float left = frect.pos.x;
    const float right = frect.pos.x + frect.size.x;
    const float top = frect.pos.y;
    const float bottom = frect.pos.y + frect.size.y;

    const int id = (int)m_tiles.size();
    m_tiles.push_back({ m_textures[sPath].get(), { { left, top }, { right, bottom } } });
    return id;
  }
};
}

IRenderer* createRenderer(IGraphicsBackend* gfxBackend)
{
  return new Renderer(gfxBackend);
}

