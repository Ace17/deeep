// Copyright (C) 2018 - Sebastien Alaiwan
// Copyright (c) 2005-2010 Lode Vandevenne
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// *This is a modified version of picoPNG*

#include "png.h"

#include "base/error.h"
#include "base/span.h"
#include "misc/decompress.h"
#include <climits>
#include <cstdint>
#include <vector>

namespace
{
struct Info
{
  unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
  bool key_defined = false; // is a transparent color key given?
  std::vector<uint8_t> palette;
};

void enforce(bool condition, String msg)
{
  if(!condition)
    throw Error(msg);
}

unsigned long read32bitInt(const uint8_t* buffer)
{
  return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

bool isColorValid(unsigned long colorType, unsigned long bd) // return type is a LodePNG error code
{
  if((colorType == 2 || colorType == 4 || colorType == 6))
    return bd == 8 || bd == 16;

  if(colorType == 0)
    return bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16;

  if(colorType == 3)
    return bd == 1 || bd == 2 || bd == 4 || bd == 8;

  return false; // unexisting color type
}

Info readPngHeader(Span<const uint8_t> in) // read the information from the header and store it in the Info
{
  enforce(in.len >= 29, "no PNG header");
  enforce(in[0] == 137 && in[1] == 80 && in[2] == 78 && in[3] == 71 && in[4] == 13 && in[5] == 10 && in[6] == 26 && in[7] == 10, "no PNG signature");
  enforce(in[12] == 'I' && in[13] == 'H' && in[14] == 'D' && in[15] == 'R', "no IDHR chunk");

  Info info;
  info.width = read32bitInt(&in[16]);
  info.height = read32bitInt(&in[20]);
  info.bitDepth = in[24];
  info.colorType = in[25];

  if(!isColorValid(info.colorType, info.bitDepth))
    throw Error("invalid combination of colorType and bitDepth");

  enforce(in[26] == 0, "unexpected compression method");
  info.compressionMethod = in[26];

  enforce(in[27] == 0, "unexpected filter method");
  info.filterMethod = in[27];

  enforce(in[28] <= 1, "unexpected interlace method");
  info.interlaceMethod = in[28];

  return info;
}

uint8_t paethPredictor(short a, short b, short c) // used by filter type 4
{
  short p = a + b - c, pa = p > a ? (p - a) : (a - p), pb = p > b ? (p - b) : (b - p), pc = p > c ? (p - c) : (c - p);
  return (uint8_t)((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
}

void unFilterScanline(uint8_t* recon, const uint8_t* scanline, const uint8_t* precon, size_t bytewidth, unsigned long filterType, size_t length)
{
  switch(filterType)
  {
  case 0:

    for(size_t i = 0; i < length; i++)
      recon[i] = scanline[i];

    break;
  case 1:

    for(size_t i = 0; i < bytewidth; i++)
      recon[i] = scanline[i];

    for(size_t i = bytewidth; i < length; i++)
      recon[i] = scanline[i] + recon[i - bytewidth];

    break;
  case 2:

    if(precon)
      for(size_t i = 0; i < length; i++)
        recon[i] = scanline[i] + precon[i];

    else
      for(size_t i = 0; i < length; i++)
        recon[i] = scanline[i];

    break;
  case 3:

    if(precon)
    {
      for(size_t i = 0; i < bytewidth; i++)
        recon[i] = scanline[i] + precon[i] / 2;

      for(size_t i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
    }
    else
    {
      for(size_t i = 0; i < bytewidth; i++)
        recon[i] = scanline[i];

      for(size_t i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + recon[i - bytewidth] / 2;
    }

    break;
  case 4:

    if(precon)
    {
      for(size_t i = 0; i < bytewidth; i++)
        recon[i] = scanline[i] + paethPredictor(0, precon[i], 0);

      for(size_t i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]);
    }
    else
    {
      for(size_t i = 0; i < bytewidth; i++)
        recon[i] = scanline[i];

      for(size_t i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0);
    }

    break;
  default:
    throw Error("unexisting filter type given");
  }
}

void setBitOfReversedStream(size_t& bitp, uint8_t* bits, unsigned long bit)
{
  bits[bitp >> 3] |= (bit << (7 - (bitp & 0x7)));
  bitp++;
}

unsigned long readBitFromReversedStream(size_t& bitp, const uint8_t* bits)
{
  unsigned long result = (bits[bitp >> 3] >> (7 - (bitp & 0x7))) & 1;
  bitp++;
  return result;
}

void adam7Pass(uint8_t* out, uint8_t* linen, uint8_t* lineo, const uint8_t* in, unsigned long w, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
{ // filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
  if(passw == 0)
    return;

  size_t bytewidth = (bpp + 7) / 8, linelength = 1 + ((bpp * passw + 7) / 8);

  for(unsigned long y = 0; y < passh; y++)
  {
    uint8_t filterType = in[y * linelength], * prevline = (y == 0) ? 0 : lineo;
    unFilterScanline(linen, &in[y * linelength + 1], prevline, bytewidth, filterType, (w * bpp + 7) / 8);

    if(bpp >= 8)
      for(size_t i = 0; i < passw; i++)
        for(size_t b = 0; b < bytewidth; b++) // b = current byte of this pixel
          out[bytewidth * w * (passtop + spacey * y) + bytewidth * (passleft + spacex * i) + b] = linen[bytewidth * i + b];

    else
      for(size_t i = 0; i < passw; i++)
      {
        size_t obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i), bp = i * bpp;

        for(size_t b = 0; b < bpp; b++)
          setBitOfReversedStream(obp, out, readBitFromReversedStream(bp, &linen[0]));
      }

    uint8_t* temp = linen;
    linen = lineo;
    lineo = temp; // swap the two buffer pointers "line old" and "line new"
  }
}

unsigned long getBpp(const Info& info)
{
  if(info.colorType == 2)
    return 3 * info.bitDepth;
  else if(info.colorType >= 4)
    return (info.colorType - 2) * info.bitDepth;
  else
    return info.bitDepth;
}

Info decode(std::vector<uint8_t>& out, Span<const uint8_t> in)
{
  enforce(in.len > 0 && in.data, "empty PNG data");

  auto info = readPngHeader(in);

  size_t pos = 33; // first byte of the first chunk after the header
  std::vector<uint8_t> idat; // the data from idat chunks
  bool IEND = false;

  while(!IEND) // loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
  {
    enforce(pos + 8 < (size_t)in.len, "truncated chunk header");

    size_t chunkLength = read32bitInt(&in[pos]);
    pos += 4;

    enforce(chunkLength <= INT_MAX, "chunk too big");
    enforce(pos + chunkLength < (size_t)in.len, "truncated chunk data");

    if(in[pos + 0] == 'I' && in[pos + 1] == 'D' && in[pos + 2] == 'A' && in[pos + 3] == 'T') // IDAT chunk, containing compressed image data
    {
      idat.insert(idat.end(), &in[pos + 4], &in[pos + 4 + chunkLength]);
      pos += (4 + chunkLength);
    }
    else if(in[pos + 0] == 'I' && in[pos + 1] == 'E' && in[pos + 2] == 'N' && in[pos + 3] == 'D')
    {
      pos += 4;
      IEND = true;
    }
    else if(in[pos + 0] == 'P' && in[pos + 1] == 'L' && in[pos + 2] == 'T' && in[pos + 3] == 'E') // palette chunk (PLTE)
    {
      pos += 4; // go after the 4 letters
      info.palette.resize(4 * (chunkLength / 3));

      enforce(info.palette.size() <= (4 * 256), "palette too big");

      for(size_t i = 0; i < info.palette.size(); i += 4)
      {
        for(size_t j = 0; j < 3; j++)
          info.palette[i + j] = in[pos++]; // RGB

        info.palette[i + 3] = 255; // alpha
      }
    }
    else if(in[pos + 0] == 't' && in[pos + 1] == 'R' && in[pos + 2] == 'N' && in[pos + 3] == 'S') // palette transparency chunk (tRNS)
    {
      pos += 4; // go after the 4 letters

      if(info.colorType == 3)
      {
        enforce(4 * chunkLength <= info.palette.size(), "truncated palette entries: alpha");

        for(size_t i = 0; i < chunkLength; i++)
          info.palette[4 * i + 3] = in[pos++];
      }
      else if(info.colorType == 0)
      {
        enforce(chunkLength == 2, "this chunk must be 2 bytes for greyscale image");

        info.key_defined = 1;
        info.key_r = info.key_g = info.key_b = 256 * in[pos] + in[pos + 1];
        pos += 2;
      }
      else if(info.colorType == 2)
      {
        enforce(chunkLength == 6, "this chunk must be 6 bytes for RGB image");

        info.key_defined = 1;
        info.key_r = 256 * in[pos] + in[pos + 1];
        pos += 2;
        info.key_g = 256 * in[pos] + in[pos + 1];
        pos += 2;
        info.key_b = 256 * in[pos] + in[pos + 1];
        pos += 2;
      }
      else
      {
        throw Error("tRNS chunk not allowed for other color models");
      }
    }
    else // it's not an implemented chunk type, so ignore it: skip over the data
    {
      enforce(in[pos + 0] & 32, "unknown critical chunk (5th bit of first byte of chunk type is 0");
      pos += (chunkLength + 4); // skip 4 letters and uninterpreted data of unimplemented chunk
    }

    pos += 4; // step over CRC (which is ignored)
  }

  unsigned long bpp = getBpp(info);
  auto scanlines = ::zlibDecompress(Span<const uint8_t> { idat.data(), (int)idat.size() });

  size_t bytewidth = (bpp + 7) / 8, outlength = (info.height * info.width * bpp + 7) / 8;
  out.resize(outlength); // time to fill the out buffer
  uint8_t* out_ = outlength ? &out[0] : 0; // use a regular pointer to the std::vector for faster code if compiled without optimization

  if(info.interlaceMethod == 0) // no interlace, just filter
  {
    size_t linestart = 0, linelength = (info.width * bpp + 7) / 8; // length in bytes of a scanline, excluding the filtertype byte

    if(bpp >= 8) // byte per byte
      for(unsigned long y = 0; y < info.height; y++)
      {
        unsigned long filterType = scanlines[linestart];
        const uint8_t* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
        unFilterScanline(&out_[linestart - y], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength);

        linestart += (1 + linelength); // go to start of next scanline
      }

    else // less than 8 bits per pixel, so fill it up bit per bit
    {
      std::vector<uint8_t> templine((info.width * bpp + 7) >> 3); // only used if bpp < 8

      for(size_t y = 0, obp = 0; y < info.height; y++)
      {
        unsigned long filterType = scanlines[linestart];
        const uint8_t* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
        unFilterScanline(&templine[0], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength);

        for(size_t bp = 0; bp < info.width * bpp;)
          setBitOfReversedStream(obp, out_, readBitFromReversedStream(bp, &templine[0]));

        linestart += (1 + linelength); // go to start of next scanline
      }
    }
  }
  else // interlaceMethod is 1 (Adam7)
  {
    size_t passw[7] = { (info.width + 7) / 8, (info.width + 3) / 8, (info.width + 3) / 4, (info.width + 1) / 4, (info.width + 1) / 2, (info.width + 0) / 2, (info.width + 0) / 1 };
    size_t passh[7] = { (info.height + 7) / 8, (info.height + 7) / 8, (info.height + 3) / 8, (info.height + 3) / 4, (info.height + 1) / 4, (info.height + 1) / 2, (info.height + 0) / 2 };
    size_t passstart[7] = { 0 };
    size_t pattern[28] = { 0, 4, 0, 2, 0, 1, 0, 0, 0, 4, 0, 2, 0, 1, 8, 8, 4, 4, 2, 2, 1, 8, 8, 8, 4, 4, 2, 2 }; // values for the adam7 passes

    for(int i = 0; i < 6; i++)
      passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);

    std::vector<uint8_t> scanlineo((info.width * bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); // "old" and "new" scanline

    for(int i = 0; i < 7; i++)
      adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
  }

  enforce(info.colorType == 6 && info.bitDepth == 8, "unsupported colorType/bitdepth");

  return info;
}
}

std::vector<uint8_t> decodePng(Span<const uint8_t> pngData, int& width, int& height)
{
  std::vector<uint8_t> r;

  auto info = decode(r, pngData);
  width = info.width;
  height = info.height;

  return r;
}

