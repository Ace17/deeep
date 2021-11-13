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
#include "base/scene.h"
#include "base/span.h"
#include "base/util.h" // clamp
#include "engine/display.h"
#include "engine/graphics_backend.h"
#include "engine/stats.h"
#include "matrix3.h"
#include "misc/file.h"
#include "misc/util.h"
#include "model.h"
#include "picture.h"

namespace
{
const int MAX_VERTICES = 8192;

Vector2f multiplyMatrix(const Matrix3f& mat, float v0, float v1, float v2)
{
  Vector2f r;
  r.x = mat[0][0] * v0 + mat[0][1] * v1 + mat[0][2] * v2;
  r.y = mat[1][0] * v0 + mat[1][1] * v1 + mat[1][2] * v2;
  return r;
}

class Renderer;
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

template<typename T>
T blend(T a, T b, float alpha)
{
  return a * (1 - alpha) + b * alpha;
}

struct Renderer : Display
{
  Renderer(IGraphicsBackend* backend_)
    : backend(backend_)
  {
    g_Renderer = this;
    m_shader = backend->createGpuProgram("standard", false);
    m_batchVbo = backend->createVertexBuffer();

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
#if 0
    {
      int w, h;
      SDL_GL_GetDrawableSize(m_window, &w, &h);
      auto size = min(w, h);
      SAFE_GL(glViewport((w - size) / 2, (h - size) / 2, size, size));
    }
#endif

    backend->useGpuProgram(m_shader.get());
    backend->clear();

    auto byPriority = [] (Quad const& a, Quad const& b)
      {
        if(a.zOrder != b.zOrder)
          return a.zOrder < b.zOrder;

        if(a.light != b.light)
          return a.light < b.light;

        return a.texture < b.texture;
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

    int currTexture = -1;
    std::array<float, 3> currLight {};
    currLight[0] = 0.0f / 0.0f;

    for(auto const& q : m_quads)
    {
      if(vboData.size() * 6 >= MAX_VERTICES)
        flush();

      if(q.texture != currTexture)
      {
        flush();

        // Bind our diffuse texture in Texture Unit 0
        m_textures[q.texture]->bind(0);
        currTexture = q.texture;
      }

      if(q.light != currLight)
      {
        flush();

        backend->setUniformFloat4(0 /* colorLoc */, q.light[0], q.light[1], q.light[2], 0);
        currLight = q.light;
      }

      vboData.push_back({ q.pos[0].x, q.pos[0].y, 0, 0 });
      vboData.push_back({ q.pos[1].x, q.pos[1].y, 0, 1 });
      vboData.push_back({ q.pos[2].x, q.pos[2].y, 1, 1 });

      vboData.push_back({ q.pos[0].x, q.pos[0].y, 0, 0 });
      vboData.push_back({ q.pos[2].x, q.pos[2].y, 1, 1 });
      vboData.push_back({ q.pos[3].x, q.pos[3].y, 1, 0 });
    }

    flush();

    backend->swap();
  }

  void drawActor(Rect2f where, float angle, bool useWorldRefFrame, int modelId, bool blinking, int actionIdx, float ratio, int zOrder) override
  {
    auto& model = m_Models.at(modelId);
    auto cam = useWorldRefFrame ? m_camera : Camera();
    pushQuad(where, angle, cam, model, blinking, actionIdx, ratio, zOrder);
  }

  void drawText(Vector2f pos, char const* text) override
  {
    Rect2f rect;
    rect.size.width = 0.5;
    rect.size.height = 0.5;
    rect.pos.x = pos.x - strlen(text) * rect.size.width / 2;
    rect.pos.y = pos.y;

    while(*text)
    {
      pushQuad(rect, 0, {}, m_fontModel, false, *text, 0, 100);
      rect.pos.x += rect.size.width;
      ++text;
    }
  }

private:
  void pushQuad(Rect2f where, float angle, Camera cam, Model const& model, bool blinking, int actionIdx, float ratio, int zOrder)
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

    if(where.size.width < 0)
      where.pos.x -= where.size.width;

    if(where.size.height < 0)
      where.pos.y -= where.size.height;

    auto const sx = where.size.width;
    auto const sy = where.size.height;

    const auto worldTransform = translate(where.pos) * rotate(angle) * scale(Vector2f(sx, sy));

    static const auto shrink = scale(0.125 * Vector2f(1, 1));
    const auto viewTransform = shrink * rotate(-cam.angle) * translate(-1 * cam.pos);

    const auto transform = viewTransform * worldTransform;

    Quad q;
    q.zOrder = zOrder;
    q.texture = action.textures[idx];

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
    int texture;
    Vector2f pos[4];
  };

  vector<Quad> m_quads;
  std::unique_ptr<IVertexBuffer> m_batchVbo;
  std::vector<std::unique_ptr<ITexture>> m_textures;

  unordered_map<int, Model> m_Models;
  Model m_fontModel;

  float m_ambientLight = 0;
  int m_frameCount = 0;

  IGraphicsBackend* const backend;

public:
  int loadTexture(String path, Rect2f frect)
  {
    const int id = (int)m_textures.size();
    auto pic = loadPicture(path, frect);
    m_textures.push_back(backend->createTexture());
    m_textures.back()->upload(pic);
    return id;
  }
};
}

// exported to Model
int loadTexture(String path, Rect2f frect)
{
  return g_Renderer->loadTexture(path, frect);
}

Display* createRenderer(IGraphicsBackend* gfxBackend)
{
  return new Renderer(gfxBackend);
}

