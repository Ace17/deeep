// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

///////////////////////////////////////////////////////////////////////////////
// OpenGL stuff

#include <cassert>
#include <memory>
#include <vector>

#include "base/error.h"
#include "base/geom.h"
#include "base/logger.h"
#include "base/span.h"
#include "engine/graphics_backend.h"
#include "misc/file.h"
#include "misc/stats.h"
#include "render/picture.h"

#include "glad.h"
#include "SDL.h" // SDL_INIT_VIDEO

#ifdef NDEBUG
#define SAFE_GL(a) a
#else
#define SAFE_GL(a) \
        do { a; ensureGl(# a, __FILE__, __LINE__); } while(0)
#endif

void ensureGl(char const* expr, const char* file, int line)
{
  auto const errorCode = glGetError();

  if(errorCode == GL_NO_ERROR)
    return;

  std::string ss;
  ss += "OpenGL error\n";
  ss += std::string(file) + "(" + std::to_string(line) + "): " + std::string(expr) + "\n";
  ss += "Error code: " + std::to_string(errorCode) + "\n";
  throw Error(ss);
}

namespace
{
Gauge ggDrawCalls("Draw calls");

const float AspectRatio = 1; // square aspect ratio

GLuint compileShader(Span<const uint8_t> code, int type)
{
  auto shaderId = glCreateShader(type);

  if(!shaderId)
    throw Error("Can't create shader");

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
    std::vector<char> msg(logLength);
    glGetShaderInfoLog(shaderId, logLength, nullptr, msg.data());
    logMsg("%s", msg.data());

    throw Error("Can't compile shader");
  }

  return shaderId;
}

GLuint linkShaders(std::vector<GLuint> ids)
{
  // Link the program
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
    std::vector<char> msg(logLength);
    glGetProgramInfoLog(ProgramID, logLength, nullptr, msg.data());
    logMsg("%s", msg.data());

    throw Error("Can't link shader");
  }

  return ProgramID;
}

GLuint loadShaders(Span<const uint8_t> vsCode, Span<const uint8_t> fsCode)
{
  auto const vertexId = compileShader(vsCode, GL_VERTEX_SHADER);
  auto const fragmentId = compileShader(fsCode, GL_FRAGMENT_SHADER);

  auto const progId = linkShaders({ vertexId, fragmentId });

  SAFE_GL(glDeleteShader(vertexId));
  SAFE_GL(glDeleteShader(fragmentId));

  return progId;
}

void printOpenGlVersion()
{
  auto notNull = [] (const void* s) -> char const*
    {
      return s ? (const char*)s : "<null>";
    };

  logMsg("[display] %s [%s]",
         notNull(glGetString(GL_VERSION)),
         notNull(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  logMsg("[display] GPU: %s [%s]",
         notNull(glGetString(GL_RENDERER)),
         notNull(glGetString(GL_VENDOR)));
}

struct OpenGlProgram : IGpuProgram
{
  OpenGlProgram(GLuint program_, bool zTest_) : program(program_), zTest(zTest_)
  {
    uniformBlockIndex = glGetUniformBlockIndex(program, "MyUniformBlock");
  }

  ~OpenGlProgram()
  {
    glDeleteProgram(program);
  }

  int uniformBlockIndex = -1;
  const GLuint program;
  const bool zTest;
};

struct OpenGlTexture : ITexture
{
  OpenGlTexture()
  {
    SAFE_GL(glGenTextures(1, &texture));
  }

  ~OpenGlTexture()
  {
    glDeleteTextures(1, &texture);
  }

  void upload(PictureView pic) override
  {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pic.dim.x, pic.dim.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic.pixels);
    SAFE_GL(glGenerateMipmap(GL_TEXTURE_2D));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void setNoRepeat() override
  {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  void bind(int unit) override
  {
    SAFE_GL(glActiveTexture(GL_TEXTURE0 + unit));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, texture));
  }

  GLuint texture;
};

struct OpenGlFrameBuffer : IFrameBuffer
{
  OpenGlFrameBuffer(Vec2i resolution, bool depth) : resolution(resolution)
  {
    SAFE_GL(glGenFramebuffers(1, &framebuffer));
    SAFE_GL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

    // Z-buffer
    if(depth)
    {
      auto depthTexture = std::make_unique<OpenGlTexture>();

      SAFE_GL(glBindTexture(GL_TEXTURE_2D, depthTexture->texture));
      SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, resolution.x, resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL));
      SAFE_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture->texture, 0));

      this->depthTexture = std::move(depthTexture);
    }

    // color buffer
    {
      auto colorTexture = std::make_unique<OpenGlTexture>();

      SAFE_GL(glBindTexture(GL_TEXTURE_2D, colorTexture->texture));
      SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr));
      SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
      SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
      SAFE_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture->texture, 0));

      this->colorTexture = std::move(colorTexture);
    }
  }

  ~OpenGlFrameBuffer()
  {
    SAFE_GL(glDeleteFramebuffers(1, &framebuffer));
  }

  ITexture* getColorTexture() override
  {
    return colorTexture.get();
  }

  const Vec2i resolution;
  GLuint framebuffer;
  std::unique_ptr<ITexture> colorTexture;
  std::unique_ptr<ITexture> depthTexture;
};

