/*
 * miniseBase.hpp
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

#ifndef MINISE_BASE_HPP__
#define MINISE_BASE_HPP__

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <stdint.h>
#include "varByte.hpp"

namespace SE{

/**
 * Search result.
 * Store a hit document's ID, title, and hit positions.
 */
struct SeResult{
  SeResult(); ///< Default Constructor 
  SeResult(std::string& title, uint32_t docID, std::vector<uint32_t>& offsets); ///< Constructor with set values

  std::string title;      ///< A title of a hit document
  uint32_t docID;         ///< A document ID in Minise
  std::vector<uint32_t> offsets;  ///< Hit positions offsets;

  bool operator < (const int val) const{
    return (int)docID < val;
  }
};

/**
 * Base class for search engines.
 */
class Minise{
public:
 /**
   * Index type
   */
  enum IndexType{
    QUICKSEARCH = 0,     ///< QuickSearch (Sequential Search)
    ONEGRAM = 1,         ///< Character 1-gram
    TWOGRAM = 2,         ///< Character 2-gram
    INVERTEDFILE = 3,    ///< Inverted File
    SUFFIXARRAY = 4,     ///< Suffix Array
    SUFFIXARRAY_UTF8 = 5 ///< Suffix Array for UTF-8
  };

  /**
   * Parsing Method
   */
  enum ParseType{
    C_ONEGRAM = 0,   ///< UTF-8 Character 1-gram
    C_TWOGRAM = 1,   ///< UTF-8 Character 2-gram
    SEPARATED = 2,   ///< Terms are separated by Space/Tab
  };


  enum {
    NOTFOUND = 0xFFFFFFFF /// Indicate the unknown ID
  };

  typedef std::vector<std::pair<uint32_t, uint32_t> > parseResult;

  Minise();            ///< Constructor
 
  virtual ~Minise();   ///< Destructor

  /** 
   * Register a new file to an index
   * @param fileName a file name to be registred
   * @return Return 0 if it succeded or -1 if failed
   */
  int addFile(const char* fileName);

  /**
   * Register a new document to an index
   * @param title A title of the document 
   * @param content A data of the document (UTF-8)
   */ 
  void addDoc(const char* title, const std::vector<uint8_t>& content);

  /**
   * Full-text search for a query using an index.
   * @param query A query 
   * @param len A length of the query
   * @param ret A search result
   */
  void search(const char* query, const size_t len, std::vector<SeResult>& ret);

  /**
   * Save the current index to disk
   * @param fileName An index file name
   * @return Return 0 if it succeded or -1 if failed
   */
  virtual int save(const char* index) = 0;

  /**
   * Load the index from disk
   * @param fileName An index file name 
   * @return Return 0 if succeded or -1 if failed
   */
  virtual int load(const char* fileName) = 0;

  /**
   * Build Index for some offline index (e.g. Suffix Array)
   * @return Return 0 if succeded or -1 if failed
   */
  virtual int build() = 0;

  /**
   * @return The number of registered docs
   */
  uint32_t getDocN() const {
    return docN;
  }

  /**
   * @return The number of used terms
   */
  uint32_t getTermN() const{
    return termN;
  }

  /**
   * Given an document ID, and a position, this returns the snipet around the position
   * @param docID document ID
   * @param offset A position in the document
   * @param len A length of a snipet
   */
  void getSnippet(const uint32_t docID, const int offset, 
		 const uint32_t len, std::string& ret) const;

  /**
   * Return the index type name (may not equal to class name)
   * @return A name of an index type
   */
  virtual std::string getIndexName() const = 0;

  /**
   * Return the index size (Not exact size)
   * @return A size of an index size
   */
  virtual size_t getIndexSize() const;

  /**
   * Set the parsing method at index building.
   * @param pt_ parsing method
   */
  void setParseType(const ParseType& pt_);

  /**
   * Report the status of the class. Use this when erros occured.
   * @return A status of the class
   */
  std::string what() const;

protected:
  /**
   * Full-text search for a query using an index
   * @param query A query 
   * @param ret A search result
   */
  virtual void search(const std::vector<uint8_t>& query, std::vector<SeResult>& ret) = 0;



  /**
   * Compute AND result
   * @param rets Results for single queries
   * @param andRet Result for AND query
   */
  static void searchAND(std::vector<std::vector<SeResult> >& rets, std::vector<SeResult>& andRet);

