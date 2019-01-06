// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// OpenGL stuff

#include "display.h"

#include <cassert>
#include <cstdio>
#include <vector>
#include <map>
#include <stdexcept>
using namespace std;

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "SDL.h" // SDL_INIT_VIDEO

#include "base/util.h" // clamp
#include "base/scene.h"
#include "base/geom.h"
#include "base/span.h"
#include "misc/file.h"
#include "misc/util.h"
#include "model.h"
#include "matrix3.h"
#include "png.h"

#ifdef NDEBUG
#define SAFE_GL(a) a
#else
#define SAFE_GL(a) \
  do { a; ensureGl(# a, __LINE__); } while(0)
#endif

static const int MAX_QUADS = 2048;

static
void ensureGl(char const* expr, int line)
{
  auto const errorCode = glGetError();

  if(errorCode == GL_NO_ERROR)
    return;

  string ss;
  ss += "OpenGL error\n";
  ss += "Expr: " + string(expr) + "\n";
  ss += "Line: " + to_string(line) + "\n";
  ss += "Code: " + to_string(errorCode) + "\n";
  throw runtime_error(ss);
}

static
int compileShader(Span<uint8_t> code, int type)
{
  auto shaderId = glCreateShader(type);

  if(!shaderId)
    throw runtime_error("Can't create shader");

  printf("[display] compiling %s shader ... ", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
  auto srcPtr = (const char*)code.data;
  auto length = (GLint)code.len;
  SAFE_GL(glShaderSource(shaderId, 1, &srcPtr, &length));
  SAFE_GL(glCompileShader(shaderId));

  // Check compile result
  GLint Result;
  SAFE_GL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result));

  if(!Result)
  {
    int logLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    vector<char> msg(logLength);
    glGetShaderInfoLog(shaderId, logLength, nullptr, msg.data());
    fprintf(stderr, "%s\n", msg.data());

    throw runtime_error("Can't compile shader");
  }

  printf("OK\n");

  return shaderId;
}

static
int linkShaders(vector<int> ids)
{
  // Link the program
  printf("[display] Linking shaders ... ");
  auto ProgramID = glCreateProgram();

  for(auto id : ids)
    glAttachShader(ProgramID, id);

  glLinkProgram(ProgramID);

  // Check the program
  GLint Result = GL_FALSE;
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);

  if(!Result)
  {
    int logLength;
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logLength);
    vector<char> msg(logLength);
    glGetProgramInfoLog(ProgramID, logLength, nullptr, msg.data());
    fprintf(stderr, "%s\n", msg.data());

    throw runtime_error("Can't link shader");
  }

  printf("OK\n");

  return ProgramID;
}

struct Picture
{
  int width, height;
  int stride;
  vector<uint8_t> pixels;
};

static
Picture loadPng(string path)
{
  Picture pic;
  auto pngDataBuf = read(path);
  auto pngData = Span<const uint8_t>((uint8_t*)pngDataBuf.data(), (int)pngDataBuf.size());
  pic.pixels = decodePng(pngData, pic.width, pic.height);
  pic.stride = pic.width * 4;

  return pic;
}

static
Picture* getPicture(string path)
{
  static map<string, Picture> cache;

  if(cache.find(path) == cache.end())
    cache[path] = loadPng(path);

  return &cache.at(path);
}

// exported to Model
int loadTexture(string path, Rect2i rect)
{
  auto surface = getPicture(path);

  if(rect.size.width == 0 && rect.size.height == 0)
    rect = Rect2i(0, 0, surface->width, surface->height);

  if(rect.pos.x < 0 || rect.pos.y < 0 || rect.pos.x + rect.size.width > surface->width || rect.pos.y + rect.size.height > surface->height)
    throw runtime_error("Invalid boundaries for '" + path + "'");

  auto const bpp = 4;

  vector<uint8_t> img(rect.size.width* rect.size.height* bpp);

  auto src = (Uint8*)surface->pixels.data() + rect.pos.x * bpp + rect.pos.y * surface->stride;
  auto dst = (Uint8*)img.data() + bpp * rect.size.width * rect.size.height;

  // from glTexImage2D doc:
  // "The first element corresponds to the lower left corner of the texture image",
  // (e.g (u,v) = (0,0))
  for(int y = 0; y < rect.size.height; ++y)
  {
    dst -= bpp * rect.size.width;
    memcpy(dst, src, bpp * rect.size.width);
    src += surface->stride;
  }

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rect.size.width, rect.size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return texture;
}