struct OpenGlVertexBuffer : IVertexBuffer
{
  OpenGlVertexBuffer()
  {
    SAFE_GL(glGenBuffers(1, &vbo));
  }

  ~OpenGlVertexBuffer()
  {
    glDeleteBuffers(1, &vbo);
  }

  void upload(const void* data, size_t len) override
  {
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, len, data, GL_DYNAMIC_DRAW));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  }

  GLuint vbo;
};

struct OpenGlGraphicsBackend : IGraphicsBackend
{
  OpenGlGraphicsBackend(Vec2i resolution)
  {
    if(SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
      char buffer[256];
      throw Error(format(buffer, "Can't init SDL video: %s", SDL_GetError()));
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // require GLES 3.0: works in both browser and native.
    // (GLES 3.0 corresponds to WebGL 2.0)
    {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }

    m_window = SDL_CreateWindow(
      "",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      resolution.x, resolution.y,
      SDL_WINDOW_OPENGL
      );

    if(!m_window)
    {
      char buffer[256];
      throw Error(format(buffer, "Can't create SDL window: %s", SDL_GetError()));
    }

    // Create our opengl context and attach it to our window
    m_context = SDL_GL_CreateContext(m_window);

    if(!m_context)
    {
      char buffer[256];
      throw Error(format(buffer, "Can't create OpenGL context: %s", SDL_GetError()));
    }

    if(!gladLoadGLES2Loader(&SDL_GL_GetProcAddress))
      throw Error("Can't load OpenGL");

    printOpenGlVersion();

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    // Create our unique vertex array
    GLuint VertexArrayID;
    SAFE_GL(glGenVertexArrays(1, &VertexArrayID));
    SAFE_GL(glBindVertexArray(VertexArrayID));

    // Create our unique uniform buffer
    SAFE_GL(glGenBuffers(1, &m_uniformBuffer));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    updateScreenSize();

    logMsg("[display] init OK ( %dx%d )", m_screenSize.x, m_screenSize.y);
  }

  ~OpenGlGraphicsBackend()
  {
    SAFE_GL(glDeleteBuffers(1, &m_uniformBuffer));
    SAFE_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    logMsg("[display] shutdown OK");
  }

  void setFullscreen(bool fs) override
  {
    auto flags = fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(m_window, flags);
  }

  void setCaption(String caption) override
  {
    SDL_SetWindowTitle(m_window, caption.data);
  }

  void readPixels(Span<uint8_t> dstRgbPixels) override
  {
    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);
    SAFE_GL(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dstRgbPixels.data));

    // reverse upside down
    const auto rowSize = width * 4;
    std::vector<uint8_t> rowBuf(rowSize);

