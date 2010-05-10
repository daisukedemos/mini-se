/*
 * suffixArray.hpp
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

#ifndef SUFFIX_ARRAY_HPP__
#define SUFFIX_ARRAY_HPP__

#include "miniseBase.hpp"

namespace SE{

/**
 * Suffix Array index
 */
class SuffixArray : public Minise {
public:
  SuffixArray(); ///< Constructor
  ~SuffixArray(); ///< Destructor

  int save(const char* fileName); ///< save the index into file(fileName)
  int load(const char* fileName); ///< load the index from file
  int build(); ///< build an index for registered documents

  void setUTF8();

  std::string getIndexName() const;
  size_t getIndexSize() const;

private:
  void search(const std::vector<uint8_t>& query, std::vector<SeResult>& ret);
  int compare(const uint32_t ind, const std::vector<uint8_t>& query, uint32_t& offset) const;
  void bsearch(const std::vector<uint8_t>& query, 
	       uint32_t& beg, uint32_t& half, uint32_t& size, 
	       uint32_t& match, uint32_t& lmatch, uint32_t& rmatch, const int state);

  void addIndex(const std::vector<uint8_t>& content);
  uint32_t select(const uint32_t i, const std::vector<uint8_t>& B, const std::vector<uint32_t>& Btable) const;
  int buildUTF8();
  
  std::vector<uint32_t> SA; ///< Suffix Array
  bool useUTF8;
};

}

#endif // SUFFIX_ARRAY__
