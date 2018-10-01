// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
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

#include <climits>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include "base/span.h"
#include "decompress.h"

namespace
{
struct PNG
{
  struct Info
  {
    unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
    bool key_defined = false; // is a transparent color key given?
    std::vector<uint8_t> palette;
  };
  static
  Info decode(std::vector<uint8_t>& out, Span<const uint8_t> in)
  {
    if(in.len == 0 || in.data == nullptr)
      throw std::runtime_error("empty PNG data");

    auto info = readPngHeader(in);

    size_t pos = 33; // first byte of the first chunk after the header
    std::vector<uint8_t> idat; // the data from idat chunks
    bool IEND = false;

    while(!IEND) // loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
    {
      if(pos + 8 >= (size_t)in.len)
        throw std::runtime_error("truncated chunk header");

      size_t chunkLength = read32bitInt(&in[pos]);
      pos += 4;

      if(chunkLength > INT_MAX)
        throw std::runtime_error("chunk too big");

      if(pos + chunkLength >= (size_t)in.len)
        throw std::runtime_error("truncated chunk data");

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

        if(info.palette.size() > (4 * 256))
          throw std::runtime_error("palette too big");

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
          if(4 * chunkLength > info.palette.size())
            throw std::runtime_error("truncated palette entries: alpha");

          for(size_t i = 0; i < chunkLength; i++)
            info.palette[4 * i + 3] = in[pos++];
        }
        else if(info.colorType == 0)
        {
          if(chunkLength != 2)
            throw std::runtime_error("this chunk must be 2 bytes for greyscale image");

          info.key_defined = 1;
          info.key_r = info.key_g = info.key_b = 256 * in[pos] + in[pos + 1];
          pos += 2;
        }
        else if(info.colorType == 2)
        {
          if(chunkLength != 6)
            throw std::runtime_error("this chunk must be 6 bytes for RGB image");

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
          throw std::runtime_error("tRNS chunk not allowed for other color models");
        }
      }
      else // it's not an implemented chunk type, so ignore it: skip over the data
      {
        if(!(in[pos + 0] & 32))
        {
          throw std::runtime_error("unknown critical chunk (5th bit of first byte of chunk type is 0");
        }

        pos += (chunkLength + 4); // skip 4 letters and uninterpreted data of unimplemented chunk
      }

      pos += 4; // step over CRC (which is ignored)
    }

    unsigned long bpp = getBpp(info);
    std::vector<uint8_t> scanlines(((info.width * (info.height* bpp + 7)) / 8) + info.height); // now the out buffer will be filled
    scanlines = ::decompress(Span<const uint8_t> { idat.data(), (int)idat.size() });

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
        std::vector<uint8_t> templine((info.width* bpp + 7) >> 3); // only used if bpp < 8

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

      std::vector<uint8_t> scanlineo((info.width* bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); // "old" and "new" scanline

      for(int i = 0; i < 7; i++)
        adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
    }

    if(info.colorType != 6 || info.bitDepth != 8) // conversion needed
    {
      std::vector<uint8_t> data = out;
      convert(out, &data[0], info, info.width, info.height);
    }