    for(int row = 0; row < height / 2; ++row)
    {
      const auto rowLo = row;
      const auto rowHi = height - 1 - row;
      auto pRowLo = dstRgbPixels.data + rowLo * rowSize;
      auto pRowHi = dstRgbPixels.data + rowHi * rowSize;
      memcpy(rowBuf.data(), pRowLo, rowSize);
      memcpy(pRowLo, pRowHi, rowSize);
      memcpy(pRowHi, rowBuf.data(), rowSize);
    }
  }

  void enableGrab(bool enable) override
  {
    SDL_SetRelativeMouseMode(enable ? SDL_TRUE : SDL_FALSE);
    SDL_SetWindowGrab(m_window, enable ? SDL_TRUE : SDL_FALSE);
    SDL_ShowCursor(enable ? 0 : 1);
  }

  std::unique_ptr<ITexture> createTexture() override
  {
    return std::make_unique<OpenGlTexture>();
  }

  std::unique_ptr<IGpuProgram> createGpuProgram(String name_, bool zTest) override
  {
    const std::string name(name_.data, name_.len);
    logMsg("[display] loading shader '%s'", name.c_str());
    auto vsCode = File::read("res/shaders/" + name + ".vert");
    auto fsCode = File::read("res/shaders/" + name + ".frag");

    auto toSpan = [] (const std::string& s)
      {
        return Span<const uint8_t>((const uint8_t*)s.c_str(), s.size());
      };

    return std::make_unique<OpenGlProgram>(loadShaders(toSpan(vsCode), toSpan(fsCode)), zTest);
  }

  void useGpuProgram(IGpuProgram* iprogram) override
  {
    auto program = dynamic_cast<OpenGlProgram*>(iprogram);
    SAFE_GL(glUseProgram(program->program));
    enableZTest(program->zTest);
    m_currProgram = program;
  }

  void useVertexBuffer(IVertexBuffer* ivb) override
  {
    auto vb = dynamic_cast<OpenGlVertexBuffer*>(ivb);
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, vb->vbo));
  }

  void enableVertexAttribute(int id, int dim, int stride, int offset) override
  {
    SAFE_GL(glEnableVertexAttribArray(id));
    SAFE_GL(glVertexAttribPointer(id, dim, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)offset));
  }

  void setUniformBlock(void* ptr, size_t size) override
  {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, size, ptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_currProgram->uniformBlockIndex, m_uniformBuffer);
  }

  std::unique_ptr<IVertexBuffer> createVertexBuffer() override
  {
    return std::make_unique<OpenGlVertexBuffer>();
  }

  std::unique_ptr<IFrameBuffer> createFrameBuffer(Vec2i resolution, bool depth) override
  {
    return std::make_unique<OpenGlFrameBuffer>(resolution, depth);
  }

  void setRenderTarget(IFrameBuffer* ifb) override
  {
    auto fb = dynamic_cast<OpenGlFrameBuffer*>(ifb);

    if(!fb)
    {
      SAFE_GL(glViewport(m_screenViewport.pos.x, m_screenViewport.pos.y, m_screenViewport.size.x, m_screenViewport.size.y));

      SAFE_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    else
    {
      SAFE_GL(glViewport(0, 0, fb->resolution.x, fb->resolution.y));
      SAFE_GL(glBindFramebuffer(GL_FRAMEBUFFER, fb->framebuffer));
    }
  }

  void enableZTest(bool enable)
  {
    if(enable)
      SAFE_GL(glEnable(GL_DEPTH_TEST));
    else
      SAFE_GL(glDisable(GL_DEPTH_TEST));
  }

  void draw(int vertexCount) override
  {
    SAFE_GL(glDrawArrays(GL_TRIANGLES, 0, vertexCount));
    ++m_drawCallCount;
  }

  void clear() override
  {
    SAFE_GL(glClearColor(0, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  }

  void swap() override
  {
    SDL_GL_SwapWindow(m_window);
    updateScreenSize();
    ggDrawCalls = m_drawCallCount;
    m_drawCallCount = 0;
  }

  void updateScreenSize()
  {
    Vec2i screenSize {};
    Rect2i screenViewport {};
    SDL_GL_GetDrawableSize(m_window, &screenSize.x, &screenSize.y);

    float cx, cy;

    if(AspectRatio > float(screenSize.x) / screenSize.y)
    {
      cx = screenSize.x;
      cy = cx / AspectRatio;
    }
    else
    {
      cy = screenSize.y;
      cx = cy * AspectRatio;
    }

    screenViewport.pos.x = (screenSize.x - cx) / 2;
    screenViewport.pos.y = (screenSize.y - cy) / 2;
    screenViewport.size.x = cx;
    screenViewport.size.y = cy;

    if(screenSize != m_screenSize || m_screenViewport.pos != m_screenViewport.pos || screenViewport.size != m_screenViewport.size)
    {
      m_screenSize = screenSize;
      m_screenViewport = screenViewport;

      if(m_screenSizeListener)
        m_screenSizeListener->onScreenSizeChanged(screenSize, screenViewport);
    }
  }

  void setScreenSizeListener(IScreenSizeListener* listener) override
  {
    m_screenSizeListener = listener;
    updateScreenSize();
    m_screenSizeListener->onScreenSizeChanged(m_screenSize, m_screenViewport);
  }

private:
  int m_drawCallCount = 0;
  Vec2i m_screenSize {};
  Rect2i m_screenViewport {};
  IScreenSizeListener* m_screenSizeListener {};
  SDL_Window* m_window;
  SDL_GLContext m_context;
  GLuint m_uniformBuffer {};
  const OpenGlProgram* m_currProgram;
};
}

IGraphicsBackend* createGraphicsBackend(Vec2i resolution)
{
  return new OpenGlGraphicsBackend(resolution);
}

