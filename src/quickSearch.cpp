/*
 * quickSearch.cpp
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

#include "quickSearch.hpp"

using namespace std;

namespace SE{

QuickSearch::QuickSearch(){
}

QuickSearch::~QuickSearch(){
}

void QuickSearch::search(const vector<uint8_t>& query, vector<SeResult>& ret){
  vector<uint32_t> table(0x100);
  size_t m = query.size();
  size_t n = text.size();
  for (size_t i = 0; i < 0x100; ++i) {
    table[i] = m+1;
  }
  for (size_t i = 0; i < m; ++i) {
    table[query[i]] = m-i;
  }

  vector<uint32_t> hitPos;
  for (size_t i = 0; i+m < n; ){
    size_t j = 0;
    while (query[j] == text[i+j]) ++j;
    if (j == m) {
      hitPos.push_back(i);
    }
    i += table[text[i+m]];
  }
  decodeDoc(hitPos, ret);
}

int QuickSearch::save(const char* index){
  ofstream ofs(index);
  if (!ofs){
    what_ << "cannot open " << index;
    return -1;
  }

  if (write(QUICKSEARCH, "indexType", ofs) == -1) return -1;
  if (write(text, "text", ofs) == -1) return -1;
  if (write(docOffsets, "docOffsets", ofs) == -1) return -1;
  if (write(titles, "titles", ofs) == -1) return -1;

  return 0;
}

int QuickSearch::load(const char* index){
  ifstream ifs(index);
  if (!ifs){
    what_ << "cannot open " << index;
    return -1;
  }
  int indexType = -1;
  if (read(indexType, "indexType", ifs) == -1) return -1;
  if (indexType != QUICKSEARCH){
    what_ << "indexType is not QUICKSEARCH:" << indexType;
    return -1;
  }


  if (read(text, "text", ifs) == -1) return -1;
  if (read(docOffsets, "docOffsets", ifs) == -1) return -1;
  if (read(titles, "titles", ifs) == -1) return -1;

  docN = static_cast<uint32_t>(docOffsets.size())-1;
  return 0;
}

void QuickSearch::addIndex(const std::vector<uint8_t>& content){
}

int QuickSearch::build(){
  return 0;
}

string QuickSearch::getIndexName() const{
  return string("Quick Search");
}

size_t QuickSearch::getIndexSize() const {
  size_t ret = Minise::getIndexSize(); // Calculate Basic Class Size()
  return ret;
}




}


