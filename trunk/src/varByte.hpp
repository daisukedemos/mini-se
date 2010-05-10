/*
 * varByte.hpp
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

#ifndef VAR_BYTE_HPP__
#define VAR_BYTE_HPP__

#include "compressedBlock.hpp"

namespace SE{

/**
 * Variable length Byte Code
 */
class VarByte : public CompressedBlock {
public:
  VarByte(); ///< Constructor
  VarByte(const std::vector<uint32_t>& v);
  ~VarByte(); ///< Destructor

  void encode(const std::vector<uint32_t>& v);
  void decode(std::vector<uint32_t>& v);
  size_t size() const;

  int save(std::ofstream& ofs) const;
  int load(std::ifstream& ifs);

private:
  std::vector<uint8_t> B;
};

}

#endif // VAR_BYTE_HPP__
