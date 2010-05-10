/*
 * compessedBlock.hpp
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

#ifndef COMPRESSED_BLOCK_HPP__
#define COMPRESSED_BLOCK_HPP__

#include <vector>
#include <fstream>
#include <stdint.h>

namespace SE{

/**
 * Compressed Blcok
 * Manage Compressed Integer Sequence
 */
class CompressedBlock {
public:
  CompressedBlock(); ///< Constructor
  virtual ~CompressedBlock(); ///< Destructor
  virtual void encode(const std::vector<uint32_t>& v) = 0;
  virtual void decode(std::vector<uint32_t>& v) = 0;
  virtual size_t size() const = 0;
  virtual int save(std::ofstream& ofs) const = 0;
  virtual int load(std::ifstream& ifs) = 0;
};

}

#endif // COMPRESSED_BLOCK_HPP__
