// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// interface to a low-level graphics API

#pragma once

#include <memory>

#include "base/box.h"
#include "base/geom.h"
#include "base/string.h"

struct PictureView;

struct IScreenSizeListener
{
  virtual ~IScreenSizeListener() = default;
  virtual void onScreenSizeChanged(Vec2i size, Rect2i viewport) = 0;
};

struct IGpuProgram
{
  virtual ~IGpuProgram() = default;
};

struct ITexture
{
  virtual ~ITexture() = default;
  virtual void create(Vec2i size) = 0;
  virtual void upload(PictureView srcPic, Vec2i dstPos = {}) = 0;
  virtual void setNoRepeat() = 0;
  virtual void bind(int unit) = 0;
};

struct IVertexBuffer
{
  virtual ~IVertexBuffer() = default;
  virtual void upload(const void* data, size_t len) = 0;
};

struct IFrameBuffer
{
  virtual ~IFrameBuffer() = default;
  virtual ITexture* getColorTexture() = 0;
};

struct IGraphicsBackend
{
  virtual ~IGraphicsBackend() = default;

  virtual void setFullscreen(bool fs) = 0;
  virtual void setCaption(String caption) = 0;
  virtual void enableGrab(bool enable) = 0;

  virtual void readPixels(Span<uint8_t> dstRgbPixels) = 0;

  virtual std::unique_ptr<ITexture> createTexture() = 0;
  virtual std::unique_ptr<IVertexBuffer> createVertexBuffer() = 0;
  virtual std::unique_ptr<IFrameBuffer> createFrameBuffer(Vec2i resolution, bool depth = true) = 0;
  virtual std::unique_ptr<IGpuProgram> createGpuProgram(String name, bool zTest) = 0;

  virtual void setScreenSizeListener(IScreenSizeListener* listener) = 0;

  // draw functions
  virtual void setRenderTarget(IFrameBuffer* fb) = 0;
  virtual void useGpuProgram(IGpuProgram* program) = 0;
  virtual void useVertexBuffer(IVertexBuffer* vb) = 0;
  virtual void enableVertexAttribute(int id, int dim, int stride, int offset) = 0;
  virtual void setUniformBlock(void* ptr, size_t size) = 0;
  virtual void draw(int vertexCount) = 0;
  virtual void clear() = 0;
  virtual void swap() = 0;
};

