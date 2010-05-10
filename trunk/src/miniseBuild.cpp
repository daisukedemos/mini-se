/*
 * miniseBuild.cpp
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
#include "minise.hpp"
#include "cmdline.h"
#include "timer.hpp"

using namespace std;
using namespace SE;
using namespace cmdline;

Minise* initMinise(const string& method, const string& cm_s){
  Minise* ms = NULL;
  InvertedFile::compressMethod cm = InvertedFile::NONE;
  if (cm_s == "none"){
    cm = InvertedFile::NONE;
  } else if (cm_s == "vb"){
    cm = InvertedFile::VARBYTE;
  } else if (cm_s == "rc"){
    cm = InvertedFile::RICECODE;
  } else {
    cerr << "Unkwnon compress method : " << cm_s << endl;
    return ms;
  }

  if (method == "inv"){
    ms = new InvertedFile;
    ms ->setParseType(Minise::SEPARATED);
    static_cast<InvertedFile*>(ms)->setCompressMethod(cm);
  } else if (method == "seq") {
    ms = new QuickSearch;
  } else if (method == "1gram") {
    ms = new InvertedFile;
    ms->setParseType(Minise::C_ONEGRAM);
    static_cast<InvertedFile*>(ms)->setCompressMethod(cm);
  } else if (method == "2gram") {
    ms = new InvertedFile;
    ms->setParseType(Minise::C_TWOGRAM);
    static_cast<InvertedFile*>(ms)->setCompressMethod(cm);
  } else if (method == "sa"){
    ms = new SuffixArray;
  } else if (method == "sa8"){
    ms = new SuffixArray;
    static_cast<SuffixArray*>(ms)->setUTF8();
  } else {
    // Nothing
  }
  return ms;
}

int buildIndex(const parser& p){
  string method = p.get<string>("method");
  string list   = p.get<string>("list");
  string index  = p.get<string>("index");
  string cm_s   = p.get<string>("compress");
  string usage  = p.usage();


  Minise* ms = initMinise(method, cm_s);
  if (ms == NULL){
    cerr << usage << endl;
    return -1;
  }

  ifstream ifs(list.c_str());
  if (!ifs){
    cerr << "cannot open " << list << endl;
    return -1;
  }

  vector<string> files;
  string file;
  while (getline(ifs, file)){
    files.push_back(file);
  }

  cout << "method: " << ms->getIndexName() << endl
       << "  docN: " << files.size() << endl
       << " index: " << index << endl;

  double start = gettimeofday_sec();
  string fileName;
  for (size_t i = 0; i < files.size(); ++i){
    if (ms->addFile(files[i].c_str()) == -1){
      cerr << ms->what() << endl;
      delete ms;
      return -1;
    }
    if (((i+1) % 1000) == 0){
      cout << i+1 << "\r" << flush;
    }
  }

  cout << "build..." << flush;
  if (ms->build() == -1){
    cerr << ms->what() << endl;
    delete ms;
    return -1;
  }
  cout << "\r termN: " << ms->getTermN() << endl;
  
  cout << " save...";
  if (ms->save(index.c_str()) == -1){
    cerr << ms->what() << endl;
    delete ms;
    return -1;
  }

  double etime = gettimeofday_sec() - start;
  cout << "\r  time: " << etime << " sec." << endl;
  cout << "  size: " << ms->getIndexSize() << " bytes." << endl;
  cout << "build finish." << endl;

  delete ms;

  return 0;
}

int main(int argc, char* argv[]){
  parser p;
  p.set_progam_name(string("minise_build"));
  p.add<string>("method", 'm', "Index method: (seq|inv|1gram|2gram|sa|sa8) ", false, "1gram");
  p.add<string>("list", 'l', "File list ", true);
  p.add<string>("index", 'i', "Index file ", true);
  p.add<string>("compress", 'c', "Compress method: (none|vb|rc)  for inv, 1gram, 2gram ", false, "none");
  p.add("help", 'h', "Print help");
  
  if (!p.parse(argc, argv)){
    if (argc == 1 || p.exist("help")){
      cerr << p.usage() << endl;
    } else {
      cerr << p.error() << p.usage() << endl;
    }
    return -1;
  }

  if (buildIndex(p) == -1){
    return -1;
  }

  return 0;
}
