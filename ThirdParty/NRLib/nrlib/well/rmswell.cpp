// $Id: rmswell.cpp 1194 2013-08-19 08:24:58Z anner $

// Copyright (c)  2011, Norwegian Computing Center
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// •  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// •  Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "rmswell.hpp"
#include "../iotools/stringtools.hpp"
#include "../iotools/fileio.hpp"

using namespace NRLib;


RMSWell::RMSWell(const std::string& filename)
{
  std::ifstream file;
  OpenRead(file, filename);

  size_t nlog;
  std::string dummy;
  getline(file,line1_);
  getline(file,line2_);
  int line = 0;
  std::string token;
  std::string wellName = ReadNext<std::string>(file, line);
  SetWellName(wellName);
  xpos0_ = ReadNext<double>(file, line);
  ypos0_ = ReadNext<double>(file, line);      // read wellname and positions
 // getline(file,dummy);// Line may contain a dummy number
  //-----line shift
  DiscardRestOfLine(file, line, false);
  nlog = ReadNext<int>(file, line);          // read number of logs
  DiscardRestOfLine(file, line, false);
  lognames_.resize(nlog+3);
  lognames_[0] = "X";
  lognames_[1] = "Y";
  lognames_[2] = "Z";

  int ident;
  std::string identstr;
  size_t j(0);
  size_t k(0);
  isDiscrete_.resize(nlog+3);
  unit_.resize(nlog);
  scale_.resize(nlog);
  isDiscrete_[0] = false;
  isDiscrete_[1] = false;
  isDiscrete_[2] = false;
  for (size_t i = 0; i < nlog; i++) {
    getline(file,dummy);
    std::istringstream ist(dummy);
    lognames_[i+3] = NRLib::Uppercase(ReadNext<std::string>(ist, line));

    //token = ReadNext<std::string>(ist, line); //H Error if line only containts log-name (test_case 19).
    ReadNextToken(ist, token, line);

    if (token != "") {
      if (token=="DISC") { // discrete log
        isDiscrete_[i+3] = true;
        std::map<int, std::string> disc;
        while(ReadNextToken(ist,token,line)) {
          ident = ParseType<int>(token);
          identstr = ReadNext<std::string>(ist, line);
          disc.insert(std::pair<int, std::string> (ident, identstr));
        }
        discnames_[lognames_[i+3]] = disc;
        j++;
      }
      else {
        isDiscrete_[i+3] = false;
        unit_[k] = token;
        scale_[k] = ReadNext<std::string>(ist, line);
        k++;
      }
    }
    else {
      isDiscrete_[i+3] = false;
      k++;
    }
  }

  size_t ndisc(j);
  size_t ncont = nlog + 3 - ndisc;
  std::vector<std::vector<int> > disclogs(ndisc);
  std::vector<std::vector<double> > contlogs(ncont);

  int count = 0;

  while(NRLib::CheckEndOfFile(file)==false && getline(file,dummy)) {
    count ++;
    std::istringstream ist(dummy);
    contlogs[0].push_back(ReadNext<double>(ist, line)); //x
    contlogs[1].push_back(ReadNext<double>(ist, line)); //y
    contlogs[2].push_back(ReadNext<double>(ist, line)); //z

    j = 0;
    k = 3;
    for (size_t i = 0; i < nlog; i++) {
      if (isDiscrete_[i+3]) {
        double dummy = ReadNext<double>(ist, line); //Double because of a problem with ReadNext<int> and facies on the form -9.9900000e+002
        if(IsMissing(dummy) == false)
          disclogs[j].push_back(static_cast<int>(dummy));
        else
          disclogs[j].push_back(GetIntMissing());

         j++;
      }
      else {
        contlogs[k].push_back(ReadNext<double>(ist, line));
        k++;
      }
    }
  }

  AddContLog(lognames_[0], contlogs[0]);
  AddContLog(lognames_[1], contlogs[1]);
  AddContLog(lognames_[2], contlogs[2]);
  j = 0;
  k = 3;
  for (size_t i = 0; i < nlog; i++) {
   if (isDiscrete_[i+3]) {
     AddDiscLog(lognames_[i+3], disclogs[j]);
     j++;
   }
   else {
     AddContLog(lognames_[i+3], contlogs[k]);
     k++;
   }
  }

  // Find n_data including WELLMISSING values
  int n_data = static_cast<int>(GetContLog(lognames_[0]).size());

  this->SetNumberOfData(n_data);
  SetXPos0(xpos0_);
  SetYPos0(ypos0_);
}


