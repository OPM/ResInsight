// $Id: laswell.cpp 1245 2014-02-25 09:57:02Z hauge $

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

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

#include "laswell.hpp"

#include "../iotools/fileio.hpp"
#include "../iotools/stringtools.hpp"

using namespace NRLib;

LasWell::LasWell() :
  version_("Unknown"),
  wrap_(false),
  comma_delimited_(false),
  depth_unit_("Unknown"),
  start_depth_(0),
  stop_depth_(0),
  depth_increment_(0)
{
}

LasWell::LasWell(const std::string & filename) :
  version_("Unknown"),
  wrap_(false),
  comma_delimited_(false),
  depth_unit_("Unknown"),
  start_depth_(0),
  stop_depth_(0),
  depth_increment_(0)
{
  std::string err_txt;
  std::ifstream fin;
  try {
    ReadHeader(filename, fin, err_txt);
    if(err_txt == "") {
      if(fin.eof() == false)
        ReadLogs(fin, err_txt);
      else
        err_txt = "No log data found in well "+GetWellName()+"\n";
    }
  }
  catch(Exception & e) {
    err_txt += e.what();
  }
  if(fin.is_open())
    fin.close();
  if(err_txt != "")
    throw(FileFormatError(err_txt));
  else { //Final initialisation of unset values
    if(stop_depth_ == 0)
      stop_depth_ = GetContMissing();
    if(depth_increment_ == 0)
      depth_increment_ = GetContMissing();
  }
  const std::map<std::string,std::vector<double> > & logs = GetContLog();
  std::map<std::string,std::vector<double> >::const_iterator it = logs.begin();
  int n_data = static_cast<int>(it->second.size());
  this->SetNumberOfData(n_data);
}

void
LasWell::ReadHeader(const std::string        & filename,
                    std::ifstream            & fin,
                    std::string              & err_txt)
{
  std::string well_name = NRLib::GetStem(filename);
  SetWellName(well_name);

  fin.open(filename.c_str());

  if (fin.is_open()) {
    bool version_read = false;
    bool well_read = false;
    bool curve_read   = false;

    std::string line;
    getline (fin,line);                   // First log line

    bool end_of_header = false;

    while ( !fin.eof() && !end_of_header) {
      line = NRLib::Chomp(line);

      if (line.empty()) {                 // Empty line, do nothing
        getline (fin,line);
        continue;
      }
      else if (line[0] == '#') {         // Comment line, do nothing
        getline (fin,line);
        continue;
      }
      else if (line[0] == '~') {         // Command trigger
        if (line[1] == 'V') {
          if(version_read == true)
            err_txt += "~VERSION INFORMATION given more than once in file "+filename+".\n";
          else {
            version_read = true;
            ParseVersionInformation(fin, line, err_txt);
          }
        }
        else if (line[1] == 'W') {
          if(well_read == true)
            err_txt += "~WELL INFORMATION given more than once in file "+filename+".\n";
          else {
            well_read = true;
            ParseWellInformation(fin, line, err_txt);
          }
        }
        else if (line[1] == 'C') {
          if(curve_read == true)
            err_txt += "~CURVE INFORMATION given more than once in file "+filename+".\n";
          else{
            curve_read = true;
            ParseCurveInformation(fin, line, err_txt);
          }
        }
        else if (line[1] == 'P')
          ParseInformation(parameter_info_, "~PARAMETER INFORMATION", fin, line, err_txt);
        else if (line[1] == 'O')
          ParseInformation(other_info_, "~OTHER INFORMATION", fin, line, err_txt);
        else if (line[1] == 'A')
          end_of_header = true;
        else {
          err_txt += "Invalid keyword \'~"+NRLib::ToString(line[1])+"\' has been encountered in LAS file "+filename+".\n";
          std::vector<std::string> junk_info;
          ParseInformation(junk_info, "", fin, line, err_txt);
        }
      }
      else {
        err_txt = "Unexpected end of header in file " + filename + "\n";
        end_of_header = true;
      }
    }
    if(curve_read == false)
      err_txt += "No ~CURVE INFORMATION given in LAS file "+filename+".\n";
    if(well_read == false)
      err_txt += "No ~WELL INFORMATION given in LAS file "+filename+".\n";
    if(version_read == false)
      err_txt += "No ~VERSION INFORMATION given in LAS file "+filename+".\n";
  }
  else {
    err_txt = std::string(" Cannot open file ") + filename + std::string("\n");
  }
}


