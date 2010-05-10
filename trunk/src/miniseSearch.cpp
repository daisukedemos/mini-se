/*
 * miniseSearch.cpp
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

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include "minise.hpp"
#include "cmdline.h"
#include "timer.hpp"

using namespace std;
using namespace SE;
using namespace cmdline;

void removeNL(string& snippet){
  for (size_t i = 0; i < snippet.size(); ++i){
    if (snippet[i] == '\n' || snippet[i] == '\r'){
      snippet[i] = ' ';
    }
  }
}

void printResult(const Minise* ms, const vector<SeResult>& ret, 
		 const int num, const int snum, const int slen){
  size_t total = 0;
  for (size_t i = 0; i < ret.size(); ++i){
    total += ret[i].offsets.size();
  }
  cout << "Hit " << ret.size() << " documents. " << total << " positions." << endl;
  for (int i = 0; i < num && i < (int)ret.size(); ++i){
    const SeResult& sr(ret[i]);
    cout << " Title: " << sr.title << endl;
    cout << " DocID: " << sr.docID << endl;
    cout << "HitPos: " << sr.offsets.size() << endl;
    for (int j = 0; j < (int)sr.offsets.size() && j < snum; ++j){
      string snippet;
      ms->getSnippet(sr.docID, sr.offsets[j], slen, snippet);
      removeNL(snippet);
      cout << setw(10) << sr.offsets[j] << "\t" << snippet << "\t" << endl;
    }
    cout << endl;
  }
  
}

int searchIndex(const string& index, const int num, 
		const int snum, const int slen, const string& usage){
  Minise::IndexType indexType = Minise::QUICKSEARCH;
  if(getIndexType(index.c_str(), indexType) == -1){
    cerr << "searchIndex read error: " << index << endl;
    return -1;
  }

  Minise* ms = NULL;
  if (indexType == Minise::QUICKSEARCH){
    ms = new QuickSearch;
  } else if (indexType == Minise::ONEGRAM ||
	     indexType == Minise::TWOGRAM ||
	     indexType == Minise::INVERTEDFILE){
    ms = new InvertedFile;
  } else if (indexType == Minise::SUFFIXARRAY ||
	     indexType == Minise::SUFFIXARRAY_UTF8){
    ms = new SuffixArray;
  } else {
    cerr << "indexType:" << indexType << endl;
    return -1;
  }

  if (ms->load(index.c_str()) == -1){
    cerr << ms->what() << endl;
    return -1;
  }
  
  cout << "method: " << ms->getIndexName() << endl
       << "  docN: " << ms->getDocN() << endl
       << " index: " << index << endl
       << " termN: " << ms->getTermN() << endl
       << "  size: " << ms->getIndexSize() << endl;

  string query;
  for (;;){
    cout << ">";
    if (!getline(cin, query)) break;
    
    cout << "query:[" << query << "]" << endl;
    vector<SeResult> ret;
    double start = gettimeofday_sec();
    ms->search(query.c_str(), query.size(), ret);
    cout << "time: " << (gettimeofday_sec() - start) * 1000 << " milli seconds." << endl;
    printResult(ms, ret, num, snum, slen);
  }

  delete ms;
  return 0;
}

int main(int argc, char* argv[]){
  parser p;
  p.set_progam_name(string("minise_search"));
  p.add<string>("index", 'i', "Index file ", true);
  p.add<int>("num", 'n', "Result Num ", false, 5);
  p.add<int>("snippetnum", 's', "Snippet Num ", false, 3);
  p.add<int>("snippetlen", 'l', "Snippet Length ", false, 60);
  p.add("help", 'h', "Print help");
  
  if (!p.parse(argc, argv)){
    if (argc == 1 || p.exist("help")){
      cerr << p.usage() << endl;
    } else {
      cerr << p.error() << p.usage() << endl;
    }
    return -1;
  }

  if (searchIndex(p.get<string>("index"), 
		  p.get<int>("num"),
		  p.get<int>("snippetnum"),
		  p.get<int>("snippetlen"), 
		  p.usage()) == -1){
    return -1;
  }

  return 0;
}
