// This is a modified version of NanoJPEG -- KeyJ's Tiny Baseline JPEG Decoder
// Copyright (C) 2024 - Sebastien Alaiwan
// Copyright (c) 2009-2016 Martin J. Fiedler <martin.fiedler@gmx.net>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/error.h"
#include "jpg.h"
#include "picture.h"

#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>

namespace
{
thread_local char msgBuf[256];

int Clip255(int x)
{
  if(x < 0)
    return 0;

  if(x > 255)
    return 255;

  return x;
}

static constexpr int W1 = 2841;
static constexpr int W2 = 2676;
static constexpr int W3 = 2408;
static constexpr int W5 = 1609;
static constexpr int W6 = 1108;
static constexpr int W7 = 565;

void rowIDCT(int* blk)
{
  int x0 = blk[0] * (1 << 11) + 128;
  int x1 = blk[4] * (1 << 11);
  int x2 = blk[6];
  int x3 = blk[2];
  int x4 = blk[1];
  int x5 = blk[7];
  int x6 = blk[5];
  int x7 = blk[3];
  int x8 = W7 * (x4 + x5);

  x4 = x8 + (W1 - W7) * x4;
  x5 = x8 - (W1 + W7) * x5;
  x8 = W3 * (x6 + x7);
  x6 = x8 - (W3 - W5) * x6;
  x7 = x8 - (W3 + W5) * x7;
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2);
  x2 = x1 - (W2 + W6) * x2;
  x3 = x1 + (W2 - W6) * x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;

  blk[0] = (x7 + x1) >> 8;
  blk[1] = (x3 + x2) >> 8;
  blk[2] = (x0 + x4) >> 8;
  blk[3] = (x8 + x6) >> 8;
  blk[4] = (x8 - x6) >> 8;
  blk[5] = (x0 - x4) >> 8;
  blk[6] = (x3 - x2) >> 8;
  blk[7] = (x7 - x1) >> 8;
}

void colIDCT(const int* blk, uint8_t* out, int stride)
{
  int x0 = blk[0] * 256 + 8192;
  int x1 = blk[8 * 4] * 256;
  int x2 = blk[8 * 6];
  int x3 = blk[8 * 2];
  int x4 = blk[8 * 1];
  int x5 = blk[8 * 7];
  int x6 = blk[8 * 5];
  int x7 = blk[8 * 3];
  int x8 = W7 * (x4 + x5) + 4;

  x4 = (x8 + (W1 - W7) * x4) >> 3;
  x5 = (x8 - (W1 + W7) * x5) >> 3;
  x8 = W3 * (x6 + x7) + 4;
  x6 = (x8 - (W3 - W5) * x6) >> 3;
  x7 = (x8 - (W3 + W5) * x7) >> 3;
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2) + 4;
  x2 = (x1 - (W2 + W6) * x2) >> 3;
  x3 = (x1 + (W2 - W6) * x3) >> 3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;

  out[stride * 0] = Clip255(((x7 + x1) >> 14) + 128);
  out[stride * 1] = Clip255(((x3 + x2) >> 14) + 128);
  out[stride * 2] = Clip255(((x0 + x4) >> 14) + 128);
  out[stride * 3] = Clip255(((x8 + x6) >> 14) + 128);
  out[stride * 4] = Clip255(((x8 - x6) >> 14) + 128);
  out[stride * 5] = Clip255(((x0 - x4) >> 14) + 128);
  out[stride * 6] = Clip255(((x3 - x2) >> 14) + 128);
  out[stride * 7] = Clip255(((x7 - x1) >> 14) + 128);
}

void resizePicture(PictureView src, PictureView dst)
{
  for(int row = 0; row < dst.dim.y; ++row)
  {
    for(int col = 0; col < dst.dim.x; ++col)
    {
      int srcCol = col * src.dim.x / dst.dim.x;
      int srcRow = row * src.dim.y / dst.dim.y;
      dst.pixels[col + row * dst.stride] = src.pixels[srcCol + srcRow * src.stride];
    }
  }
}

class Decoder
{
public:
  Decoder(Span<const uint8_t> jpeg)
  {
    decodeToYUV(jpeg);
    upsample();

    if(m_planeCount == 3)
      convertToRGB();
  }

