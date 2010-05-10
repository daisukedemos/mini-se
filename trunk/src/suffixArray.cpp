/*
 * suffixArray.cpp
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

#include <algorithm>
#include "sais.hxx"
#include "suffixArray.hpp"

using namespace std;

namespace SE{

static const int _popcount_table[256] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


SuffixArray::SuffixArray() : useUTF8(false) {
}

SuffixArray::~SuffixArray(){
}

void  SuffixArray::addIndex(const std::vector<uint8_t>& content){
}

void SuffixArray::setUTF8(){
  useUTF8 = true;
}

int SuffixArray::build(){
  if (useUTF8){
    return buildUTF8();
  }

  SA.resize(text.size()+1);
  if (saisxx(text.begin(), SA.begin(), (int)text.size(), 0x100) != 0){
    what_ << "saisxx error";
    return -1;
  }
  return 0;
}

int SuffixArray::buildUTF8(){
  bool first = true;
  size_t n = text.size();
  vector<uint32_t> T;
  uint64_t cur = 0;

  vector<uint8_t> B((n + 8 - 1) / 8);
  for (size_t i = 0; i <= text.size(); ++i){
    if (first){
      B[i/8] |= (1U << (i%8));
    }
    if (first ||
	(i != text.size() && (text[i] & 0xC0) == 0x80)){
      cur <<= 8;
      cur += text[i];
      first = false;
      continue;
    }
    B[i/8] |= (1U << (i%8));
    const uint32_t id = getiID(cur, true);
    T.push_back(id);
    
    if (i == text.size()) break;
    cur = text[i];
  }

  vector<uint32_t> mapping(iterm2id.size());
  uint32_t alpha = 0;
  for (map<uint64_t, uint32_t>::const_iterator it = iterm2id.begin(); it != iterm2id.end(); ++it){
    mapping[it->second] = alpha++;
  }

  int alphaSize = static_cast<int>(iterm2id.size());
  iterm2id.clear();
  id2iterm.clear();

  for (size_t i = 0; i < T.size(); ++i){
    T[i] = mapping[T[i]];
  }
  mapping.clear();

  SA.resize(T.size()+1);
  if (saisxx(T.begin(), SA.begin(), (int)T.size(), alphaSize) != 0){
    what_ << "saisxx error";
    return -1;
  }

  vector<uint32_t> Btable(B.size() / 4);
  uint32_t sum = 0;
  for (size_t i = 0; i < B.size(); ++i){
    if (i % 4 == 0){
      Btable[i/4] = sum;
    }
    sum += _popcount_table[B[i]];
  }
  Btable.push_back(sum); // gurad

  for (size_t i = 0; i < SA.size(); ++i){
    SA[i] = select(SA[i], B, Btable); ///< Convert Position into Original Position
  }


  return 0;
}

/// Return select(B, i), the position of (i+1)-th one in B
uint32_t SuffixArray::select(const uint32_t i_, const vector<uint8_t>& B, const vector<uint32_t>& Btable) const{
  uint32_t i = i_ + 1;
  vector<uint32_t>::const_iterator it = lower_bound(Btable.begin(), Btable.end(), i);

  it--;
  uint32_t remain = i - *it;
  uint32_t blockPos = (it - Btable.begin()) * 4;
  uint32_t pos      = blockPos * 8;

  for (uint32_t i = blockPos; ; i++){
    uint32_t val =  _popcount_table[B[i]];
    if (val < remain){
      remain -= val;
      pos += 8;
      continue;
    }
    for (uint32_t j = 0;; ++j){
      if ((B[i] >> j) & 1U){
	remain--;
	if (remain == 0) break;
      }
      pos++;
    }
    break;
  }
  return pos;
}

/// Return -1 if text[ind...] < query, or return 1 otherwise
int SuffixArray::compare(const uint32_t ind, const vector<uint8_t>& query, uint32_t& match) const{
  while (match < query.size() && match + ind < text.size()){
    if (text[ind+match] != query[match]){
      return (int)text[ind+match] - query[match];
    }
    ++match;
  }
  if (match + ind == text.size()){
    return -1;
  } else { // match == query.size()
    return 0;
  }
}

void SuffixArray::bsearch(const vector<uint8_t>& query, 
			  uint32_t& beg, uint32_t& half, uint32_t& size, 
			  uint32_t& match, uint32_t& lmatch, uint32_t& rmatch, 
			  const int state){
  half = size/2;
  for (; size > 0; size = half, half /= 2){
    match = min(lmatch, rmatch);
    int r = compare(SA[beg + half], query, match);
    if (r < 0 || (r == 0 && state==2)){
      beg += half + 1;
      half -= (1 - (size & 1));
      lmatch = match;
    } else if (r > 0 || state > 0){
      rmatch = match;
    } else {
      // T[SA[beg+half]...]'s prefix equals to query
      break; 
    }
  }
}

void SuffixArray::search(const vector<uint8_t>& query, vector<SeResult>& res){
  res.clear();

  // Binary Search of the SA position containing a query as a prefix
  uint32_t beg    = 0;
  uint32_t size   = static_cast<uint32_t>(SA.size());
  uint32_t half   = size/2;
  uint32_t match  = 0;
  uint32_t lmatch = 0;
  uint32_t rmatch = 0;
  bsearch(query, beg, half, size, match, lmatch, rmatch, 0);

  //cerr << beg << endl
  //<< half << endl
  //<< size << endl
  //   << match << endl
  //   << lmatch << endl
  //   << rmatch << endl << endl;
  
  if (size == 0) return; // No matching found

  // Lower Bound
  uint32_t lbeg    = beg;
  uint32_t lsize   = half;
  uint32_t lhalf   = half / 2;
  uint32_t llmatch = lmatch;
  uint32_t lrmatch = match;
  uint32_t lmatch2 = 0;
  bsearch(query, lbeg, lhalf, lsize, lmatch2, llmatch, lrmatch, 1);

  // Upper Bound
  uint32_t rbeg    = beg + half + 1;
  uint32_t rsize   = size - half - 1;
  uint32_t rhalf   = rsize / 2;
  uint32_t rlmatch = match;
  uint32_t rrmatch = rmatch;
  uint32_t rmatch2 = 0;
  bsearch(query, rbeg, rhalf, rsize, rmatch2, rlmatch, rrmatch, 2);

  // SA[lbeg...rbeg) are matching positions;  
  vector<uint32_t> poses;
  for (uint32_t i = lbeg; i < rbeg; ++i){
    poses.push_back(SA[i]);
  }

  sort(poses.begin(), poses.end());
  decodeDoc(poses, res);
}

int SuffixArray::save(const char* fileName){
  ofstream ofs(fileName);
  if (!ofs){
    what_ << "cannot open " << fileName;
    return -1;
  }

  if (useUTF8){
    if (write(SUFFIXARRAY_UTF8, "indexType", ofs) == -1) return -1;
  } else {
    if (write(SUFFIXARRAY, "indexType", ofs) == -1) return -1;
  }
  if (write(text, "text", ofs) == -1) return -1;
  if (write(docOffsets, "docOffsets", ofs) == -1) return -1;
  if (write(titles, "titles", ofs) == -1) return -1;
  if (write(SA, "SA", ofs) == -1) return -1;

  return 0;
}

int SuffixArray::load(const char* fileName){
  ifstream ifs(fileName);
  if (!ifs){
    what_ << "cannot open " << fileName;
    return -1;
  }

  int indexType = -1;
  if (read(indexType, "indexType", ifs) == -1) return -1;
  if (indexType == SUFFIXARRAY_UTF8){
    useUTF8 = true; 
  } else if (indexType == SUFFIXARRAY){
    // do nohting
  } else {
    what_ << "indexType is not SUFFIX ARRAY, SUFFIX ARRAY UTF8";
    return -1;
  }
  if (read(text, "text", ifs) == -1) return -1;
  if (read(docOffsets, "docOffsets", ifs) == -1) return -1;
  if (read(titles, "titles", ifs) == -1) return -1;
  if (read(SA, "SA", ifs) == -1) return -1;
	 
  docN = static_cast<uint32_t>(docOffsets.size())-1;

  return 0;
}

string SuffixArray::getIndexName() const{
  if (useUTF8){
    return string("Suffix Array UTF8");
  } else {
    return string("Suffix Array");
  }
}


size_t SuffixArray::getIndexSize() const {
  size_t ret = Minise::getIndexSize(); // Calculate Basic Class Size()
  ret += SA.size() * sizeof(uint32_t);
  return ret;
}
  



}

