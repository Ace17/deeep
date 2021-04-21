// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/error.h"
#include "decompress.h"
#include <vector>

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
namespace
{
const unsigned long LENBASE[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
const unsigned long LENEXTRA[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
const unsigned long DISTBASE[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
const unsigned long DISTEXTRA[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
const unsigned long CLCL[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 }; // code length code lengths

int readBit(size_t& bitp, const uint8_t* bits)
{
  int r = (bits[bitp / 8] >> (bitp % 8)) & 1;
  bitp++;
  return r;
}

int readBits(size_t& bitp, const uint8_t* bits, int nbits)
{
  int r = 0;

  for(int i = 0; i < nbits; i++)
    r += (readBit(bitp, bits)) << i;

  return r;
}

struct HuffmanTree
{
  int makeFromLengths(const std::vector<unsigned long>& bitlen, unsigned long maxbitlen)
  {
    // make tree given the lengths
    unsigned long numcodes = (unsigned long)(bitlen.size()), treepos = 0, nodefilled = 0;
    std::vector<unsigned long> tree1d(numcodes), blcount(maxbitlen + 1, 0), nextcode(maxbitlen + 1, 0);

    for(unsigned long bits = 0; bits < numcodes; bits++)
      blcount[bitlen[bits]]++; // count number of instances of each code length

    for(unsigned long bits = 1; bits <= maxbitlen; bits++)
      nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;

    for(unsigned long n = 0; n < numcodes; n++)
      if(bitlen[n] != 0)
        tree1d[n] = nextcode[bitlen[n]]++;

    // generate all the codes

    tree2d.clear();
    tree2d.resize(numcodes * 2, 32767); // 32767 here means the tree2d isn't filled there yet

    for(unsigned long n = 0; n < numcodes; n++) // the codes
      for(unsigned long i = 0; i < bitlen[n]; i++) // the bits for this code
      {
        unsigned long bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;

        if(treepos > numcodes - 2)
          return 55;

        if(tree2d[2 * treepos + bit] == 32767) // not yet filled in
        {
          if(i + 1 == bitlen[n])
          {
            tree2d[2 * treepos + bit] = n;
            treepos = 0;
          } // last bit
          else
          {
            tree2d[2 * treepos + bit] = ++nodefilled + numcodes;
            treepos = nodefilled;
          } // addresses are encoded as values > numcodes
        }
        else
          treepos = tree2d[2 * treepos + bit] - numcodes; // subtract numcodes from address to get address value
      }

    return 0;
  }

  int decode(bool& decoded, unsigned long& result, size_t& treepos, unsigned long bit) const
  {
    // Decodes a symbol from the tree
    unsigned long numcodes = (unsigned long)tree2d.size() / 2;

    if(treepos >= numcodes)
      return 11; // error: you appeared outside the codetree

    result = tree2d[2 * treepos + bit];
    decoded = (result < numcodes);
    treepos = decoded ? 0 : result - numcodes;
    return 0;
  }

  // 2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all nodes and leaves of the tree.
  std::vector<unsigned long> tree2d;
};

struct Inflator
{
  int error;

  void inflate(std::vector<uint8_t>& out, Span<const uint8_t> in, size_t inpos = 0)
  {
    size_t bp = 0, pos = 0; // bit pointer and byte pointer
    error = 0;
    unsigned long BFINAL = 0;

    while(!BFINAL && !error)
    {
      if((bp >> 3) >= (size_t)in.len)
      {
        error = 52;
        return;
      } // error, bit pointer will jump past memory

      BFINAL = readBit(bp, &in[inpos]);
      unsigned long BTYPE = readBit(bp, &in[inpos]);
      BTYPE += 2 * readBit(bp, &in[inpos]);

      if(BTYPE == 3)
      {
        error = 20;
        return;
      } // error: invalid BTYPE
      else if(BTYPE == 0)
        inflateNoCompression(out, &in[inpos], bp, pos, in.len);
      else
        inflateHuffmanBlock(out, &in[inpos], bp, pos, in.len, BTYPE);
    }

    if(!error)
      out.resize(pos); // Only now we know the true size of out, resize it to that
  }

  void generateFixedTrees(HuffmanTree& tree, HuffmanTree& treeD) // get the tree of a deflated block with fixed tree
  {
    std::vector<unsigned long> bitlen(288, 8), bitlenD(32, 5);

    for(size_t i = 144; i <= 255; i++)
      bitlen[i] = 9;

    for(size_t i = 256; i <= 279; i++)
      bitlen[i] = 7;

    tree.makeFromLengths(bitlen, 15);
    treeD.makeFromLengths(bitlenD, 15);
  }

  // the code tree for Huffman codes, dist codes, and code length codes
  HuffmanTree codetree, codetreeD, codelengthcodetree;

  unsigned long huffmanDecodeSymbol(const uint8_t* in, size_t& bp, const HuffmanTree& codetree, size_t inlength)
  { // decode a single symbol from given list of bits with given code tree. return value is the symbol
    bool decoded;
    unsigned long ct;

    for(size_t treepos = 0;;)
    {
      if((bp & 0x07) == 0 && (bp >> 3) > inlength)
      {
        error = 10;
        return 0;
      } // error: end reached without endcode

      error = codetree.decode(decoded, ct, treepos, readBit(bp, in));

      if(error)
        return 0; // stop, an error happened

      if(decoded)
        return ct;
    }
  }

  void getTreeInflateDynamic(HuffmanTree& tree, HuffmanTree& treeD, const uint8_t* in, size_t& bp, size_t inlength)
  { // get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
    std::vector<unsigned long> bitlen(288, 0), bitlenD(32, 0);

    if(bp >> 3 >= inlength - 2)
    {
      error = 49;
      return;
    } // the bit pointer is or will go past the memory

    size_t HLIT = readBits(bp, in, 5) + 257; // number of literal/length codes + 257
    size_t HDIST = readBits(bp, in, 5) + 1; // number of dist codes + 1
    size_t HCLEN = readBits(bp, in, 4) + 4; // number of code length codes + 4
    std::vector<unsigned long> codelengthcode(19); // lengths of tree to decode the lengths of the dynamic tree

    for(size_t i = 0; i < 19; i++)
      codelengthcode[CLCL[i]] = (i < HCLEN) ? readBits(bp, in, 3) : 0;

    error = codelengthcodetree.makeFromLengths(codelengthcode, 7);

    if(error)
      return;

    size_t i = 0, replength;

    while(i < HLIT + HDIST)
    {
      unsigned long code = huffmanDecodeSymbol(in, bp, codelengthcodetree, inlength);

      if(error)
        return;

      if(code <= 15)
      {
        if(i < HLIT)
          bitlen[i++] = code;
        else
          bitlenD[i++ - HLIT] = code;
      } // a length code
      else if(code == 16) // repeat previous
      {
        if(bp >> 3 >= inlength)
        {
          error = 50;
          return;
        } // error, bit pointer jumps past memory

        replength = 3 + readBits(bp, in, 2);
        unsigned long value; // set value to the previous code

        if((i - 1) < HLIT)
          value = bitlen[i - 1];
        else
          value = bitlenD[i - HLIT - 1];

        for(size_t n = 0; n < replength; n++) // repeat this value in the next lengths
        {
          if(i >= HLIT + HDIST)
          {
            error = 13;
            return;
          } // error: i is larger than the amount of codes

          if(i < HLIT)
            bitlen[i++] = value;
          else
            bitlenD[i++ - HLIT] = value;
        }
      }
      else if(code == 17) // repeat "0" 3-10 times
      {
        if(bp >> 3 >= inlength)
        {
          error = 50;
          return;
        } // error, bit pointer jumps past memory

        replength = 3 + readBits(bp, in, 3);

        for(size_t n = 0; n < replength; n++) // repeat this value in the next lengths
        {
          if(i >= HLIT + HDIST)
          {
            error = 14;
            return;
          } // error: i is larger than the amount of codes

          if(i < HLIT)
            bitlen[i++] = 0;
          else
            bitlenD[i++ - HLIT] = 0;
        }
      }
      else if(code == 18) // repeat "0" 11-138 times
      {
        if(bp >> 3 >= inlength)
        {
          error = 50;
          return;
        } // error, bit pointer jumps past memory

        replength = 11 + readBits(bp, in, 7);

        for(size_t n = 0; n < replength; n++) // repeat this value in the next lengths
        {
          if(i >= HLIT + HDIST)
          {
            error = 15;
            return;
          } // error: i is larger than the amount of codes

          if(i < HLIT)
            bitlen[i++] = 0;
          else
            bitlenD[i++ - HLIT] = 0;
        }
      }
      else
      {
        error = 16;
        return;
      } // error: somehow an unexisting code appeared. This can never happen.
    }

    if(bitlen[256] == 0)
    {
      error = 64;
      return;
    } // the length of the end code 256 must be larger than 0

    error = tree.makeFromLengths(bitlen, 15);

    if(error)
      return; // now we've finally got HLIT and HDIST, so generate the code trees, and the function is done

    error = treeD.makeFromLengths(bitlenD, 15);

    if(error)
      return;
  }

  void inflateHuffmanBlock(std::vector<uint8_t>& out, const uint8_t* in, size_t& bp, size_t& pos, size_t inlength, unsigned long btype)
  {
    if(btype == 1)
    {
      generateFixedTrees(codetree, codetreeD);
    }
    else if(btype == 2)
    {
      getTreeInflateDynamic(codetree, codetreeD, in, bp, inlength);

      if(error)
        return;
    }

    for(;;)
    {
      unsigned long code = huffmanDecodeSymbol(in, bp, codetree, inlength);

      if(error)
        return;

      if(code == 256)
        return; // end code
      else if(code <= 255) // literal symbol
      {
        if(pos >= out.size())
          out.resize((pos + 1) * 2); // reserve more room

        out[pos++] = (uint8_t)(code);
      }
      else if(code >= 257 && code <= 285) // length code
      {
        size_t length = LENBASE[code - 257], numextrabits = LENEXTRA[code - 257];

        if((bp >> 3) >= inlength)
        {
          error = 51;
          return;
        } // error, bit pointer will jump past memory

        length += readBits(bp, in, numextrabits);
        unsigned long codeD = huffmanDecodeSymbol(in, bp, codetreeD, inlength);

        if(error)
          return;

        if(codeD > 29)
        {
          error = 18;
          return;
        } // error: invalid dist code (30-31 are never used)

        unsigned long dist = DISTBASE[codeD], numextrabitsD = DISTEXTRA[codeD];

        if((bp >> 3) >= inlength)
        {
          error = 51;
          return;
        } // error, bit pointer will jump past memory

        dist += readBits(bp, in, numextrabitsD);
        size_t start = pos, back = start - dist; // backwards

        if(pos + length >= out.size())
          out.resize((pos + length) * 2); // reserve more room

        for(size_t i = 0; i < length; i++)
        {
          out[pos++] = out[back++];

          if(back >= start)
            back = start - dist;
        }
      }
    }
  }

  void inflateNoCompression(std::vector<uint8_t>& out, const uint8_t* in, size_t& bp, size_t& pos, size_t inlength)
  {
    while((bp & 0x7) != 0)
      bp++; // go to first boundary of byte

    size_t p = bp / 8;

    if(p >= inlength - 4)
    {
      error = 52;
      return;
    } // error, bit pointer will jump past memory

    unsigned long LEN = in[p] + 256 * in[p + 1], NLEN = in[p + 2] + 256 * in[p + 3];
    p += 4;

    if(LEN + NLEN != 65535)
    {
      error = 21;
      return;
    } // error: NLEN is not one's complement of LEN

    if(pos + LEN >= out.size())
      out.resize(pos + LEN);

    if(p + LEN > inlength)
    {
      error = 23;
      return;
    } // error: reading outside of in buffer

    for(unsigned long n = 0; n < LEN; n++)
      out[pos++] = in[p++]; // read LEN bytes of literal data

    bp = p * 8;
  }
};
}

using namespace std;

// https://tools.ietf.org/html/rfc1950#page-5
//
// zlib header:
// - 2 bytes: 0x78 0x9C
// - <deflated stream>
// - 4 bytes: adler32 checksum
vector<uint8_t> zlibDecompress(Span<const uint8_t> in)
{
  if(in.len < 2)
    throw Error("zlibDecompress: zlib data too small");

  if((in[0] * 256 + in[1]) % 31 != 0)
    throw Error("zlibDecompress: invalid header");

  const int CM = (in[0] >> 0) & 15;
  const int CINFO = (in[0] >> 4) & 15;
  const int FDICT = (in[1] >> 5) & 1;

  if(CM != 8 || CINFO > 7)
    throw Error("zlibDecompress: unsupported compression method");

  if(FDICT != 0)
    throw Error("zlibDecompress: unsupported preset directory");

  vector<uint8_t> out;
  Inflator inflator;
  inflator.inflate(out, in, 2);
  // skip adler32 checksum

  if(inflator.error)
    throw Error("zlibDecompress: decompression error");

  return out;
}

// gzip header:
// - 2 bytes: magic: 0x1f 0x8b
// - 1 byte: compression method (0x08 = deflate)
// - 1 byte $00 FLaGs bit 0   FTEXT - indicates file is ASCII text (can be safely ignored) bit 1   FHCRC - there is a CRC16 for the header immediately following the header bit 2   FEXTRA - extra fields are present bit 3   FNAME - the zero-terminated filename is present. encoding; ISO-8859-1. bit 4   FCOMMENT - a zero-terminated file comment is present. encoding: ISO-8859-1 bit 5-7   reserved
// - 4 bytes: Modification time
// - 1 byte: extra flags
// - 1 byte: OS
// - <deflated stream>
// - 4 bytes crc32
// - 4 bytes input size
vector<uint8_t> gzipDecompress(Span<const uint8_t> in)
{
  if(in.len < 10)
    throw Error("gzipDecompress: gzip data too small");

  if(in[0] != 0x1F || in[1] != 0x8B)
    throw Error("gzipDecompress: invalid header");

  const int CM = in[2];

  if(CM != 8)
    throw Error("gzipDecompress: unsupported compression method");

  const int flags = in[3];

  if(flags != 0)
    throw Error("gzipDecompress: unsupported flags");

  vector<uint8_t> out;
  Inflator inflator;
  inflator.inflate(out, in, 10);
  // skip crc32 and input size

  if(inflator.error)
    throw Error("gzipDecompress: decompression error");

  return out;
}