void
LasWell::ParseInformation(std::vector<std::string>               & info,
                          const std::string                      & text,
                          std::ifstream                          & fin,
                          std::string                            & line,
                          std::string                            & err_txt)
{
  if (info.size() == 0) {
    bool end_of_section = false;
    while ( !fin.eof() && !end_of_section && err_txt == "") {
      getline (fin,line);
      line = NRLib::Chomp(line);

      if (line.size() == 0 || line[0] == '#') {
          continue;
      }
      else if (line[0] == '~') {
        end_of_section = true;
        continue;
      }
      else {
        info.push_back(line);
      }
    }
  }
  else {
    err_txt += "There is more than one "+text+" section present in well "+GetWellName()+".\n";
  }
}

void
LasWell::ParseVersionInformation(std::ifstream & fin,
                                 std::string   & line,
                                 std::string   & err_txt)
{
  bool end_of_section = false;
  while (!fin.eof() && !end_of_section) {
    getline (fin,line);
    line = NRLib::Chomp(line);
    if(line.size() == 0 || line[0] == '#') {
      continue;
    }
    else if (line[0] == '~') {
      end_of_section = true;
      continue;
    }
    else {
      std::string::size_type end_pos = line.find_first_of(".");
      std::string token = NRLib::Chomp(line.substr(0, end_pos));
      std::string::size_type start_pos = end_pos;
      start_pos = line.find_first_of(" ",start_pos);
      end_pos   = line.find_first_of(":",start_pos);
      if(end_pos == std::string::npos)
        err_txt += "Not enough items under keyword "+token+" under ~VERSION INOFRMATION in "+GetWellName()+".\n";
      else {
        std::string value = NRLib::Chomp(line.substr(start_pos,end_pos-start_pos));
        if(token == "VERS")
          version_ = value;
        else if(token == "WRAP") {
          if(value == "NO")
            wrap_ = false;
          else
            wrap_ = true;
        }
        else if(token == "DLM") {
          if(value == "COMMA")
            comma_delimited_ = true;
        }
        else
          version_info_.push_back(line);
      }
    }
  }
}

void
LasWell::ParseWellInformation(std::ifstream & fin,
                              std::string   & line,
                              std::string   & err_txt)
{
  bool end_of_section = false;
  while (!fin.eof() && !end_of_section) {
    getline (fin,line);
    line = NRLib::Chomp(line);

    if(line.size() == 0 || line[0] == '#') {
      continue;
    }
    else if (line[0] == '~') {
      end_of_section = true;
      continue;
    }
    else {
      std::string::size_type end_pos = line.find_first_of(".");
      std::string token = NRLib::Chomp(line.substr(0, end_pos));
      std::string::size_type start_pos = end_pos;
      start_pos = line.find_first_of(" ",start_pos);
      end_pos   = line.find_first_of(":",start_pos);
      if(end_pos == std::string::npos)
        err_txt += "Not enough items under keyword "+token+" under ~WELL INOFRMATION in "+GetWellName()+".\n";
      else {
        std::string value = NRLib::Chomp(line.substr(start_pos,end_pos-start_pos));
        ParseWellToken(token, value, line, err_txt);
      }
    }
  }
}

void
LasWell::ParseWellToken(const std::string & token,
                        const std::string & value,
                        const std::string & line,
                        std::string       & err_txt)
{
  if(token == "WELL") {
    if(value.empty() == false)
      SetWellName(value);
  }
  else if (token == "DATE")
  {
    if (value.empty() == false)
      SetDate(value);
  }
  else if(token == "STRT" || token == "STOP" || token == "STEP" ||
          token == "NULL" || token == "XWELL" || token == "YWELL")
  {
    if(value.empty() == false) { //Do not set value if there is nothing to set. Do not trust missingcode, may change later.
      try {
        double val = NRLib::ParseType<double>(value);
        if(token == "STRT")
          start_depth_ = val;
        else if(token == "STOP")
          stop_depth_ = val;
        else if(token == "STEP")
          depth_increment_ = val;
        else if(token == "NULL")
          SetMissing(val);
        else if(token == "XWELL")
          SetXPos0(val);
        else if(token == "YWELL")
          SetYPos0(val);
      }
      catch(NRLib::Exception & e) {
        err_txt += std::string(e.what())+" for keyword "+token+" under ~WELL INFORMATION in well "+GetWellName();
      }
    }
  }
  else
    well_info_.push_back(line);

}