extern const Span<uint8_t> VertexShaderCode;
extern const Span<uint8_t> FragmentShaderCode;

static
GLuint loadShaders()
{
  auto const vertexId = compileShader(VertexShaderCode, GL_VERTEX_SHADER);
  auto const fragmentId = compileShader(FragmentShaderCode, GL_FRAGMENT_SHADER);

  auto const progId = linkShaders(vector<int>({ vertexId, fragmentId }));

  SAFE_GL(glDeleteShader(vertexId));
  SAFE_GL(glDeleteShader(fragmentId));

  return progId;
}

static
Model boxModel()
{
  const float w = 1;
  const float h = 1;

  Model model;

  model.vertices.push_back({ 0, h, /* uv */ 0, 1 });
  model.vertices.push_back({ 0, 0, /* uv */ 0, 0 });
  model.vertices.push_back({ w, 0, /* uv */ 1, 0 });

  model.vertices.push_back({ w, 0, /* uv */ 1, 0 });
  model.vertices.push_back({ w, h, /* uv */ 1, 1 });
  model.vertices.push_back({ 0, h, /* uv */ 0, 1 });

  return model;
}

struct Camera
{
  Vector2f pos = Vector2f(0, 0);
  float angle = 0;
};

static
Model loadTiledModel(string path, int count, int COLS, int SIZE)
{
  auto m = boxModel();

  for(int i = 0; i < count; ++i)
  {
    auto col = i % COLS;
    auto row = i / COLS;

    Action action;
    addTexture(action, path, Rect2i(col * SIZE, row * SIZE, SIZE, SIZE));
    m.actions.push_back(action);
  }

  return m;
}

static
Model loadAnimation(string path)
{
  if(endsWith(path, ".model"))
  {
    auto m = boxModel();

    if(!exists(path))
    {
      printf("[display] model '%s' doesn't exist, fallback on default model\n", path.c_str());
      path = "res/sprites/rect.model";
    }

    auto m2 = loadModel(path);
    m.actions = move(m2.actions);
    return m;
  }
  else if(endsWith(path, ".tiles"))
  {
    path = setExtension(path, "png");

    if(!exists(path))
    {
      printf("[display] tileset '%s' was not found, fallback on default tileset\n", path.c_str());
      path = "res/tiles/default.png";
    }

    return loadTiledModel(path, 64 * 2, 8, 16);
  }
  else
  {
    throw runtime_error("unknown format for '" + path + "'");
  }
}

static
void printOpenGlVersion()
{
  auto sVersion = (char const*)glGetString(GL_VERSION);
  auto sLangVersion = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

  auto notNull = [] (char const* s) -> char const*
    {
      return s ? s : "<null>";
    };

  printf("[display] OpenGL version: %s\n", notNull(sVersion));
  printf("[display] OpenGL shading version: %s\n", notNull(sLangVersion));
}

static
void setOpenglMatrix4f(GLint matrixId, Matrix3f const& mat)
{
  float mvp[4][4] = { 0 };

  // opengl expects a column-major matrix
  // (i.e columns are contiguous in memory)
  // So the indexing is:
  // mvp[col][row] = mat[row][col]

  // row #0
  mvp[0][0] = mat[0][0];
  mvp[1][0] = mat[0][1];
  mvp[2][0] = 0;
  mvp[3][0] = mat[0][2];

  // row #1
  mvp[0][1] = mat[1][0];
  mvp[1][1] = mat[1][1];
  mvp[2][1] = 0;
  mvp[3][1] = mat[1][2];

  // row #2
  mvp[0][2] = 0;
  mvp[1][2] = 0;
  mvp[2][2] = 1;
  mvp[3][2] = 0;

  // row #3
  mvp[0][3] = mat[2][0];
  mvp[1][3] = mat[2][1];
  mvp[2][3] = 0;
  mvp[3][3] = mat[2][2];

  SAFE_GL(glUniformMatrix4fv(matrixId, 1, GL_FALSE, mvp[0]));
};

template<typename T>
T blend(T a, T b, float alpha)
{
  return a * (1 - alpha) + b * alpha;
}

struct OpenglDisplay : Display
{
  OpenglDisplay(Size2i resolution)
  {
    if(SDL_InitSubSystem(SDL_INIT_VIDEO))
      throw runtime_error("Can't init SDL");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // require OpenGL 2.0, ES or Core. No compatibility mode.
    {
      // SDL_GL_CONTEXT_PROFILE_ES: works in browser, not in native
      // SDL_GL_CONTEXT_PROFILE_CORE: works in native, not in browser
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE | SDL_GL_CONTEXT_PROFILE_ES);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }

