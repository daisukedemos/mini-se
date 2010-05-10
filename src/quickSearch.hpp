/*
 * quickSearch.hpp
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

#ifndef QUICK_SEARCH_HPP__
#define QUICK_SEARCH_HPP__

#include "miniseBase.hpp"

namespace SE{

/**
 * Sequential Search by QuickSearch)
 */
class QuickSearch : public Minise {
public:
  QuickSearch(); ///< Constructor
  ~QuickSearch(); ///< Destructor

  int save(const char* index); ///< Save current index to the file (text itself)
  int load(const char* index); ///< Save current index to the file (text itself)
  void addIndex(const std::vector<uint8_t>& content); ///< Do nothing
  int build();

  std::string getIndexName() const;
  size_t getIndexSize() const;

private:
  void search(const std::vector<uint8_t>& query, std::vector<SeResult>& ret);
};

}

#endif // QUICK_SEARCH_HPP__
