/*
 * invertedFile.cpp
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
#include <cassert>
#include "invertedFile.hpp"

using namespace std;

namespace SE{

InvertedFile::InvertedFile() : cm(NONE){
  buf.resize(BLOCKSIZE);
}

InvertedFile::~InvertedFile(){
  for (size_t i = 0; i < cPosList.size(); ++i){
    for (size_t j = 0; j < cPosList[i].size(); ++j){
      delete cPosList[i][j];
    }
  }
}

void  InvertedFile::addIndex(const std::vector<uint8_t>& content){
  const uint32_t offset = static_cast<uint32_t>(text.size());

  parseResult parsed;
  parse(content, true, parsed);
  if (parsed.size() == 0) return;

  for (size_t i = 0; i < parsed.size(); ++i){
    parsed[i].second += offset;
  }
  if (parsed.size() == 0) return;

  sort(parsed.begin(), parsed.end());
  
  if (parsed.back().first >= posList.size()){
    uint32_t maxID = parsed.back().first+1;
    posList.resize(maxID);
    cPosList.resize(maxID);
    blockFront.resize(maxID);
  }

  for (size_t i = 0; i < parsed.size(); ++i){
    uint32_t id = parsed[i].first;
    posList[id].push_back(parsed[i].second);
    if (cm != NONE && posList[id].size() >= BLOCKSIZE) {
      CompressedBlock* cb = NULL;
      if (cm == VARBYTE){
	cb = new VarByte(posList[id]);
      } else if (cm == RICECODE){
	cb = new RiceCode(posList[id]);
      } else {
	assert(false);
      }
      cPosList[id].push_back(cb);
      blockFront[id].push_back(posList[id].back());
      posList[id].clear();
    }
  }
}

void InvertedFile::setCompressMethod(const compressMethod& cm_){
  cm = cm_;
}

void InvertedFile::search(const vector<uint8_t>& query, vector<SeResult>& res) {
  res.clear();

  parseResult parsed;
  if (parse(query, false, parsed) == -1) return;
  if (parsed.size() == 0) return;

  if (pt == C_TWOGRAM){
    // Remove even elements.
    parseResult parsedEven;
    for (size_t i = 0; i < parsed.size(); i += 2){
      parsedEven.push_back(parsed[i]);
    }
    parsed.swap(parsedEven);

    if (parsed[0].first == NOTFOUND){
      return searchOneCharacter(query, res); // One character only
    }
  }

  vector<pair<size_t, uint32_t> > ord;
  for (size_t i = 0; i < parsed.size(); ++i){
    uint32_t id = parsed[i].first;
    uint32_t num = posList[id].size() + cPosList[id].size() * BLOCKSIZE;
    ord.push_back(make_pair(num, i));
  }
  sort(ord.begin(), ord.end());

  vector<uint32_t> cand;
  for (size_t i = 0; i < ord.size(); ++i){
    merge(parsed[ord[i].second], cand);
    if (cand.size() == 0) return;
  }

  decodeDoc(cand, res);
}

void InvertedFile::searchOneCharacter(const vector<uint8_t>& query, vector<SeResult>& res){
  uint64_t query_i = 0;
  for (size_t i = 0; i < query.size(); ++i){
    query_i <<= 8;
    query_i += query[i];
  }
  map<uint64_t, uint32_t>::const_iterator beg = iterm2id.lower_bound(query_i << 32);

  vector<uint32_t> poses;
  for (map<uint64_t, uint32_t>::const_iterator it = beg;
       it != iterm2id.end(); ++it){
    if ((it->first >> 32) > query_i){
      break;
    }
    for (size_t i = 0; i < posList[it->second].size(); ++i){
      poses.push_back(posList[it->second][i]);
    }
  }

  sort(poses.begin(), poses.end());

  decodeDoc(poses, res);
}

void InvertedFile::merge(const pair<uint32_t, uint32_t> qid, vector<uint32_t>& cand){
  const vector<uint32_t>& v(posList[qid.first]);
  const vector<CompressedBlock*>& cb(cPosList[qid.first]);
  const vector<uint32_t>& last(blockFront[qid.first]);
  const uint32_t offset = qid.second;
  
  if (cand.size() == 0){
    cand.resize(v.size() + cb.size() * BLOCKSIZE);
    size_t ind = 0;
    for (size_t i = 0; i < cb.size(); ++i){
      cb[i]->decode(buf);
      copy(buf.begin(), buf.end(), cand.begin() + ind);
      ind += BLOCKSIZE;
    }
    copy(v.begin(), v.end(), cand.begin() + ind);
    for (size_t i = 0; i < cand.size(); ++i){
      cand[i] -= offset;
    }

    return;
  }

  // search in compressed blocks
  vector<uint32_t> nextCand;
  size_t cand_i = 0;
  for ( ; cand_i < cand.size(); ){
    vector<uint32_t>::const_iterator it = lower_bound(last.begin(), 
						      last.end(), 
						      cand[cand_i] +offset);
    if (it == last.end()) break;
    cb[it - last.begin()]->decode(buf);

    size_t buf_ind = 0;
    while (buf_ind < buf.size()){
      vector<uint32_t>::const_iterator it = 
	lower_bound(buf.begin() + buf_ind, buf.end(), cand[cand_i] + offset);
      if (it == buf.end()) break;
      if (*it == cand[cand_i] + offset){
	nextCand.push_back(cand[cand_i]);
      }
      buf_ind = it - buf.begin() + 1;
    }
    cand_i++;
  }

  size_t ind = 0;
  for ( ; cand_i < cand.size(); ++cand_i){
    vector<uint32_t>::const_iterator it = 
      lower_bound(v.begin() + ind, v.end(), cand[cand_i] + offset);
    if (it == v.end()) break;
    if (*it == cand[cand_i] + offset){
      nextCand.push_back(cand[cand_i]);
    }
    ind = it - v.begin();
  }
  
  cand.swap(nextCand);
}



int InvertedFile::save(const char* fileName){
  ofstream ofs(fileName);
  if (!ofs){
    what_ << "cannot open " << fileName;
    return -1;
  }

  IndexType it = ONEGRAM;
  if (pt == C_ONEGRAM){
    it = ONEGRAM; 
  } else if (pt == C_TWOGRAM){
    it = TWOGRAM;
  } else if (pt == SEPARATED){
    it = INVERTEDFILE;
  } else {
    what_ << "Unknown Parsing Method" << endl;
    return -1;
  }

  if (write(it,         "IndexType", ofs) == -1) return -1;
  if (write(pt,         "parseType", ofs) == -1) return -1;
  if (write(cm,         "compressMethod", ofs) == -1) return -1;
  if (write(text,       "text", ofs) == -1) return -1;
  if (write(docOffsets, "docOffset", ofs) == -1) return -1;
  if (write(titles,     "titles", ofs) == -1) return -1;
  if (write(id2term,    "id2term", ofs) == -1) return -1;
  if (write(id2iterm,   "id2iterm", ofs) == -1) return -1;
  if (write(posList,    "posList", ofs) == -1) return -1;

  if (write(blockFront, "blockFront", ofs) == -1) return -1;
  uint32_t size = static_cast<uint32_t>(cPosList.size());
  if (write(size, "cPosList", ofs) == -1) return -1;
  for (uint32_t i = 0; i < size; ++i){
    uint32_t ssize = static_cast<uint32_t>(cPosList[i].size());
    if (write(ssize, "cPosList[i]", ofs) == -1) return -1;
    for (uint32_t j = 0; j < ssize; ++j){
      if (cPosList[i][j]->save(ofs) == -1){
	what_ << "cPosList write error " << i << " " << j;
	return -1;
      }
    }
  }

  if (write(cPosList,   "cPosList", ofs) == -1) return -1;

  return 0;
}

int InvertedFile::load(const char* fileName){
  ifstream ifs(fileName);
  if (!ifs){
    what_ << "cannot open " << fileName;
    return -1;
  }

  IndexType it = ONEGRAM;

  if (read(it,   "indexType", ifs) == -1) return -1;
  if (read(pt,   "parseType", ifs) == -1) return -1;
  if (read(cm,   "compressMethod", ifs) == -1) return -1;

  if (read(text, "text", ifs) == -1) return -1;
  if (read(docOffsets, "docOffsets", ifs) == -1) return -1;
  if (read(titles, "titles", ifs) == -1) return -1;
  if (read(id2term, "id2term", ifs) == -1) return -1;
  if (read(id2iterm, "id2iterm", ifs) == -1) return -1;
  if (read(posList, "posList", ifs) == -1) return -1;

  if (read(blockFront, "blockFront", ifs) == -1) return -1;


  uint32_t size = 0;
  if (read(size, "cPosList", ifs) == -1) return -1;
  cPosList.resize(size);
  for (uint32_t i = 0; i < size; ++i){
    uint32_t ssize = 0;
    if (read(ssize, "cPosList", ifs) == -1) return -1;
    cPosList[i].resize(ssize);
    for (uint32_t j = 0; j < ssize; ++j){
      if (cm == VARBYTE){
	cPosList[i][j] = new VarByte;
      } else if (cm == RICECODE){
	cPosList[i][j] = new RiceCode;
      } else {
	what_ << "Unkwnon Compress Method";
	return -1;
      }
      if (cPosList[i][j]->load(ifs) == -1){
	what_ << "cPosList read error " << i << " " << j;
	return -1;
      }
    }
  }

  assert(posList.size() == cPosList.size());
  assert(posList.size() == blockFront.size());

  for (size_t i = 0; i < id2term.size(); ++i){
    term2id[id2term[i]] = static_cast<uint32_t>(i);
  }

  for (size_t i = 0; i < id2iterm.size(); ++i){
    iterm2id[id2iterm[i]] = static_cast<uint32_t>(i);
  }

  docN = static_cast<uint32_t>(docOffsets.size())-1;
  termN = static_cast<uint32_t>(posList.size());
  return 0;
}

int InvertedFile::build() {
  return 0;
}

string InvertedFile::getIndexName() const{
  string name = "";
  if (pt == C_ONEGRAM) {
    name = "1-gram";
  } else if (pt == C_TWOGRAM){
    name = "2-gram";
  } else if (pt == SEPARATED) {
    name = "Inverted File";
  } else {
    name = "Unknown";
  }
  if (cm == NONE){

  } else if (cm == VARBYTE){
    name += " VarByte";
  } else if (cm == RICECODE){
    name += " RiceCode";
  } else {
    name += " Unknown";
  }
  return name;
}

size_t InvertedFile::getIndexSize() const {
  size_t ret = Minise::getIndexSize(); // Calculate Basic Class Size()
  for (size_t i = 0; i < posList.size(); ++i){
    ret += posList[i].size() * sizeof(uint32_t);
    ret += blockFront[i].size() * sizeof(uint32_t);
    for (size_t j = 0; j < cPosList[i].size(); ++j){
      ret += cPosList[i][j]->size();
    }
  }
  return ret;
}

}

