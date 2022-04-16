// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <array>
#include <unordered_map>
#include <vector>
using namespace std;

#include "base/error.h"
#include "base/geom.h"
#include "base/my_algorithm.h" // sort
#include "base/renderer.h"
#include "base/scene.h"
#include "base/span.h"
#include "base/util.h" // clamp
#include "engine/graphics_backend.h"
#include "engine/stats.h"
#include "matrix3.h"
#include "misc/file.h"
#include "misc/util.h"
#include "model.h"
#include "picture.h"

namespace
{
const int MAX_QUADS = 32678;
const float SCALE = 0.125f;

Vector2f multiplyMatrix(const Matrix3f& mat, float v0, float v1, float v2)
{
  Vector2f r;
  r.x = mat[0][0] * v0 + mat[0][1] * v1 + mat[0][2] * v2;
  r.y = mat[1][0] * v0 + mat[1][1] * v1 + mat[1][2] * v2;
  return r;
}

struct Renderer;
Renderer* g_Renderer;

struct Camera
{
  Vector2f pos = Vector2f(0, 0);
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

template<typename T>
T blend(T a, T b, float alpha)
{
  return a * (1 - alpha) + b * alpha;
}

struct Renderer : IRenderer
{
  Renderer(IGraphicsBackend* backend_)
    : backend(backend_)
  {
    g_Renderer = this;
    m_shader = backend->createGpuProgram("standard", false);
    m_batchVbo = backend->createVertexBuffer();
    m_fb = backend->createFrameBuffer(Size2i(256, 256), false);

    m_quadVbo = backend->createVertexBuffer();
    m_quadVbo->upload(quadVertices, sizeof quadVertices);

    m_fontModel = ::loadModel("res/font.model");
  }

  void loadModel(int id, String path) override
  {
    m_Models[id] = ::loadModel(path);
  }

  void setCamera(Vector2f pos) override
  {
    auto cam = Camera { pos, 0 };

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

    m_camera.pos = blend(m_camera.pos, cam.pos, 0.3f);
    m_camera.angle = cam.angle;
  }

  void setAmbientLight(float ambientLight) override
  {
    m_ambientLight = ambientLight;
  }

  void beginDraw() override
  {
    m_frameCount++;
    m_quads.clear();
  }

  void endDraw() override
  {
    // draw to internal framebuffer, with fixed resolution
    backend->setRenderTarget(m_fb.get());
    backend->clear();

    processQuads();

    // draw to screen
    backend->setRenderTarget(nullptr);
    backend->clear();
    backend->useVertexBuffer(m_quadVbo.get());
    m_fb->getColorTexture()->bind(0);
    backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
    backend->enableVertexAttribute(1 /* uvLoc       */, 2, sizeof(Vertex), offsetof(Vertex, u));
    backend->draw(6);

    backend->swap();
  }

