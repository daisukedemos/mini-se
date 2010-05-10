/*
 * riceCode.hpp
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

#ifndef RICE_CODE_HPP__
#define RICE_CODE_HPP__

#include "compressedBlock.hpp"

namespace SE{

/**
 * Rice Code
 */
class RiceCode : public CompressedBlock {
public:
  RiceCode(); ///< Constructor
  RiceCode(const std::vector<uint32_t>& v);
  ~RiceCode(); ///< Destructor

  void encode(const std::vector<uint32_t>& v);
  void decode(std::vector<uint32_t>& v);
  size_t size() const;

  int save(std::ofstream& ofs) const;
  int load(std::ifstream& ifs);

private:
  void setUnary(uint32_t x);
  void  putBits(uint32_t x, uint32_t w);
  uint32_t getBits(uint32_t w);
  uint32_t getUnary();
  uint32_t offset;
  uint32_t bytePos;
  std::vector<uint32_t> B;
};

}

#endif // RICE_CODE_HPP__
