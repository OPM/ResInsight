// $Id: norsarwell.cpp 882 2011-09-23 13:10:16Z perroe $

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
#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "norsarwell.hpp"
#include "../iotools/stringtools.hpp"
#include "../iotools/fileio.hpp"
//#include "src/definitions.h"

using namespace NRLib;

NorsarWell::NorsarWell(const std::string & filename)
  : header_units_(8)
{
  std::ifstream file;
  NRLib::OpenRead(file, filename);
  std::string token;
  getline(file, token);
  if(token.find("[Version information]",0) > 0)
    throw(FileFormatError("Found '"+token+"' instead of '[Version information]' on first line in file "+filename+"."));

  int line = 2;
  ReadNext<std::string>(file, line);
  version_ = ReadNext<std::string>(file, line);
  if(version_ != "1000")
    throw(FileFormatError("Found '"+version_+"' as version number in file "+filename+". Can only read version 1000."));

  ReadNext<std::string>(file, line);
  format_ = ReadNext<std::string>(file, line);
  if(format_ != "ASCII")
    throw(FileFormatError("Found '"+format_+"' as format in file "+filename+". Can only read ASCII format."));

  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line); //Together with previous, reads '[Well information]'

  ReadNext<std::string>(file, line); //'MDMIN'
  header_units_[0] = ReadNext<std::string>(file, line); //unit for MDMIN
  md_min_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'MDMAX'
  header_units_[1] = ReadNext<std::string>(file, line); //unit for MDMAX
  md_max_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'MDMINSTEP'
  header_units_[2] = ReadNext<std::string>(file, line); //unit for MDMINSTEP
  md_min_step_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'MDMAXSTEP'
  header_units_[3] = ReadNext<std::string>(file, line); //unit for MDMAXSTEP
  md_max_step_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'UTMX'
  header_units_[4] = ReadNext<std::string>(file, line); //unit for UTMX
  xpos0_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'UTMY'
  header_units_[5] = ReadNext<std::string>(file, line); //unit for UTMY
  ypos0_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'EKB'
  header_units_[6] = ReadNext<std::string>(file, line); //unit for EKB
  ekb_ = ReadNext<double>(file, line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'UNDEFVAL'
  header_units_[7] = ReadNext<std::string>(file, line); //unit for UNDEFVAL
  SetMissing(ReadNext<double>(file, line));
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line); //Together with previous, reads '[Well track data information]'

  ReadNext<std::string>(file, line); //'NUMMD'
  int n_track = ReadNext<int>(file,line);
  DiscardRestOfLine(file, line, false);

  ReadNext<std::string>(file, line); //'NUMPAR'
  int n_track_par = ReadNext<int>(file,line);
  DiscardRestOfLine(file, line, false);

  typedef std::pair<std::string,std::string> unitpair;

  int track_MD = -1;
  std::string name;
  std::vector<std::string> log_name;
  for(int i=0; i<n_track_par;i++) {
    name = ReadNext<std::string>(file, line); //parameter
    log_name.push_back(NRLib::Uppercase(name));
    if(name == "MD")
      track_MD = i;
    token = ReadNext<std::string>(file, line); //unit
    units_.insert(unitpair(name,token));

    DiscardRestOfLine(file, line, false);
  }
  if(track_MD < 0)
    throw(FileFormatError("Could not find MD for track file in file '"+filename+"'."));

  std::string track_filename = ReplaceExtension(filename, ".nwt");
  std::vector<std::vector<double> > track_logs = ReadLogs(track_filename, n_track_par, n_track,1);

  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line);
  ReadNext<std::string>(file, line); //Together with previous, reads '[Well log data information]'

  std::string path = GetPath(filename);
  std::string log_filename;
  std::string log_section_name;
  std::vector<std::vector<double> > log_logs;
  int n_log, n_log_par, log_MD;
  while(CheckEndOfFile(file) == false) {
    ReadNext<std::string>(file, line);
    log_section_name = ReadNext<std::string>(file, line);
    ReadNext<std::string>(file, line);
    log_filename = ReadNext<std::string>(file, line);

    ReadNext<std::string>(file, line); //'NUMMD'
    n_log = ReadNext<int>(file,line);
    DiscardRestOfLine(file, line, false);

    ReadNext<std::string>(file, line); //'NUMPAR'
    n_log_par = ReadNext<int>(file,line);
    DiscardRestOfLine(file, line, false);

    log_MD = -1;
    for(int i=0; i<n_log_par;i++) {
      name = ReadNext<std::string>(file, line); //parameter
      if(name == "MD")
        log_MD = i;
      else
        log_name.push_back(NRLib::Uppercase(name));
      token = ReadNext<std::string>(file, line); //unit
      if(log_MD != i)
        units_.insert(unitpair(name,token));
      DiscardRestOfLine(file, line, false);
    }
    if(log_MD < 0)
      throw(FileFormatError("Could not find MD for logs '"+log_section_name+"' in file '"+filename+"'."));

    log_filename = PrependDir(path, log_filename);
    log_logs = ReadLogs(log_filename, n_log_par, n_log, 2);

    track_logs = MergeLogs(track_logs, track_MD, log_logs, log_MD);
  }

  file.close();

  for(int i=0;i<static_cast<int>(track_logs.size());i++)
    AddContLog(log_name[i], track_logs[i]);

  // find n_data including WELLMISSING values
  int n_data = static_cast<int>(GetContLog(log_name[0]).size());
  SetNumberOfData(n_data);
  SetXPos0(xpos0_);
  SetYPos0(ypos0_);
}