  /**
   * Sort results by term-frequency
   * @param ret Sort Result
   */
  static void rankByTF(std::vector<SeResult>& ret);

  /**
   * Assign ID to Term
   * @param str Term
   * @param modify Assign new ID to unknown term if modify==true
   * @return TermID or NOTFOUND if term is unknown term and modify==false
   */
  uint32_t getID(const std::string& str, const bool modify);

  /**
   * Assign ID to UTF-8Term
   * @param str Term
   * @param modify Assign new ID to unknown term if modify==true
   * @return TermID or NOTFOUND if term is unknown term and modify==false
   */
  uint32_t getiID(const uint64_t str, const bool modify);

  /**
   * Add the index for new document
   * @param content A content of new document
   */
  virtual void addIndex(const std::vector<uint8_t>& content) = 0;

  /**
   * Convert Global Positions into docs and offsets
   * @param cand Global Positions
   * @param ret Converted result
   */
  void decodeDoc(const std::vector<uint32_t>& cand, std::vector<SeResult>& ret);

  /**
   * Parse the input and extract terms
   * @param buf input to be parsed
   * @param modify Assign new termID to unknown term if modify==true
   * @param parsed result
   * @return Return 0 if succeded or -1 if failed
   */
  int parse(const std::vector<uint8_t>& buf, const bool modify, parseResult& parsed);

  /**
   * Parse the input and extract UTF-8 characters.
   * @param buf input to be parsed
   * @param modify Assign new termID to unknown term if modify==true
   * @param parsed result
   * @return Return 0 if succeded or -1 if failed
   */
  int parseUTF8(const std::vector<uint8_t>& buf, const bool modify, parseResult& parsed);

  /**
   * Parse the input where terms are seprated by space or tab
   * @param buf input to be parsed
   * @param modify Assign new ID to unknown term if modify==true
   * @param parsed result
   * @return Return 0 if succeded or -1 if failed
   */
  int parseSeparated(const std::vector<uint8_t>& buf, const bool modify, parseResult& parsed);

  int write(const std::vector<std::string>& vs, const char* vname, std::ofstream& ofs);
  int read(std::vector<std::string>& vs, const char* vname, std::ifstream& ifs);

  template<class T> int write(const std::vector<T>& v, const char* vname, std::ofstream& ofs){
    uint32_t size = static_cast<uint32_t>(v.size());
    if (write(size, vname, ofs) == -1) return -1;

    for (size_t i = 0; i < size; ++i){
      if (write(v[i], vname, ofs) == -1){
	return -1;
      }
    }
    return 0;
  }

  template<class T> int write(const T v, const char* vname, std::ofstream& ofs){
    if (!ofs.write((const char*)(&v), sizeof(T))){
      what_ << "write error:" << vname;
      return -1;
    }
    return 0;
  }

  template<class T> int read(std::vector<T>& v, const char* vname, std::ifstream& ifs){
    uint32_t size = 0;
    if (read(size, vname, ifs) == -1) return -1;

    if (size == 0) return 0;
    v.resize(size);
    for (size_t i = 0; i < size; ++i){
      if (read(v[i], vname, ifs) == -1){
	return -1;
      }
    }
    return 0;
  }

  template<class T> int read(T& v, const char* vname, std::ifstream& ifs){
    if (!ifs.read((char*)(&v), sizeof(T))){
      what_ << "read error:" << vname;
      return -1;
    }
    return 0;
  }

protected:
  std::vector<uint8_t> text;               ///< A concatenated text for registered documents.
  std::map<std::string, uint32_t> term2id; ///< A mapping from term to ID
  std::vector<std::string> id2term;        ///< A mapping from ID to term
  std::map<uint64_t, uint32_t> iterm2id; ///< A mapping from utf8term to ID
  std::vector<uint64_t> id2iterm;        ///< A mapping from ID to utf8term
  std::vector<std::string> titles;         ///< Titles of registered documents.OA
  std::vector<uint32_t> docOffsets;        ///< Beginning positions of documents in text
  uint32_t docN;                           ///< Number of documents
  uint32_t termN;                          ///< Number of (appeared) terms

  ParseType pt;                            ///< Parsing method 
  std::ostringstream what_;                ///< Message about the class's state
};

int getIndexType(const char* index, Minise::IndexType& indexType);

}

#endif // MINISE_BASE_HPP__