void
LasWell::ParseCurveInformation(std::ifstream            & fin,
                               std::string              & line,
                               std::string              & err_txt)
{
  bool end_of_section = false;
  while (!fin.eof() && !end_of_section) {
    getline (fin,line);
    line = NRLib::Chomp(line);

    if(line.size() == 0 || line[0] == '#') {
      continue;
    }
    else if (line[0] == '~') {
      end_of_section = true;
      continue;
    }
    else {
      std::string::size_type end_pos   = line.find_first_of(".");
      std::string token                = NRLib::Chomp(line.substr(0, end_pos));
      std::string::size_type start_pos = end_pos+1;
      end_pos                          = line.find_first_of(" ",start_pos);
      std::string unit                 = NRLib::Chomp(line.substr(start_pos, end_pos-start_pos));
      start_pos   = line.find_first_of(":",start_pos);
      if(start_pos == std::string::npos)
        err_txt += "Not enough items under keyword "+token+" under ~CURVE INOFRMATION in "+GetWellName()+".\n";
      else {
        log_name_.push_back(token);
        log_unit_.push_back(unit);
        logUnitMap_[token] = unit;
        std::string comment = NRLib::Chomp(line.substr(start_pos+1));
        log_comment_.push_back(comment);
      }
    }
  }
  if(log_unit_.size() > 0)
    depth_unit_ = log_unit_[0];
}


void
LasWell::ReadLogs(std::ifstream                  & fin,
                  std::string                    & err_txt)
{
  std::string tmp_err_txt;
  std::vector<std::vector<double> > log(log_name_.size());
  size_t n_data = 2000000000; //Anything more than this will easily give indexing problems, so ok upper limit.
  bool n_data_given = false;
  if(IsMissing(start_depth_) == false && IsMissing(stop_depth_) == false && IsMissing(depth_increment_) == false &&
     stop_depth_ > start_depth_ && depth_increment_ != 0) {
    n_data_given = true;
    n_data = static_cast<size_t>(floor(0.5+(stop_depth_ - start_depth_)/depth_increment_))+1;
    for(size_t i=0;i<log.size();i++)
      log[i].resize(n_data);
  }

  std::string line;
  size_t n_records = 0;
  size_t n_errors  = 0;
  std::vector<std::string> record;
  while(GetRecord(fin, log.size(), record) == true && n_errors < 5 && n_records < n_data) {
    n_records++;
    if(record.size() != log.size()) {
      n_errors++;
      tmp_err_txt += "Error in well "+GetWellName()+", record "+NRLib::ToString(n_records)+"("+log_name_[0]+"="+record[0]
        +"?): Wrong number of items, found "+NRLib::ToString(record.size())+" when expecting "+NRLib::ToString(log.size())+".\n";
    }
    else {
      for(size_t i=0;i<log.size();i++) {
        if(n_data_given == false)
          log[i].push_back(0);
        try {
          log[i][n_records-1] = NRLib::ParseType<double>(record[i]);
        }
        catch(Exception & e) {
          tmp_err_txt += "Error in well "+GetWellName()+", record "+NRLib::ToString(n_records)+"("+log_name_[0]+"="+record[0]
            +"?), item "+NRLib::ToString(i+1)+": "+std::string(e.what());
            n_errors++;
        }
      }
    }
  }
  while(GetRecord(fin, log.size(), record) == true) //Find actual record count.
    n_records++;
  if(n_errors >= 5) //Note intentional use of err_txt below, final error.
     err_txt += tmp_err_txt +"Too many log errors found in well "+GetWellName()+". Stopped processing.\n";
  else if(tmp_err_txt == "" && n_records < n_data && n_data_given == true) { //Note intentional use of err_txt below, only this error has occured.
    err_txt += "Wrong number of data records found in well "+GetWellName()+", found "+NRLib::ToString(n_records)
      +" while expecting "+NRLib::ToString(n_data)+".\n";
  }
  else {
    for(size_t i=0;i<log.size();i++) {
      AddContLog(log_name_[i],log[i]);
    }
  }
}


bool
LasWell::GetRecord(std:: ifstream           & fin,
                   size_t                     n_items,
                   std::vector<std::string> & record) const
{
  std::string line;
  while(line.empty()==true && fin.eof() == false)
    getline(fin,line);
  if(line.empty() == true)
    return(false);

  NRLib::Substitute(line, ",", " "); //Makes comma delimited space delimited.
  record = NRLib::GetTokens(line);
  if(wrap_ == true) {
    while(record.size() < n_items && fin.eof()==false) {
      getline(fin,line);
      NRLib::Substitute(line, ",", " "); //Makes comma delimited space delimited.
      std::vector<std::string> items = GetTokens(line);
      record.insert(record.end(), items.begin(), items.end());
    }
  }
  return(true);
}

void LasWell::AddLog(const std::string         & name,
                     const std::string         & units,
                     const std::string         & comment,
                     const std::vector<double> & log)
{
  Well::AddContLog(name, log);
  log_name_.push_back(name);
  log_unit_.push_back(units);
  log_comment_.push_back(comment);
}


