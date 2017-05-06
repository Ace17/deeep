// OpenGL stuff

/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>
using namespace std;

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "SDL_video.h"
#include "SDL_image.h"

#include "base/util.h"
#include "base/scene.h"
#include "base/geom.h"
#include "model.h"

static GLint g_MVP;
static GLint g_colorId;
static GLuint g_ProgramId;
static vector<Model> g_Models;
static Model g_fontModel;

#ifdef NDEBUG
#define SAFE_GL(a) a
#else
#define SAFE_GL(a) \
  do { a; ensureGl(# a, __LINE__); } while(0)
#endif

inline
void ensureGl(char const* expr, int line)
{
  auto const errorCode = glGetError();

  if(errorCode == GL_NO_ERROR)
    return;

  stringstream ss;
  ss << "OpenGL error" << endl;
  ss << "Expr: " << expr << endl;
  ss << "Line: " << line << endl;
  ss << "Code: " << errorCode;
  throw runtime_error(ss.str());
}

static
int compileShader(string code, int type)
{
  auto shaderId = glCreateShader(type);

  cout << "Compiling " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader ... ";
  auto srcPtr = code.c_str();
  SAFE_GL(glShaderSource(shaderId, 1, &srcPtr, nullptr));
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
    cerr << msg.data();

    throw runtime_error("Can't compile shader");
  }

  cout << "OK" << endl;

  return shaderId;
}

static
int linkShaders(vector<int> ids)
{
  // Link the program
  cout << "Linking shaders ... ";
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
    cout << msg.data();

    throw runtime_error("Can't link shader");
  }

  cout << "OK" << endl;

  return ProgramID;
}

static
SDL_Surface* loadPicture(string path)
{
  auto surface = IMG_Load((char*)path.c_str());

  if(!surface)
    throw runtime_error(string("Can't load texture: ") + SDL_GetError());

  if(surface->format->BitsPerPixel != 32)
    throw runtime_error("only 32 bit pictures are supported");

  return surface;
}

int loadTexture(string path, Rect2i rect)
{
  auto surface = loadPicture(path);

  if(rect.width == 0 && rect.height == 0)
    rect = Rect2i(0, 0, surface->w, surface->h);

  if(rect.x < 0 || rect.y < 0 || rect.x + rect.width > surface->w || rect.y + rect.height > surface->h)
    throw runtime_error("Invalid boundaries for '" + path + "'");

  GLuint texture;

  auto const bpp = surface->format->BytesPerPixel;

  vector<uint8_t> img(rect.width* rect.height* bpp);

  auto src = (Uint8*)surface->pixels + rect.x * bpp + rect.y * surface->pitch;
  auto dst = (Uint8*)img.data();

  for(int y = 0; y < rect.height; ++y)
  {
    memcpy(dst, src, bpp * rect.width);
    src += surface->pitch;
    dst += bpp * rect.width;
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rect.width, rect.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  SDL_FreeSurface(surface);

  return texture;
}

extern char VertexShaderCode[];
extern size_t VertexShaderCode_size;
extern char FragmentShaderCode[];
extern size_t FragmentShaderCode_size;

static
GLuint loadShaders()
{
  auto const vsCode = string(VertexShaderCode, VertexShaderCode + VertexShaderCode_size);
  auto const fsCode = string(FragmentShaderCode, FragmentShaderCode + FragmentShaderCode_size);
  auto const vertexId = compileShader(vsCode, GL_VERTEX_SHADER);
  auto const fragmentId = compileShader(fsCode, GL_FRAGMENT_SHADER);

  auto const progId = linkShaders(vector<int>({ vertexId, fragmentId }));

  SAFE_GL(glDeleteShader(vertexId));
  SAFE_GL(glDeleteShader(fragmentId));

  return progId;
}

static
Model boxModel()
{
  float w = 1;
  float h = 1;

  const GLfloat myBox[] =
  {
    0, h, 0, /* uv */ 0, 0,
    0, 0, 0, /* uv */ 0, 1,
    w, 0, 0, /* uv */ 1, 1,
    w, h, 0, /* uv */ 1, 0,
  };

  const GLushort indices[] =
  {
    0, 1, 2,
    2, 3, 0,
  };

  Model model;

  model.size = 4;

  SAFE_GL(glGenBuffers(1, &model.buffer));
  SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model.buffer));
  SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(myBox), myBox, GL_STATIC_DRAW));

  SAFE_GL(glGenBuffers(1, &model.indices));
  SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices));
  SAFE_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

  model.numIndices = sizeof(indices) / sizeof(*indices);

  return model;
}

static
Model loadTiledAnimation(string path, int count, int COLS, int SIZE)
{
  auto m = boxModel();

  for(int i = 0; i < count; ++i)
  {
    auto col = i % COLS;
    auto row = i / COLS;

    Action action;
    action.addTexture(path, Rect2i(col * SIZE, row * SIZE, SIZE, SIZE));
    m.actions.push_back(action);
  }

  return m;
}

static
Model loadAnimation(string path)
{
  if(endsWith(path, ".json"))
  {
    auto m = boxModel();

    auto m2 = loadModel(path);
    m.actions = move(m2.actions);
    return m;
  }
  else if(endsWith(path, ".mdl"))
  {
    path = setExtension(path, "png");

    return loadTiledAnimation(path, 64 * 2, 8, 16);
  }
  else
  {
    auto m = boxModel();
    Action action;
    action.addTexture(path, Rect2i());
    m.actions.push_back(action);
    return m;
  }
}