    m_window = SDL_CreateWindow(
        "My Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        resolution.width, resolution.height,
        SDL_WINDOW_OPENGL
        );

    if(!m_window)
      throw runtime_error("Can't set video mode");

    // Create our opengl context and attach it to our window
    m_context = SDL_GL_CreateContext(m_window);

    if(!m_context)
      throw runtime_error("Can't create OpenGL context");

    printOpenGlVersion();

    // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);

    GLuint VertexArrayID;
    SAFE_GL(glGenVertexArrays(1, &VertexArrayID));
    SAFE_GL(glBindVertexArray(VertexArrayID));

    m_programId = loadShaders();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_fontModel = loadTiledModel("res/font.png", 256, 16, 8);
    sendToOpengl(m_fontModel);

    m_MVP = glGetUniformLocation(m_programId, "MVP");
    assert(m_MVP >= 0);

    m_colorId = glGetUniformLocation(m_programId, "fragOffset");
    assert(m_colorId >= 0);

    m_positionLoc = glGetAttribLocation(m_programId, "vertexPos_model");
    assert(m_positionLoc >= 0);

    m_texCoordLoc = glGetAttribLocation(m_programId, "vertexUV");
    assert(m_texCoordLoc >= 0);

    SAFE_GL(glGenBuffers(1, &m_batchVbo));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, m_batchVbo));
    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Model::Vertex) * 4 * MAX_QUADS, NULL, GL_DYNAMIC_DRAW));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    printf("[display] init OK\n");
  }

  ~OpenglDisplay()
  {
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    printf("[display] shutdown OK\n");
  }

  void loadModel(int id, const char* path) override
  {
    if((int)m_Models.size() <= id)
      m_Models.resize(id + 1);

    m_Models[id] = loadAnimation(path);
    sendToOpengl(m_Models[id]);
  }

  void sendToOpengl(Model& model)
  {
    SAFE_GL(glGenBuffers(1, &model.buffer));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model.buffer));
    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(model.vertices[0]) * model.vertices.size(), model.vertices.data(), GL_STATIC_DRAW));

    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  }

  void setCamera(Vector2f pos) override
  {
    auto cam = (Camera { pos, 0 });

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

  void setFullscreen(bool fs) override
  {
    auto flags = fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(m_window, flags);
  }

  void setCaption(const char* caption) override
  {
    SDL_SetWindowTitle(m_window, caption);
  }

  void setAmbientLight(float ambientLight) override
  {
    m_ambientLight = ambientLight;
  }

  void drawModel(Rect2f where, Camera cam, Model const& model, bool blinking, int actionIdx, float ratio)
  {
    if(model.actions.empty())
      throw runtime_error("model has no actions");

    if(actionIdx < 0 || actionIdx >= (int)model.actions.size())
      throw runtime_error("invalid action index");

    auto const& action = model.actions[actionIdx];

    if(action.textures.empty())
      throw runtime_error("action has no textures");

    auto const N = (int)action.textures.size();
    auto const idx = ::clamp<int>(ratio * N, 0, N - 1);

    if(where.size.width < 0)
      where.pos.x -= where.size.width;

    if(where.size.height < 0)
      where.pos.y -= where.size.height;

    auto const relPos = where.pos - cam.pos;

    auto const sx = where.size.width;
    auto const sy = where.size.height;

    auto mat = scale(Vector2f(sx, sy));
    mat = translate(relPos) * mat;
    mat = rotate(cam.angle) * mat;

    auto shrink = scale(0.125 * Vector2f(1, 1));
    mat = shrink * mat;

    Quad q;
    q.texture = action.textures[idx];

    auto const m0x = 0;
    auto const m0y = 0;
    auto const m1x = 1;
    auto const m1y = 1;

    q.pos1.x = mat[0][0] * m0x + mat[0][1] * m0y + mat[0][2];
    q.pos1.y = mat[1][0] * m0x + mat[1][1] * m0y + mat[1][2];

    q.pos2.x = mat[0][0] * m1x + mat[0][1] * m1y + mat[0][2];
    q.pos2.y = mat[1][0] * m1x + mat[1][1] * m1y + mat[1][2];

    // Don't call opengl if the object isn't visible.
    // Huge FPS boost.
    if(1)
    {
      auto const MAX = 1.0;

      if(q.pos1.x < -MAX && q.pos2.x < -MAX)
        return;

      if(q.pos1.x > +MAX && q.pos2.x > +MAX)
        return;

      if(q.pos1.y < -MAX && q.pos2.y < -MAX)
        return;

      if(q.pos1.y > +MAX && q.pos2.y > +MAX)
        return;
    }

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

  void drawActor(Rect2f where, bool useWorldRefFrame, int modelId, bool blinking, int actionIdx, float ratio) override
  {
    auto& model = m_Models.at(modelId);
    auto cam = useWorldRefFrame ? m_camera : Camera();
    drawModel(where, cam, model, blinking, actionIdx, ratio);
  }

  void drawText(Vector2f pos, char const* text) override
  {
    Rect2f rect;
    rect.size.width = 0.5;
    rect.size.height = 0.5;
    rect.pos.x = pos.x - strlen(text) * rect.size.width / 2;
    rect.pos.y = pos.y;

    auto cam = Camera();

    while(*text)
    {
      drawModel(rect, cam, m_fontModel, false, *text, 0);
      rect.pos.x += rect.size.width;
      ++text;
    }
  }

  void beginDraw() override
  {
    m_frameCount++;

    {
      int w, h;
      SDL_GL_GetDrawableSize(m_window, &w, &h);
      auto size = min(w, h);
      SAFE_GL(glViewport((w - size) / 2, (h - size) / 2, size, size));
    }

    SAFE_GL(glUseProgram(m_programId));

    SAFE_GL(glClearColor(0, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));

    SAFE_GL(glEnableVertexAttribArray(m_positionLoc));
    SAFE_GL(glEnableVertexAttribArray(m_texCoordLoc));

    m_quads.clear();
  }

  void endDraw() override
  {
#define OFFSET(a) \
  ((GLvoid*)(&((Model::Vertex*)nullptr)->a))

    setOpenglMatrix4f(m_MVP, scale({ 1, 1 }));

    for(auto const& q : m_quads)
    {
      SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, m_batchVbo));

      m_batchVboData.clear();
      m_batchVboData.push_back({ q.pos1.x, q.pos1.y, 0, 0 });
      m_batchVboData.push_back({ q.pos1.x, q.pos2.y, 0, 1 });
      m_batchVboData.push_back({ q.pos2.x, q.pos2.y, 1, 1 });
      m_batchVboData.push_back({ q.pos2.x, q.pos1.y, 1, 0 });

      if(m_batchVboData.size() > MAX_QUADS)
        m_batchVboData.resize(MAX_QUADS);

      SAFE_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Quad) * m_batchVboData.size(), m_batchVboData.data()));

      SAFE_GL(glEnableVertexAttribArray(m_positionLoc));
      SAFE_GL(glVertexAttribPointer(m_positionLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), OFFSET(x)));

      SAFE_GL(glEnableVertexAttribArray(m_texCoordLoc));
      SAFE_GL(glVertexAttribPointer(m_texCoordLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), OFFSET(u)));

      SAFE_GL(glUniform4f(m_colorId, q.light[0], q.light[1], q.light[2], 0));

      // Bind our diffuse texture in Texture Unit 0
      SAFE_GL(glActiveTexture(GL_TEXTURE0));
      SAFE_GL(glBindTexture(GL_TEXTURE_2D, q.texture));
      SAFE_GL(glDrawArrays(GL_QUADS, 0, m_batchVboData.size()));
    }

    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, 0));

    SDL_GL_SwapWindow(m_window);
#undef OFFSET
  }

  // end-of public API

  SDL_Window* m_window;
  SDL_GLContext m_context;

  Camera m_camera;
  bool m_cameraValid = false;

  GLint m_MVP;
  GLint m_colorId;
  GLint m_positionLoc;
  GLint m_texCoordLoc;

  struct Quad
  {
    float light[3] {};
    GLuint texture;
    Vector2f pos1, pos2;
  };

  vector<Quad> m_quads;
  vector<Model::Vertex> m_batchVboData;
  GLuint m_batchVbo;

  GLuint m_programId;
  vector<Model> m_Models;
  Model m_fontModel;

  float m_ambientLight = 0;
  int m_frameCount = 0;
};

Display* createDisplay(Size2i resolution)
{
  return new OpenglDisplay(resolution);
}