  void decodeToYUV(Span<const uint8_t> jpeg)
  {
    input = jpeg;

    const int magic = parseBigEndian16();

    if(magic != 0xFFD8)
      throw Error("invalid JPG signature");

    bool done = false;

    while(!done)
    {
      const int type = parseBigEndian16();
      const int size = parseBigEndian16();
      m_sectionLength = size - 2;

      if(m_sectionLength > input.len)
        throw Error("truncated file");
      switch(type)
      {
      case 0xFFC0: decodePictureHeader();
        break;
      case 0xFFC4: decodeHuffmanTable();
        break;
      case 0xFFDB: decodeQuantizationTable();
        break;
      case 0xFFDD: decodeResetInterval();
        break;
      case 0xFFDA: decodeBlocks();
        done = true;
        break;
      case 0xFFFE:
        skipBytes(m_sectionLength);
        break;
      default:

        if((type & 0xF0) != 0xE0)
          throw Error(format(msgBuf, "jpeg: Unsupported marker type 0x%.2X", type));

        skipBytes(m_sectionLength);
      }
    }
  }

  struct HuffmanCode
  {
    uint8_t bits, code;
  };

  struct ColorPlane
  {
    int planeId;
    int ssx, ssy;
    int width, height;
    int stride;
    int qTab;
    int acTab, dcTab;
    int dcPredictor;
    std::vector<uint8_t> pixels;
  };

  struct Context
  {
    uint32_t buf;
    int bufbits;
  };

  int m_sectionLength = 0;
  int m_width, m_height;
  int m_planeCount;
  ColorPlane m_planes[3] {};
  uint8_t m_quantizers[4][64] {};
  HuffmanCode m_hufTable[4][65536] {};
  std::vector<uint8_t> m_pixels;
  int m_resetInterval = 0;
  int m_widthInMbs, m_heightInMbs;

  Context ctx {};
  Span<const uint8_t> input;

  int peekBits(int bits)
  {
    while(ctx.bufbits < bits)
      fetchOneByte();

    return (ctx.buf >> (ctx.bufbits - bits)) & ((1 << bits) - 1);
  }

  void fetchOneByte()
  {
    if(input.len <= 0)
    {
      ctx.buf = (ctx.buf << 8) | 0xFF;
      ctx.bufbits += 8;
      return;
    }

    uint8_t newbyte = input[0];
    input += 1;
    ctx.bufbits += 8;
    ctx.buf = (ctx.buf << 8) | newbyte;

    // 0xFF is coded FF 00. Decoders should skip the 00.
    if(newbyte == 0xFF)
    {
      if(input.len == 0)
        throw Error("truncated file");

      uint8_t marker = input[0];
      input += 1;
      switch(marker)
      {
      case 0:    break;
      case 0xD9: input.len = 0;
        break;
      default:

        if((marker & 0xF8) != 0xD0)
          throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

        ctx.buf = (ctx.buf << 8) | marker;
        ctx.bufbits += 8;
      }
    }
  }

  int readBits(int bits)
  {
    int r = peekBits(bits);
    ctx.bufbits -= bits;
    return r;
  }

  void skipBytes(int count)
  {
    if(count > input.len)
      throw Error("Read buffer overrun");

    input += count;
    m_sectionLength -= count;
  }

  int parseByte()
  {
    if(input.len < 1)
      throw Error("truncated file");

    const int r = input[0];
    skipBytes(1);
    return r;
  }

  int parseBigEndian16()
  {
    const auto b0 = parseByte() << 8;
    const auto b1 = parseByte();
    return b0 | b1;
  }

