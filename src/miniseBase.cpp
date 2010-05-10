/*
 * minise.cpp
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

#include <stdio.h>
#include <algorithm>
#include <sstream>
#include "miniseBase.hpp"

using namespace std;

namespace SE{

SeResult::SeResult(string& title, uint32_t docID, vector<uint32_t>& offsets) :  title(title), docID(docID), offsets(offsets)
{
}

Minise::Minise() : docN(0), termN(0) {
  docOffsets.push_back(0);
}

Minise::~Minise(){}

int Minise::addFile(const char* fileName){
 FILE* fp = fopen(fileName, "rb");
  if (fp == NULL){
    what_ << "cannot open " << fileName;
    return -1;
  }
  
  fseek(fp, 0, SEEK_END);
  size_t fileSize = ftell(fp);
  rewind(fp);
  
  vector<uint8_t> content(fileSize);
  
  size_t readSize = fread(&content[0], sizeof(uint8_t), fileSize, fp);
  if (readSize != fileSize){
    what_ << "read error readSize:" << readSize << " fileSize:" << fileSize;
    fclose(fp);
    return -1;
  }

  addDoc(fileName, content);
  fclose(fp);
  return 0;
}

void Minise::addDoc(const char* title, const vector<uint8_t>& content){
  titles.push_back(title);

  addIndex(content);
  for (size_t i = 0; i < content.size(); ++i){
    text.push_back(content[i]);
  }
  text.push_back(0); // Guard
  docOffsets.push_back(static_cast<uint32_t>(text.size()));
  
  docN++;
}


uint32_t Minise::getID(const string& str, const bool modify){
  map<string, uint32_t>::const_iterator it = term2id.find(str);
  if (it != term2id.end()){
    return it->second;
  } else if (modify){
    id2term.push_back(str);
    return term2id[str] = termN++;
  } else {
    return NOTFOUND;
  }
}

uint32_t Minise::getiID(const uint64_t str, const bool modify){
  map<uint64_t, uint32_t>::const_iterator it = iterm2id.find(str);
  if (it != iterm2id.end()){
    return it->second;
  } else if (modify){
    id2iterm.push_back(str);
    return iterm2id[str] = termN++;
  } else {
    return NOTFOUND;
  }
}

void Minise::search(const char* query, const size_t len, vector<SeResult>& ret){
  ret.clear();
  if (len == 0) return;
  string query_s(query);
  istringstream is(query_s);
  vector< vector<SeResult> > origRets;
  string querySingle;

  while (is >> querySingle){
    vector<uint8_t> vquery;
    for (size_t i = 0; i < querySingle.size(); ++i){
      vquery.push_back(static_cast<uint8_t>(querySingle[i]));
    }
    vector<SeResult> retSingle;
    search(vquery, retSingle);
    origRets.push_back(retSingle);
  }

  searchAND(origRets, ret);
  rankByTF(ret);
  return ;

  while (is >> querySingle){
    vector<uint8_t> vquery;
    for (size_t i = 0; i < querySingle.size(); ++i){
      vquery.push_back(static_cast<uint8_t>(querySingle[i]));
    }
    vector<SeResult> retSingle;
    search(vquery, retSingle);
    origRets.push_back(retSingle);
  }

  searchAND(origRets, ret);
  rankByTF(ret);
}

void Minise::searchAND(vector<vector<SeResult> >& origRets, vector<SeResult>& andRet){
  if (origRets.size() == 0) return;
  vector<pair<size_t, size_t> > ord;
  for (size_t i = 0; i < origRets.size(); ++i){
    ord.push_back(make_pair(origRets[i].size(), i));
  }

  sort(ord.begin(), ord.end());
  andRet.swap(origRets[ord[0].second]);

  for (size_t i = 1; i < ord.size(); ++i){
    vector<SeResult>& ret(origRets[ord[i].second]);
    vector<SeResult> next;
    size_t ind = 0;
    for (size_t j = 0; j < andRet.size(); ++j){
      uint32_t docID = andRet[j].docID;
      
      vector<SeResult>::const_iterator it = 
	lower_bound(ret.begin() + ind, ret.end(), docID);
      if (it == ret.end()) break;
      if (it->docID == docID){
	SeResult r(andRet[j].title, docID, andRet[j].offsets);
	for (size_t k = 0; k < it->offsets.size(); ++k){
	  r.offsets.push_back(it->offsets[k]);
	}
	next.push_back(r);
      }
    }
    andRet.swap(next);
  }
}

class CompByTF{
public:
  int operator () (const SeResult& sr1, const SeResult& sr2) const{
    return sr1.offsets.size() < sr2.offsets.size();
  }
};

void Minise::rankByTF(vector<SeResult>& ret){
  sort(ret.rbegin(), ret.rend(), CompByTF());
}

void Minise::getSnippet(const uint32_t docID, const int offset, const uint32_t len, string& ret) const{
  const uint32_t beg = docOffsets[docID] + offset;
  const uint32_t end = std::min(beg + len, docOffsets[docID+1]); // [docN] = test.size()
  
  for (size_t i = beg; i < end; ++i){
    ret.push_back(text[i]);
  }
}

void Minise::decodeDoc(const vector<uint32_t>& cand, vector<SeResult>& res){
  if (cand.size() == 0) return;
  
  uint32_t begDocID = 0;
  for (size_t i = 0; i < cand.size(); ){
    vector<uint32_t>::const_iterator it = 
      upper_bound(docOffsets.begin() + begDocID, docOffsets.end(), cand[i]);
    uint32_t cur_offset = *(it-1);
    uint32_t next_offset = *it;
    vector<uint32_t> offsets;
    while (i < cand.size() && cand[i] < next_offset ){
      offsets.push_back(cand[i] - cur_offset);
      ++i;
    }
    uint32_t docID = it - docOffsets.begin() - 1;
    res.push_back(SeResult(titles[docID], docID, offsets));
    begDocID = docID + 1;
  }
}


string Minise::what() const {
  return what_.str();
}

int Minise::write(const vector<string>& vs, const char* vnmae, ofstream& ofs){
  uint32_t size = static_cast<uint32_t>(vs.size());
  if (write(size, "vector<string>.size()", ofs) == -1) {
    return -1;
  }

  for (size_t i = 0; i < vs.size(); ++i){
    uint32_t ssize = vs[i].size();
    if (write(ssize, "string.size", ofs) == -1) {
      return -1;
    }

    if (!ofs.write((const char*)(&vs[i][0]), sizeof(char) * ssize)){
      what_ << "vector::string::write error";
      return -1;
    }
  }

  return 0;
}

int Minise::read(vector<string>& vs, const char* vname, ifstream& ifs){
  uint32_t size = 0;
  if (read(size, "string::size", ifs) == -1){
    return -1;
  }

  vs.resize(size);
  for (size_t i = 0; i < vs.size(); ++i){
    uint32_t ssize = 0;
    if (read(ssize, "strig::size", ifs) == -1){
      return -1;
    }

    vs[i].resize(ssize);
    if (!ifs.read((char*)(&vs[i][0]), sizeof(char) * ssize)){
      what_ << "vector::string::read error";
      return -1;
    }
  }
  return 0;
}

size_t Minise::getIndexSize() const {
  size_t ret = 0;
  ret += text.size() * sizeof(uint8_t);
  
  for (size_t i = 0; i < id2term.size(); ++i){
    ret += id2term[i].size();
  }

  ret += sizeof(uint32_t) * id2iterm.size();

  for (size_t i = 0; i < titles.size(); ++i){
    ret += titles[i].size();
  }

  ret += docOffsets.size() * sizeof(uint32_t);
  return ret;
}

void Minise::setParseType(const ParseType& pt_){
  pt = pt_;
}

int Minise::parse(const vector<uint8_t>& buf, const bool modify, parseResult& parsed){
  if (pt == C_ONEGRAM || pt == C_TWOGRAM){
    return parseUTF8(buf, modify, parsed);
  } else if (pt == SEPARATED){
    return parseSeparated(buf, modify, parsed);
  } else {
    what_ << "Unknown Lang";
    return -1;
  }
}


int Minise::parseUTF8(const vector<uint8_t>& buf, const bool modify, parseResult& parsed){
  uint32_t cur = 0;
  uint32_t prev = NOTFOUND;
  uint32_t len = 0;
  bool first = true;
  for (size_t i = 0; i <= buf.size(); ++i){
    if (first ||
	(i != buf.size() && (buf[i] & 0xC0) == 0x80)){
      cur <<= 8;
      cur += buf[i];
      len++;
      first = false;
      continue;
    }

    uint64_t term = (pt != C_TWOGRAM) ? cur : ((uint64_t)prev << 32) + cur;

    if (pt != C_TWOGRAM || prev != NOTFOUND){
      const uint32_t id = getiID(term, modify);
      if (id == NOTFOUND) return -1;
      const uint32_t offset = static_cast<uint32_t>(i - len);
      parsed.push_back(make_pair(id, offset));
    }

    if (C_TWOGRAM){
      prev = cur;
    }

    if (i == buf.size()) break;
    cur = buf[i]; // cur.clear() 
    len = 1;
  }

  if (pt == C_TWOGRAM && 
      parsed.size() == 0 &&
      prev != NOTFOUND){
    // Special-case: Query is One character
    uint32_t id = NOTFOUND;
    parsed.push_back(make_pair(id, 0));
  }
      
  return 0;
}

int Minise::parseSeparated(const vector<uint8_t>& buf, const bool modify, parseResult& parsed){
  string cur;
  bool first = true;
  for (size_t i = 0; i <= buf.size(); ++i){
    if (first ||
	(i != buf.size() && !isspace(buf[i]))){
      cur += buf[i];
      first = false;
      continue;
    } 
    if (cur.size() == 0) continue;
    const uint32_t id = getID(cur, modify);
    if (id == NOTFOUND) return -1;
    parsed.push_back(make_pair(id, static_cast<uint32_t>(i - cur.size())));
    cur.clear();
    // buf[i] is space
  }
  return 0;
}

int getIndexType(const char* indexName, Minise::IndexType& indexType){
  ifstream ifs(indexName);
  if (!ifs){
    return -1;
  }
  if (!ifs.read((char*)&indexType, sizeof(Minise::IndexType))){
    return -1;
  }
  return 0;
}


}
