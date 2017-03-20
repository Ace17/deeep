/**
 * @brief OpenGL stuff
 * @author Sebastien Alaiwan
 */

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
#include "GL/glu.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "SDL_video.h"
#include "SDL_image.h"

#include "base/util.h"
#include "base/scene.h"
#include "base/geom.h"
#include "model.h"

static vector<Model> g_Models;
static GLuint g_ProgramId;

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
  ss << "Msg: " << gluErrorString(errorCode) << endl;
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
  int logLength;
  GLint Result;
  SAFE_GL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result));

  if(!Result)
  {
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    vector<char> msg(logLength);
    glGetShaderInfoLog(shaderId, logLength, nullptr, msg.data());
    cerr << msg.data();

    cerr << code << endl;

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
  int InfoLogLength;
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

  if(0)
  {
    vector<char> msgBuf(InfoLogLength);
    glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, msgBuf.data());
    cout << msgBuf.data();
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
Model rectangularModel(float w, float h)
{
  const GLfloat myTriangle[] =
  {
    0, h, 0, 0, 0,
    0, 0, 0, 0, 1,
    w, 0, 0, 1, 1,
    w, h, 0, 1, 0,
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
  SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(myTriangle), myTriangle, GL_STATIC_DRAW));

  SAFE_GL(glGenBuffers(1, &model.indices));
  SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices));
  SAFE_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

  model.numIndices = 6;

  return model;
}

static
Model loadAnimation(string path)
{
  auto m = rectangularModel(1, 1);

  if(endsWith(path, ".json"))
  {
    auto m2 = loadModel(path);
    m.actions = move(m2.actions);
  }
  else if(endsWith(path, ".mdl"))
  {
    path = setExtension(path, "png");

    for(int i = 0; i < 64 * 2; ++i)
    {
      auto const COLS = 8;
      auto col = i % COLS;
      auto row = i / COLS;

      auto const SIZE = 16;
      Action action;
      action.addTexture(path, Rect2i(col * SIZE, row * SIZE, SIZE, SIZE));
      m.actions.push_back(action);
    }
  }
  else
  {
    Action action;
    action.addTexture(path, Rect2i());
    m.actions.push_back(action);
  }

  return m;
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
}

void Display_setCaption(const char* caption)
{
  SDL_SetWindowTitle(mainWindow, caption);
}

void Display_drawActor(Rect2f where, int modelId, bool blinking, int actionIdx, float ratio)
{
  auto& model = g_Models.at(modelId);

  auto const colorId = glGetUniformLocation(g_ProgramId, "v_color");

  SAFE_GL(glUniform4f(colorId, 0, 0, 0, 0));

  if(blinking)
  {
    static int blinkCounter;
    blinkCounter++;

    if((blinkCounter / 4) % 2)
      SAFE_GL(glUniform4f(colorId, 0.8, 0.4, 0.4, 0));
  }

  auto const matrixId = glGetUniformLocation(g_ProgramId, "MVP");

  if(actionIdx < 0 || actionIdx >= (int)model.actions.size())
    throw runtime_error("invalid action index");

  auto const& action = model.actions[actionIdx];

  if(action.textures.empty())
    throw runtime_error("action has no textures");

  int idx = clamp<int>(ratio * action.textures.size(), 0, action.textures.size() - 1);
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

  SAFE_GL(glUniformMatrix4fv(matrixId, 1, GL_FALSE, mat));

  SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model.buffer));
  SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices));

  SAFE_GL(glDrawElements(GL_TRIANGLES, model.numIndices, GL_UNSIGNED_SHORT, 0));
}

void Display_beginDraw()
{
  SAFE_GL(glUseProgram(g_ProgramId));

  SAFE_GL(glClearColor(0, 0, 0, 1));
  SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));

  {
    auto const scaleMatrixId = glGetUniformLocation(g_ProgramId, "Scale");
    auto const s = 0.125f;
    float mat[16] =
    {
      s, 0, 0, 0,
      0, s, 0, 0,
      0, 0, s, 0,
      0, 0, 0, 1,
    };
    SAFE_GL(glUniformMatrix4fv(scaleMatrixId, 1, GL_FALSE, mat));
  }

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