RMSWell::RMSWell(const Well& wellobj)
  : Well(wellobj)
{
  xpos0_ = GetContValue(0, "x");
  ypos0_ = GetContValue(0, "y");
  line1_ = "1.0";
  line2_ = "Undefined";
  size_t nlog = wellobj.GetNlog();
  lognames_.resize(nlog + 3);
  lognames_[0] = "x";
  lognames_[1] = "y";
  lognames_[2] = "z";
  isDiscrete_.resize(nlog + 3);
  isDiscrete_[0] = false;
  isDiscrete_[1] = false;
  isDiscrete_[2] = false;
  size_t i = 0;
  size_t k = 0;

  typedef std::map<std::string, std::vector<int> >::const_iterator CI;
  std::map<std::string,std::vector<int> > disclog = wellobj.GetDiscLog();
  for (CI p = disclog.begin(); p != disclog.end(); ++p) {
    lognames_[i+3] = p->first;
    isDiscrete_[i+3] = true;
    i++;
  }
  typedef std::map<std::string, std::vector<double> >::const_iterator CID;
  std::map<std::string,std::vector<double> > contlog = wellobj.GetContLog();
  for (CID p = contlog.begin(); p != contlog.end(); ++p) {
    if (p->first != "x" && p->first != "y" && p->first != "z") {
      lognames_[i+3] = p->first;
      isDiscrete_[i+3] = false;
      i++;
      k++;
    }
  }

  unit_.resize(k);
  scale_.resize(k);
  for (i = 0; i < k; i++) {
    unit_[i] = "unit1";
    scale_[i] ="scale1";
  }

  size_t ndisc = nlog - k - 3;
  std::vector<std::vector<int> > discvalues(ndisc);
  i = 0;
  size_t j;
  size_t kk = 0;
  bool found = false;
  for (CI p = disclog.begin(); p != disclog.end(); ++p) {
    while(wellobj.IsMissing((p->second)[kk]))
      kk++;
    discvalues[i].push_back((p->second)[kk]);

    for (j = kk + 1; j < p->second.size(); j++) {
      for (k = 0; k < discvalues[i].size(); k++)
        if ((p->second)[j] == discvalues[i][k])
          found = true;
      if (found == false && (!wellobj.IsMissing((p->second)[j])))
        discvalues[i].push_back((p->second)[j]);
      found = false;
    }
  }
  ////Frode NB: Må se nøyere på dette.
  typedef std::pair<int, std::string> pp;
  std::string identstr;
  for (CI p = disclog.begin(); p !=disclog.end(); ++p) {
    std::string disc_name = p->first;
    for (j = 0; j < p->second.size(); j++) {
      std::string identstr = ToString(p->second[j]);
      discnames_[disc_name].insert(pp(p->second[j], identstr));
    }
  }
}

const std::map<int, std::string>
RMSWell::GetDiscNames(const std::string& log_name) const
{
  return (discnames_.find(log_name)->second);
}

void RMSWell::WriteToFile(const std::string& filename)
{
  std::ofstream file;
  OpenWrite(file, filename, std::ios::out);

  file << line1_ << "\n"
    << line2_ << "\n"
    << std::setprecision(8)
    << GetWellName() << " " << xpos0_ <<" " << ypos0_ << "\n"
    << GetNlog() - 3 << "\n";

  size_t k = 0;
  std::map<std::string, std::map<int, std::string> >::iterator iter = discnames_.begin();
  typedef std::map<int, std::string>::const_iterator ci;
  for (size_t i = 0; i < GetNlog(); i++) {
    if (lognames_[i] != "x" && lognames_[i] != "y" && lognames_[i] != "z") {
      file<< lognames_[i] << " ";
      iter = discnames_.find(lognames_[i]);
      if (isDiscrete_[i]==true) {
        file << "DISC" << " ";
        for (ci p = iter->second.begin(); p !=iter->second.end();++p)
          file << p->first << " " << p->second << " ";
        file << "\n";
       // ++iter;
      }
      else {
        file<< unit_[k] << " " << scale_[k] <<"\n";
        k++;
      }
    }
  }
  file.precision(8);
  size_t n = GetContLogLength(lognames_[0]);
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < GetNlog(); j++) {
      if (isDiscrete_[j]==true)
        file << GetDiscValue(i, lognames_[j]) << " ";
      else
        file << GetContValue(i, lognames_[j]) << " ";
    }
    file << "\n";
  }
  file.close();
}