    return info;
  }

  static
  Info readPngHeader(Span<const uint8_t> in) // read the information from the header and store it in the Info
  {
    Info info;

    if(in.len < 29)
      throw std::runtime_error("no PNG header");

    if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
      throw std::runtime_error("no PNG signature");

    if(in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R')
      throw std::runtime_error("no IDHR chunk");

    info.width = read32bitInt(&in[16]);
    info.height = read32bitInt(&in[20]);
    info.bitDepth = in[24];
    info.colorType = in[25];

    if(checkColorValidity(info.colorType, info.bitDepth))
      throw std::runtime_error("invalid combination of colorType and bitDepth");

    info.compressionMethod = in[26];

    if(in[26] != 0)
      throw std::runtime_error("unexpected compression method");

    info.filterMethod = in[27];

    if(in[27] != 0)
      throw std::runtime_error("unexpected filter method");

    info.interlaceMethod = in[28];

    if(in[28] > 1)
      throw std::runtime_error("unexpected interlace method");

    return info;
  }

  static
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

  static
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
      throw std::runtime_error("unexisting filter type given");
    }
  }

  static
  unsigned long readBitFromReversedStream(size_t& bitp, const uint8_t* bits)
  {
    unsigned long result = (bits[bitp >> 3] >> (7 - (bitp & 0x7))) & 1;
    bitp++;
    return result;
  }

  static
  unsigned long readBitsFromReversedStream(size_t& bitp, const uint8_t* bits, unsigned long nbits)
  {
    unsigned long result = 0;

    for(size_t i = nbits - 1; i < nbits; i--)
      result += ((readBitFromReversedStream(bitp, bits)) << i);

    return result;
  }

  static
  void setBitOfReversedStream(size_t& bitp, uint8_t* bits, unsigned long bit)
  {
    bits[bitp >> 3] |= (bit << (7 - (bitp & 0x7)));
    bitp++;
  }

  static
  unsigned long read32bitInt(const uint8_t* buffer)
  {
    return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
  }

  static
  int checkColorValidity(unsigned long colorType, unsigned long bd) // return type is a LodePNG error code
  {
    if((colorType == 2 || colorType == 4 || colorType == 6))
    {
      if(!(bd == 8 || bd == 16))
        return 37;
      else
        return 0;
    }
    else if(colorType == 0)
    {
      if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16))
        return 37;
      else
        return 0;
    }
    else if(colorType == 3)
    {
      if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8))
        return 37;
      else
        return 0;
    }
    else
      return 31; // unexisting color type
  }

  static
  unsigned long getBpp(const Info& info)
  {
    if(info.colorType == 2)
      return 3 * info.bitDepth;
    else if(info.colorType >= 4)
      return (info.colorType - 2) * info.bitDepth;
    else
      return info.bitDepth;
  }

  static
  void convert(std::vector<uint8_t>& out, const uint8_t* in, Info& infoIn, unsigned long w, unsigned long h)
  {
    // converts from any color type to 32-bit. return value = LodePNG error code
    size_t numpixels = w * h, bp = 0;
    out.resize(numpixels * 4);
    uint8_t* out_ = out.empty() ? 0 : &out[0]; // faster if compiled without optimization

    if(infoIn.bitDepth == 8 && infoIn.colorType == 0) // greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[i];
        out_[4 * i + 3] = (infoIn.key_defined && in[i] == infoIn.key_r) ? 0 : 255;
      }

    else if(infoIn.bitDepth == 8 && infoIn.colorType == 2) // RGB color
      for(size_t i = 0; i < numpixels; i++)
      {
        for(size_t c = 0; c < 3; c++)
          out_[4 * i + c] = in[3 * i + c];

        out_[4 * i + 3] = (infoIn.key_defined == 1 && in[3 * i + 0] == infoIn.key_r && in[3 * i + 1] == infoIn.key_g && in[3 * i + 2] == infoIn.key_b) ? 0 : 255;
      }

    else if(infoIn.bitDepth == 8 && infoIn.colorType == 3) // indexed color (palette)
      for(size_t i = 0; i < numpixels; i++)
      {
        if(4U * in[i] >= infoIn.palette.size())
          throw std::runtime_error("conversion error");

        for(size_t c = 0; c < 4; c++)
          out_[4 * i + c] = infoIn.palette[4 * in[i] + c]; // get rgb colors from the palette
      }

    else if(infoIn.bitDepth == 8 && infoIn.colorType == 4) // greyscale with alpha
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i + 0];
        out_[4 * i + 3] = in[2 * i + 1];
      }

    else if(infoIn.bitDepth == 8 && infoIn.colorType == 6)
      for(size_t i = 0; i < numpixels; i++)
        for(size_t c = 0; c < 4; c++)
          out_[4 * i + c] = in[4 * i + c];

    // RGB with alpha
    else if(infoIn.bitDepth == 16 && infoIn.colorType == 0) // greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i];
        out_[4 * i + 3] = (infoIn.key_defined && 256U * in[i] + in[i + 1] == infoIn.key_r) ? 0 : 255;
      }

    else if(infoIn.bitDepth == 16 && infoIn.colorType == 2) // RGB color
      for(size_t i = 0; i < numpixels; i++)
      {
        for(size_t c = 0; c < 3; c++)
          out_[4 * i + c] = in[6 * i + 2 * c];

        out_[4 * i + 3] = (infoIn.key_defined && 256U * in[6 * i + 0] + in[6 * i + 1] == infoIn.key_r && 256U * in[6 * i + 2] + in[6 * i + 3] == infoIn.key_g && 256U * in[6 * i + 4] + in[6 * i + 5] == infoIn.key_b) ? 0 : 255;
      }

    else if(infoIn.bitDepth == 16 && infoIn.colorType == 4) // greyscale with alpha
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[4 * i]; // most significant byte
        out_[4 * i + 3] = in[4 * i + 2];
      }

    else if(infoIn.bitDepth == 16 && infoIn.colorType == 6)
      for(size_t i = 0; i < numpixels; i++)
        for(size_t c = 0; c < 4; c++)
          out_[4 * i + c] = in[8 * i + 2 * c];

    // RGB with alpha
    else if(infoIn.bitDepth < 8 && infoIn.colorType == 0) // greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        unsigned long value = (readBitsFromReversedStream(bp, in, infoIn.bitDepth) * 255) / ((1 << infoIn.bitDepth) - 1); // scale value from 0 to 255
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = (uint8_t)(value);
        out_[4 * i + 3] = (infoIn.key_defined && value && ((1U << infoIn.bitDepth) - 1U) == infoIn.key_r && ((1U << infoIn.bitDepth) - 1U)) ? 0 : 255;
      }

    else if(infoIn.bitDepth < 8 && infoIn.colorType == 3) // palette
      for(size_t i = 0; i < numpixels; i++)
      {
        unsigned long value = readBitsFromReversedStream(bp, in, infoIn.bitDepth);

        if(4 * value >= infoIn.palette.size())
          throw std::runtime_error("conversion error");

        for(int c = 0; c < 4; c++)
          out_[4 * i + c] = infoIn.palette[4 * value + c]; // get rgb colors from the palette
      }
  }

  static
  uint8_t paethPredictor(short a, short b, short c) // Paeth predicter, used by PNG filter type 4
  {
    short p = a + b - c, pa = p > a ? (p - a) : (a - p), pb = p > b ? (p - b) : (b - p), pc = p > c ? (p - c) : (c - p);
    return (uint8_t)((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
  }
};
}

std::vector<uint8_t> decodePng(Span<const uint8_t> pngData, int& width, int& height)
{
  std::vector<uint8_t> r;

  auto info = PNG::decode(r, pngData);
  width = info.width;
  height = info.height;

  return r;
}