void Display_loadModel(int id, const char* path)
{
  if((int)g_Models.size() <= id)
    g_Models.resize(id + 1);

  g_Models[id] = loadAnimation(path);
}

static
void printOpenGlVersion()
{
  auto sVersion = (char const*)glGetString(GL_VERSION);
  auto sLangVersion = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

  auto notNull = [] (char const* s) -> string
                 {
                   return s ? s : "<null>";
                 };

  cout << "OpenGL version: " << notNull(sVersion) << endl;
  cout << "OpenGL shading version: " << notNull(sLangVersion) << endl;
}

static SDL_Window* mainWindow;
static SDL_GLContext mainContext;

void Display_init(int width, int height)
{
  if(SDL_InitSubSystem(SDL_INIT_VIDEO))
    throw runtime_error("Can't init SDL");

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  mainWindow = SDL_CreateWindow(
    "My Game",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height,
    SDL_WINDOW_OPENGL
    );

  if(!mainWindow)
    throw runtime_error("Can't set video mode");

  // Create our opengl context and attach it to our window
  mainContext = SDL_GL_CreateContext(mainWindow);

  printOpenGlVersion();

  // This makes our buffer swap syncronized with the monitor's vertical refresh
  SDL_GL_SetSwapInterval(1);

  GLuint VertexArrayID;
  SAFE_GL(glGenVertexArrays(1, &VertexArrayID));
  SAFE_GL(glBindVertexArray(VertexArrayID));

  g_ProgramId = loadShaders();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  g_fontModel = loadTiledAnimation("res/font.png", 256, 16, 8);

  g_MVP = glGetUniformLocation(g_ProgramId, "MVP");
  assert(g_MVP >= 0);

  g_colorId = glGetUniformLocation(g_ProgramId, "v_color");
  assert(g_colorId >= 0);
}

void Display_setCaption(const char* caption)
{
  SDL_SetWindowTitle(mainWindow, caption);
}

static
void drawModel(Rect2f where, Model const& model, bool blinking, int actionIdx, float ratio)
{
  SAFE_GL(glUniform4f(g_colorId, 0, 0, 0, 0));

  if(blinking)
  {
    static int blinkCounter;
    blinkCounter++;

    if((blinkCounter / 4) % 2)
      SAFE_GL(glUniform4f(g_colorId, 0.8, 0.4, 0.4, 0));
  }

  if(actionIdx < 0 || actionIdx >= (int)model.actions.size())
    throw runtime_error("invalid action index");

  auto const& action = model.actions[actionIdx];

  if(action.textures.empty())
    throw runtime_error("action has no textures");

  auto const N = (int)action.textures.size();
  auto const idx = ::clamp<int>(ratio * N, 0, N - 1);
  glBindTexture(GL_TEXTURE_2D, action.textures[idx]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if(where.width < 0)
    where.x -= where.width;

  if(where.height < 0)
    where.y -= where.height;

  auto const dx = where.x;
  auto const dy = where.y;

  auto const sx = where.width;
  auto const sy = where.height;

  float mat[16] =
  {
    sx, 0, 0, 0,
    0, sy, 0, 0,
    0, 0, 1, 0,
    dx, dy, 0, 1,
  };

  // scaling
  {
    for(auto& val : mat)
      val *= 0.125;

    mat[15] = 1;
  }

  SAFE_GL(glUniformMatrix4fv(g_MVP, 1, GL_FALSE, mat));

  SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model.buffer));
  SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices));

  SAFE_GL(glDrawElements(GL_TRIANGLES, model.numIndices, GL_UNSIGNED_SHORT, 0));
}

void Display_drawActor(Rect2f where, int modelId, bool blinking, int actionIdx, float ratio)
{
  auto& model = g_Models.at(modelId);
  drawModel(where, model, blinking, actionIdx, ratio);
}

void Display_drawText(Vector2f pos, char const* text)
{
  Rect2f rect;
  rect.width = 0.5;
  rect.height = 0.5;
  rect.x = pos.x - strlen(text) * rect.width / 2;
  rect.y = pos.y;

  while(*text)
  {
    drawModel(rect, g_fontModel, false, *text, 0);
    rect.x += rect.width;
    ++text;
  }
}

void Display_beginDraw()
{
  SAFE_GL(glUseProgram(g_ProgramId));

  SAFE_GL(glClearColor(0, 0, 0, 1));
  SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));

  {
    auto const positionLoc = glGetAttribLocation(g_ProgramId, "a_position");
    auto const texCoordLoc = glGetAttribLocation(g_ProgramId, "a_texCoord");

    assert(positionLoc >= 0);
    assert(texCoordLoc >= 0);

    // connect the xyz to the "a_position" attribute of the vertex shader
    SAFE_GL(glEnableVertexAttribArray(positionLoc));
    SAFE_GL(glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr));

    // connect the uv coords to the "v_texCoord" attribute of the vertex shader
    SAFE_GL(glEnableVertexAttribArray(texCoordLoc));
    SAFE_GL(glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat))));
  }
}

void Display_endDraw()
{
  SDL_GL_SwapWindow(mainWindow);
}