  void decodePictureHeader()
  {
    const int bitdepth = parseByte();

    if(bitdepth != 8)
      throw Error("Only 8-bit is supported");

    m_height = parseBigEndian16();
    m_width = parseBigEndian16();
    m_planeCount = parseByte();

    if(m_planeCount != 3)
      throw Error("Only RGB is supported");

    int maxSsx = 0;
    int maxSsy = 0;

    for(int i = 0; i < m_planeCount; ++i)
    {
      auto c = &m_planes[i];
      c->planeId = parseByte();

      const auto subsampling = parseByte();
      c->ssx = (subsampling >> 4) & 0b1111;
      c->ssy = (subsampling >> 0) & 0b1111;

      if(c->ssx == 0 || c->ssy == 0)
        throw Error(format(msgBuf, "invalid subsampling: %dx%d", c->ssx, c->ssy));

      if(c->ssx & (c->ssx - 1) || c->ssy & (c->ssy - 1))
        throw Error(format(msgBuf, "unsupported subsampling: %dx%d", c->ssx, c->ssy));

      c->qTab = parseByte();

      if(c->qTab >= 4)
        throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

      if(c->ssx > maxSsx)
        maxSsx = c->ssx;

      if(c->ssy > maxSsy)
        maxSsy = c->ssy;
    }

    const int mbSizeX = maxSsx * 8;
    const int mbSizeY = maxSsy * 8;
    m_widthInMbs = (m_width + mbSizeX - 1) / mbSizeX;
    m_heightInMbs = (m_height + mbSizeY - 1) / mbSizeY;

    for(int i = 0; i < m_planeCount; ++i)
    {
      auto c = &m_planes[i];

      c->width = (m_width * c->ssx + maxSsx - 1) / maxSsx;
      c->height = (m_height * c->ssy + maxSsy - 1) / maxSsy;
      c->stride = m_widthInMbs * mbSizeX * c->ssx / maxSsx;

      if(((c->width < 3) && (c->ssx != maxSsx)) || ((c->height < 3) && (c->ssy != maxSsy)))
        throw Error("Picture is too small");

      c->pixels.resize(c->stride * (m_heightInMbs * mbSizeY * c->ssy / maxSsy));
    }

    skipBytes(m_sectionLength);
  }

  void decodeHuffmanTable()
  {
    while(m_sectionLength > 0)
    {
      int q = parseByte();

      if(q & 0xEC)
        throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

      if(q & 0x02)
        throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

      q = (q | (q >> 3)) & 3;  // combined DC/AC + tableid value

      int counts[16];

      for(int i = 0; i < 16; ++i)
        counts[i] = parseByte();

      HuffmanCode* vlc = m_hufTable[q];
      int spread = 65536;

      for(int codeLen = 0; codeLen < 16; ++codeLen)
      {
        spread /= 2;
        const int count = counts[codeLen];

        for(int i = 0; i < count; ++i)
        {
          const int code = parseByte();

          for(int k = 0; k < spread; ++k)
          {
            vlc->bits = codeLen + 1;
            vlc->code = code;
            ++vlc;
          }
        }
      }
    }
  }

  void decodeQuantizationTable()
  {
    while(m_sectionLength > 0)
    {
      const int qTab = parseByte();

      if(qTab >= 4)
        throw Error(format(msgBuf, "decoding error, qTab must be in [0;3], got %d", qTab));

      for(int k = 0; k < 64; ++k)
        m_quantizers[qTab][k] = parseByte();
    }
  }

  void decodeResetInterval()
  {
    m_resetInterval = parseBigEndian16();
    skipBytes(m_sectionLength);
  }

  void decodeBlocks()
  {
    const int planeCount = parseByte();

    if(planeCount != m_planeCount)
      throw Error(format(msgBuf, "invalid plane count: %d instead of %d", planeCount, m_planeCount));

    for(int i = 0; i < m_planeCount; ++i)
    {
      const int planeId = parseByte();

      if(planeId != m_planes[i].planeId)
        throw Error(format(msgBuf, "invalid plane id: %d instead of %d", planeId, m_planes[i].planeId));

      const int tab = parseByte();

      if(tab & 0xEE)
        throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

      m_planes[i].dcTab = tab >> 4;
      m_planes[i].acTab = (tab & 1) | 2;
    }

    if(input[0] || input[1] != 63 || input[2])
      throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

    skipBytes(m_sectionLength);

    int resetCounter = m_resetInterval;
    int nextReset = 0;

    for(int row = 0; row < m_heightInMbs; ++row)
    {
      for(int col = 0; col < m_widthInMbs; ++col)
      {
        for(int i = 0; i < m_planeCount; ++i)
        {
          ColorPlane& plane = m_planes[i];

          for(int sby = 0; sby < plane.ssy; ++sby)
          {
            for(int sbx = 0; sbx < plane.ssx; ++sbx)
            {
              const int pos = (row * plane.ssy + sby) * plane.stride + col * plane.ssx + sbx;
              decodeBlock(&plane, &plane.pixels[pos * 8]);
            }
          }
        }

        if(m_resetInterval && !(--resetCounter))
        {
          ctx.bufbits &= 0xF8; // byte-align
          int i = readBits(16);

          if(((i & 0xFFF8) != 0xFFD0) || ((i & 7) != nextReset))
            throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

          nextReset = (nextReset + 1) & 7;
          resetCounter = m_resetInterval;

          for(i = 0; i < 3; ++i)
            m_planes[i].dcPredictor = 0;
        }
      }
    }
  }