  void processQuads()
  {
    backend->useGpuProgram(m_shader.get());

    auto byPriority = [&] (Quad const& a, Quad const& b)
      {
        if(a.zOrder != b.zOrder)
          return a.zOrder < b.zOrder;

        if(a.light != b.light)
          return a.light < b.light;

        return m_tiles[a.tile].texture < m_tiles[b.tile].texture;
      };

    my::sort<Quad>(m_quads, byPriority);

    vector<Vertex> vboData;

    auto flush = [&] ()
      {
        if(vboData.empty())
          return;

        m_batchVbo->upload(vboData.data(), vboData.size() * sizeof(vboData[0]));

        backend->useVertexBuffer(m_batchVbo.get());
        backend->enableVertexAttribute(0 /* positionLoc */, 2, sizeof(Vertex), offsetof(Vertex, x));
        backend->enableVertexAttribute(1 /* uvLoc       */, 2, sizeof(Vertex), offsetof(Vertex, u));
        backend->draw(vboData.size());

        vboData.clear();
      };

    ITexture* currTexture = nullptr;
    std::array<float, 3> currLight {};
    currLight[0] = 0.0f / 0.0f;

    for(auto const& q : m_quads)
    {
      if(vboData.size() * 6 >= MAX_QUADS)
        flush();

      if(m_tiles[q.tile].texture != currTexture)
      {
        flush();

        currTexture = m_tiles[q.tile].texture;
        // Bind our diffuse texture in Texture Unit 0
        currTexture->bind(0);
      }

      if(q.light != currLight)
      {
        flush();

        backend->setUniformFloat4(0 /* colorLoc */, q.light[0], q.light[1], q.light[2], 0);
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

    flush();
  }

  void drawSprite(const RenderSprite& sprite) override
  {
    auto& model = m_Models.at(sprite.modelId);
    auto cam = sprite.useWorldRefFrame ? m_camera : Camera();
    pushQuad(sprite.pos, sprite.halfSize, sprite.angle, cam, model, sprite.blinking, sprite.actionIdx, sprite.frame, sprite.zOrder);
  }

  void drawText(const RenderText& text) override
  {
    Vector2f size = { 0.5, 0.5 };

    auto pos = text.pos;

    pos.x = pos.x - text.text.len * size.x * 0.5;
    pos.y = pos.y;

    for(auto& c : text.text)
    {
      pushQuad(pos, size, 0, {}, m_fontModel, false, c, 0, 100);
      pos.x += size.x;
    }
  }

private:
  void pushQuad(Vector2f pos, Vector2f halfSize, float angle, Camera cam, Model const& model, bool blinking, int actionIdx, float ratio, int zOrder)
  {
    if(model.actions.empty())
      throw Error("model has no actions");

    if(actionIdx < 0 || actionIdx >= (int)model.actions.size())
      throw Error("invalid action index");

    auto const& action = model.actions[actionIdx];

    if(action.textures.empty())
      throw Error("action has no textures");

    auto const N = (int)action.textures.size();
    auto const idx = ::clamp<int>(ratio * N, 0, N - 1);

    if(halfSize.x < 0)
      pos.x -= halfSize.x;

    if(halfSize.y < 0)
      pos.y -= halfSize.y;

    auto const sx = halfSize.x;
    auto const sy = halfSize.y;

    const auto worldTransform = translate(pos) * rotate(angle) * scale(Vector2f(sx, sy));

    static const auto shrink = scale(SCALE * Vector2f(1, 1));
    const auto viewTransform = shrink * rotate(-cam.angle) * translate(-1 * cam.pos);

    const auto transform = viewTransform * worldTransform;

    Quad q;
    q.zOrder = zOrder;
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

      if(blinking)
      {
        if((m_frameCount / 4) % 2)
        {
          q.light[0] = 0.8;
          q.light[1] = 0.4;
          q.light[2] = 0.4;
        }
      }
    }

    m_quads.push_back(q);
  }

  Camera m_camera;
  bool m_cameraValid = false;

  std::unique_ptr<IGpuProgram> m_shader;

  struct Quad
  {
    int zOrder;
    std::array<float, 3> light {};
    int tile;
    Vector2f pos[4];
  };

  struct Tile
  {
    ITexture* texture;
    Vector2f uv[2];
  };

  vector<Quad> m_quads;
  std::unique_ptr<IVertexBuffer> m_batchVbo;
  std::unique_ptr<IVertexBuffer> m_quadVbo;
  std::unique_ptr<IFrameBuffer> m_fb;

  unordered_map<int, Model> m_Models;
  unordered_map<std::string, std::unique_ptr<ITexture>> m_textures;
  std::vector<Tile> m_tiles;
  Model m_fontModel;

  float m_ambientLight = 0;
  int m_frameCount = 0;

  IGraphicsBackend* const backend;

public:
  int loadTexture(String path, Rect2f frect)
  {
    const std::string sPath(path.data, path.len);

    if(m_textures.find(sPath) == m_textures.end())
    {
      auto pic = loadPicture(path, Rect2f(0, 0, 1, 1));
      m_textures[sPath] = backend->createTexture();
      m_textures[sPath]->upload(pic);

      if(0)
        printf("[renderer] loaded '%.*s'\n", path.len, path.data);
    }

    const float left = frect.pos.x;
    const float right = frect.pos.x + frect.size.width;
    const float top = frect.pos.y;
    const float bottom = frect.pos.y + frect.size.height;

    const int id = (int)m_tiles.size();
    m_tiles.push_back({ m_textures[sPath].get(), { { left, top }, { right, bottom } } });
    return id;
  }
};
}

// exported to Model
int loadTexture(String path, Rect2f frect)
{
  return g_Renderer->loadTexture(path, frect);
}

IRenderer* createRenderer(IGraphicsBackend* gfxBackend)
{
  return new Renderer(gfxBackend);
}

