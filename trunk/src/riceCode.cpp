/*
 * riceCode.cpp
 * Copyright (c) 2009 Daisuke Okanohara All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "riceCode.hpp"
#include <cassert>

using namespace std;

namespace SE{

RiceCode::RiceCode() : offset(0), bytePos(0) {}
RiceCode::RiceCode(const vector<uint32_t>& v) : offset(0), bytePos(0) {
  encode(v);
}
RiceCode::~RiceCode(){}

void RiceCode::encode(const vector<uint32_t>& v){
  if (v.size() == 0) return;
  if (v.size() == 1) return;
  uint32_t b   = (v.back() - v.front()) / (v.size()-1);
  uint32_t radix = 1;
  while (b >> radix){
    radix++;
  }

  offset = 0;
  B.push_back(v.front());
  assert(radix != 32);
  B.push_back(0);
  putBits(1U << radix, radix + 1);

  for (size_t i = 1; i < v.size(); ++i){
    uint32_t dif = v[i] - v[i-1] - 1;
    uint32_t low = dif & ((1 << radix) - 1);
    uint32_t up  = dif >> radix;

    setUnary(up);
    putBits(low, radix);
  }
}

void RiceCode::setUnary(uint32_t x) {
  offset += x;
  while (offset >= 32){
    B.push_back(0);
    offset -= 32;
  }
  B.back() |= (1U << offset);
  if (++offset == 32){
    B.push_back(0);
    offset = 0;
  }
}


void RiceCode::putBits(uint32_t x, uint32_t w){
  if (offset + w >= 32){
    B.back() |= (x << offset);
    w -= (32 - offset);
    x >>= (32 - offset);
    offset = 0;
    B.push_back(0);
  }
  B.back() |= (x << offset);
  offset += w;
}

uint32_t RiceCode::getBits(uint32_t w){
  if (offset + w <= 32){
    uint32_t ret = (B[bytePos] >> offset) & ((1U << w) - 1);
    offset += w;
    if (offset == 32){
      bytePos++;
      offset = 0;
    }
    return ret;
  } else {
    // offset + w > 32
    // w > 32 - offset
    uint32_t ret = (B[bytePos] >> offset);
    uint32_t w2 = w - 32 + offset;
    ret += (B[bytePos+1] & ((1U << w2) - 1)) << (32 - offset);
    bytePos++;
    offset = w2;
    return ret;
  }
}

uint32_t RiceCode::getUnary() {
  uint32_t count = 0;
  for (;;){
    uint32_t bit = (B[bytePos] >> offset) & 1U;
    offset++;
    if (offset == 32){
      bytePos++;
      offset = 0;
    }
    if (bit) break;
    count++;
  }
  return count;
}

void RiceCode::decode(vector<uint32_t>& v) {
  if (v.size() == 0) return;
  v[0] = B[0];
  bytePos = 1;
  offset = 0;
  uint32_t radix  = getUnary();

  for (size_t i = 1; i < v.size(); ++i){
    uint32_t up  = getUnary();
    uint32_t low = getBits(radix);
    v[i] = (up << radix) + low + v[i-1] + 1;
  }
}

size_t RiceCode::size() const{
  return B.size() * sizeof(B[0]);
}

int RiceCode::save(ofstream& ofs) const{
  uint32_t size = static_cast<uint32_t>(B.size());
  if (!ofs.write((const char*)(&size), sizeof(size))) return -1;
  if (size == 0) return 0;
  if (!ofs.write((const char*)(&B[0]), sizeof(B[0]) * B.size())) return -1;
  return 0;
}
  
int RiceCode::load(ifstream& ifs) {
  uint32_t size = 0;
  if (!ifs.read((char*)(&size), sizeof(size))) return -1;
  B.resize(size);
  if (!ifs.read((char*)(&B[0]), sizeof(B[0]) * size)) return -1;
  return 0;
}

}