int calculatePrecision(double value)
{
  double absVal = fabs(value);
  if (1e-16 < absVal && absVal < 1.0e3){
      int logVal = static_cast<int>(log10(absVal));
      int numDigitsAfterPoint = abs(logVal - 6);
      return numDigitsAfterPoint;
  }
  else{
      return 3;
  }
}

void LasWell::WriteToFile(const std::string              & filename,
                          const std::vector<std::string> & comment_header)
{
  std::ofstream file;
  OpenWrite(file, filename);

  // Comment header
  for (size_t i = 0; i < comment_header.size(); ++i) {
    file << "# " << comment_header[i] << "\n";
  }
  file << "#\n";

  // Version information
  file << "~Version information\n";
  WriteLasLine(file, "VERS", "", version_, "");
  WriteLasLine(file, "WRAP", "", (wrap_ ? "YES" : "NO"), "");
  for(size_t i=0;i<version_info_.size();i++)
    file << version_info_[i] << "\n";
  file << "#\n";

  file.setf(std::ios_base::fixed);
  file.precision(5);

  // Well information
  file << "~Well information\n";
  WriteLasLine(file, "STRT", depth_unit_, start_depth_, "");
  WriteLasLine(file, "STOP", depth_unit_, stop_depth_, "");
  WriteLasLine(file, "STEP", depth_unit_, depth_increment_, "");
  WriteLasLine(file, "NULL", "", GetContMissing(), "");
  for(size_t i=0;i<well_info_.size();i++)
    file << well_info_[i] << "\n";
  file << "#\n";

  if (GetNContLog() == 0) {
    // No log data in file.
    return;
  }

  //Parameter information. Only what is read from file; may add Set-functions.
  file << "~Parameter information\n";
  for(size_t i=0;i<parameter_info_.size();i++)
    file << parameter_info_[i] << "\n";
  file << "#\n";


  // Curve information
  // LAS only supports continuous logs...

  file << "~Curve information\n";
  for (size_t i = 0; i < GetNContLog(); ++i) {
    WriteLasLine(file, log_name_[i], log_unit_[i], "", log_comment_[i]);
  }
  file << "#\n";

  // Data section
  file << "~Ascii\n";

  std::vector<const std::vector<double> *> logs(GetNContLog());
  for (size_t i = 0; i < GetNContLog(); ++i) {
    logs[i] = &(GetContLog(log_name_[i]));
  }

  // We don't support wrapped output yet.
  assert(wrap_ == false);

  file.precision(3);
  for (size_t i = 0; i < logs[0]->size(); ++i) {
    for (size_t j = 0; j < logs.size(); ++j) {
      // Calculate a sensible precision. LAS does not support scientific notation 
      double value = (*logs[j])[i];
      int numDigitsAfterPoint = calculatePrecision(value);

      file.precision(numDigitsAfterPoint);
      file << value << " ";
    }
    file << "\n";
  }
}

void LasWell::WriteLasLine(std::ofstream     & file,
                           const std::string & mnemonic,
                           const std::string & units,
                           const std::string & data,
                           const std::string & description)
{
  file << mnemonic << " ." << units << " " << data << " : " << description << "\n";
}


void LasWell::WriteLasLine(std::ofstream     & file,
                           const std::string & mnemonic,
                           const std::string & units,
                           double              data,
                           const std::string & description)
{
  file << mnemonic << " ." << units << " " << data << " : " << description << "\n";
}

void LasWell::setDepthUnit(const std::string& depthUnit)
{
    depth_unit_ = depthUnit;
};

std::string LasWell::depthUnit() const
{
    return depth_unit_;
};

std::string LasWell::unitName(const std::string& logName) const
{
    std::map<std::string, std::string >::const_iterator it = logUnitMap_.find(logName);
    if (it != logUnitMap_.end())
    {
        return it->second;
    }

    return "";
};

void NRLib::LasWell::setVersionInfo(const std::string& versionInfo)
{
    version_ = versionInfo;
}

void NRLib::LasWell::setStartDepth(double startDepth)
{
    start_depth_ = startDepth;
}

void NRLib::LasWell::setStopDepth(double stopDepth)
{
    stop_depth_ = stopDepth;
}

void NRLib::LasWell::setDepthStep(double depthStep)
{
    depth_increment_ = depthStep;
}

void NRLib::LasWell::addWellInfo(const std::string& parameter, const std::string& value)
{
    // Example of line formatting taken from the documentation
    // WELL. aaaaaaaaaaaaaaaaaaaaa : WELL
    // 

    std::string info = parameter;
    info += " . ";
    info += value;
    info += " :";

    well_info_.push_back(info);
}