  void decodeBlock(ColorPlane* c, uint8_t* out)
  {
    int block[64] {};

    uint8_t unused;
    c->dcPredictor += readVlc(m_hufTable[c->dcTab], unused);
    block[0] = c->dcPredictor * m_quantizers[c->qTab][0];

    int coef = 0;

    while(coef < 63)
    {
      uint8_t code;
      const int value = readVlc(m_hufTable[c->acTab], code);

      if(!code)
        break; // EOB

      if(!(code & 0x0F) && (code != 0xF0))
        throw Error(format(msgBuf, "decoding error (%s:%d)", __FILE__, __LINE__));

      coef += (code >> 4) + 1;

      if(coef > 63)
        throw Error("decoding error, too many coeffs");

      static constexpr char zScan[64] = {
        0, 1, 8, 16, 9, 2, 3, 10, //
        17, 24, 32, 25, 18, 11, 4, 5, //
        12, 19, 26, 33, 40, 48, 41, 34, //
        27, 20, 13, 6, 7, 14, 21, 28, //
        35, 42, 49, 56, 57, 50, 43, 36, //
        29, 22, 15, 23, 30, 37, 44, 51, //
        58, 59, 52, 45, 38, 31, 39, 46, //
        53, 60, 61, 54, 47, 55, 62, 63, //
      };

      block[(int)zScan[coef]] = value * m_quantizers[c->qTab][coef];
    }

    for(int i = 0; i < 8; ++i)
      rowIDCT(&block[i * 8]);

    for(int i = 0; i < 8; ++i)
      colIDCT(&block[i], &out[i], c->stride);
  }

  int readVlc(HuffmanCode* vlc, uint8_t& outCode)
  {
    const int prefix = peekBits(16);
    const int size = vlc[prefix].bits;

    if(!size)
      throw Error(format(msgBuf, "Invalid VLC code: 0x%.X", prefix));

    readBits(size);
    const int code = vlc[prefix].code;

    outCode = (uint8_t)code;

    const int bits = code & 15;

    if(!bits)
      return 0;

    int value = readBits(bits);

    if(value < (1 << (bits - 1)))
      value -= (1 << bits) - 1;

    return value;
  }

  void upsample()
  {
    for(int i = 0; i < m_planeCount; ++i)
    {
      ColorPlane& c = m_planes[i];

      if(c.width != m_width || c.height != m_height)
      {
        std::vector<uint8_t> intermediateBuf(m_height * m_width);

        PictureView srcPic { { c.width, c.height }, c.stride, c.pixels.data() };
        PictureView dstPic { { m_width, m_height }, m_width, intermediateBuf.data() };
        resizePicture(srcPic, dstPic);
        c.pixels = std::move(intermediateBuf);
        c.width = m_width;
        c.height = m_height;
        c.stride = m_width;
      }
    }
  }

  void convertToRGB()
  {
    m_pixels.resize(m_width * m_height * 4);

    uint8_t* pRGB = m_pixels.data();
    const uint8_t* pY = m_planes[0].pixels.data();
    const uint8_t* pCb = m_planes[1].pixels.data();
    const uint8_t* pCr = m_planes[2].pixels.data();

    for(int y = 0; y < m_height; ++y)
    {
      for(int x = 0; x < m_width; ++x)
      {
        const int luma = pY[x] << 8;
        const int cb = pCb[x] - 128;
        const int cr = pCr[x] - 128;
        pRGB[x * 4 + 0] = Clip255((luma + 359 * cr + 128) >> 8);
        pRGB[x * 4 + 1] = Clip255((luma - 88 * cb - 183 * cr + 128) >> 8);
        pRGB[x * 4 + 2] = Clip255((luma + 454 * cb + 128) >> 8);
        pRGB[x * 4 + 3] = 0xFF;
      }

      pRGB += m_width * 4;
      pY += m_planes[0].stride;
      pCb += m_planes[1].stride;
      pCr += m_planes[2].stride;
    }
  }
};
}

std::vector<uint8_t> decodeJpg(Span<const uint8_t> jpgData, int& width, int& height)
{
  auto d = std::make_unique<Decoder>(jpgData);

  width = d->m_width;
  height = d->m_height;

  return std::move(d->m_pixels);
}

