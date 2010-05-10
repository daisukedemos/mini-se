/*
 * invertedFile.hpp
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

#ifndef INVERTED_FILE_HPP__
#define INVERTED_FILE_HPP__

#include "miniseBase.hpp"
#include "varByte.hpp"
#include "riceCode.hpp"

namespace SE{

/**
 * Inverted File Index.
 * Also support 1/2-gram index
 */

class InvertedFile : public Minise {
  enum {
    BLOCKSIZE = 128,
  };
public:
  enum compressMethod {
    NONE = 0,
    VARBYTE = 1,
    RICECODE = 2
  };

  InvertedFile(); ///< Constructor
  ~InvertedFile(); ///< Destructor

  int save(const char* fileName); ///< Save the index to file
  int load(const char* fileName); ///< Load the index from file

  int build();
  size_t getIndexSize() const;
  void setCompressMethod(const compressMethod& cm_);
  std::string getIndexName() const;

private:
  void search(const std::vector<uint8_t>& query, std::vector<SeResult>& ret); ///< Search the document for the query
  void searchOneCharacter(const std::vector<uint8_t>& query, std::vector<SeResult>& ret);

  void merge(const std::pair<uint32_t, uint32_t> qid, std::vector<uint32_t>& cand);
  void addIndex(const std::vector<uint8_t>& content);

  std::vector<std::vector<uint32_t> > posList;
  std::vector<std::vector<CompressedBlock*>  > cPosList;
  std::vector<std::vector<uint32_t> > blockFront;

  std::vector<uint32_t> buf;

  compressMethod cm;
};

}

#endif // INVERTED_FILE_HPP__