std::vector<std::vector<double> >
NorsarWell::ReadLogs(const std::string & filename, int n_col, int n_row, int skip_lines)
{
  std::ifstream file;
  OpenRead(file, filename);
  std::vector<std::vector<double> > result(n_col, std::vector<double>(n_row, 0));

  int line = 1;
  std::string token;
  for(int i=0;i<skip_lines;i++)
    DiscardRestOfLine(file, line, false);

  int baseline = line;
  int i,j; //For use in error message.

  //int legal_data = 0;

  try {
    for(i=0;i<n_row;i++) {
      for(j=0;j<n_col;j++)
      {
        result[j][i] = ReadNext<double>(file, line);
        if(line-i > baseline) {
          std::string error = "Too few elements on line "+NRLib::ToString(line-1)+" in file "+filename+": Expected to read "+NRLib::ToString(n_col)+" elements, found only "+NRLib::ToString(j)+".";
          throw (NRLib::IOError(error));
        }
        else if(line-i < baseline) {
          while(line-i < baseline) {
            j++;
            ReadNext<double>(file, line);
          }
          std::string error = "Too many elements on line "+NRLib::ToString(line-1)+" in file "+filename+": Expected to read "+NRLib::ToString(n_col)+" elements, found "+NRLib::ToString(n_col+j)+".";
          throw (NRLib::IOError(error));
        }
      }
      //if(result[0][i] != WELLMISSING) //H [0] First?
      //  legal_data++;
    }
  }
  catch (NRLib::EndOfFile) {
    std::string error = "Unexpected end of file "+filename+": Expected to read "+NRLib::ToString(n_row*n_col)+"elements, found only "+NRLib::ToString(i*n_col+j)+".";
    throw (NRLib::IOError(error));
  }

  //this->SetNumberOfLegalData(legal_data);

  return(result);
}


std::vector<std::vector<double> >
NorsarWell::MergeLogs(const std::vector<std::vector<double> > & track_logs, int track_MD,
                      const std::vector<std::vector<double> > & log_logs, int log_MD)
{
  int nt = static_cast<int>(track_logs[0].size());
  int nl = static_cast<int>(log_logs[0].size());
  int n = nt;
  if(nl > nt)
    n = nl; //Minimum number of rows.
  int nc = static_cast<int>(track_logs.size() + log_logs.size()) - 1;
  std::vector<std::vector<double> > result(nc);
  for(int i = 0; i < nc; i++) {
    result[i].reserve(n);
  }

  double depth_track, depth_log;
  int ntl = static_cast<int>(track_logs.size());
  int nll = static_cast<int>(log_logs.size());
  int it = 0; //track counter
  int il = 0; //log counter

  while(it < nt && il < nl) {
    depth_track = track_logs[track_MD][it];
    depth_log   = log_logs[log_MD][il];
    if(depth_track == depth_log) { //Lucky.
      for(int i=0;i<ntl;i++)
        result[i].push_back(track_logs[i][it]);
      int mdp = 0; //Indicate whether md log is passed. This one is discarded, so index adjustment needed.
      for(int i=0;i<nll;i++) {
        if(i != log_MD)
          result[i-mdp+ntl].push_back(log_logs[i][il]);
        else
          mdp = 1;
      }
      it++;
      il++;
    }
    else if(depth_track < depth_log) {
      for(int i=0;i<ntl;i++)
        result[i].push_back(track_logs[i][it]);
      for(int i=0;i<nll-1;i++)
        result[i+ntl].push_back(GetContMissing());
      it++;
    }
    else {
      for(int i=0;i<ntl;i++) {
        if(i != track_MD)
          result[i].push_back(GetContMissing());
        else
          result[i].push_back(depth_log); //Always log MD, must take form log instead of track here.
      }
      int mdp = 0; //Indicate whether md log is passed. This one is discarded, so index adjustment needed.
      for(int i=0;i<nll;i++) {
        if(i != log_MD)
          result[i-mdp+ntl].push_back(log_logs[i][il]);
        else
          mdp = 1;
      }
      il++;
    }
  }

  for(;it<nt;it++) {
    for(int i=0;i<ntl;i++)
      result[i].push_back(track_logs[i][it]);
    for(int i=0;i<nll-1;i++)
      result[i+ntl].push_back(GetContMissing());
  }

  for(;il<nl;il++) {
    for(int i=0;i<ntl;i++) {
      if(i != track_MD)
        result[i].push_back(GetContMissing());
      else
        result[i].push_back(log_logs[log_MD][il]); //Always log MD, must take form log instead of track here.
    }
    int mdp = 0; //Indicate whether md log is passed. This one is discarded, so index adjustment needed.
    for(int i=0;i<nll;i++) {
      if(i != log_MD)
        result[i-mdp+ntl].push_back(log_logs[i][il]);
      else
        mdp = 1;
    }
    il++;
  }

  return(result);
}


std::string
NorsarWell::GetLogUnit(const std::string & name)
{
  std::map<std::string,std::string>::iterator item = units_.find(name);
  if(item == units_.end())
    return("");
  else
    return(item->second);
}
