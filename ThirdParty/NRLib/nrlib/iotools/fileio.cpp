//$Id: fileio.cpp 1068 2012-09-18 11:21:53Z perroe $

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

#include "fileio.hpp"

#include "../exception/exception.hpp"
#include "stringtools.hpp"

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

using namespace NRLib::NRLibPrivate;
using namespace NRLib;

const std::string format_desc[5] = {"storm_petro_binary",
                                    "storm_petro_ascii",
                                    "storm_facies_binary",
                                    "storm_facies_ascii",
                                    "NORSAR"};


void NRLib::OpenRead(std::ifstream&          stream,
                     const std::string&      filename,
                     std::ios_base::openmode mode)
{
  namespace fs = boost::filesystem;

  try {
    boost::filesystem::path file_path(filename);
    if (!fs::exists(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading: " +
                    "File does not exist.");
    }
    if (fs::is_directory(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading: " +
                    " It is a directory.");
    }
    stream.open(file_path.file_string().c_str(), mode);
    if (!stream) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading.");
    }
  }
  catch (fs::filesystem_error& e) {
    throw IOError("Failed to open " + filename + " for reading: " + e.what());
  }
}


void NRLib::OpenRead(std::fstream&          stream,
                     const std::string&      filename,
                     std::ios_base::openmode mode)
{
  namespace fs = boost::filesystem;

  try {
    boost::filesystem::path file_path(filename);
    if (!fs::exists(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading: " +
                    "File does not exist.");
    }
    if (fs::is_directory(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading: " +
                    " It is a directory.");
    }
    stream.open(file_path.file_string().c_str(), mode);
    if (!stream) {
      throw IOError("Failed to open " + file_path.file_string() + " for reading.");
    }
  }
  catch (fs::filesystem_error& e) {
    throw IOError("Failed to open " + filename + " for reading: " + e.what());
  }
}


void NRLib::OpenWrite(std::ofstream&          stream,
                       const std::string&      filename,
                       std::ios_base::openmode mode,
                       bool                    create_dir)
{
  namespace fs = boost::filesystem;
  try {
    fs::path file_path(filename);
    fs::path dir = file_path.parent_path();
    if (fs::is_directory(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for writing: " +
                    " It is a directory.");
    }
    if (!dir.empty()) {
      if (!fs::exists(dir) && create_dir) {
        create_directories(dir);
      }
      else if (!fs::exists(dir)) {
        throw IOError("Failed to open " + file_path.file_string() + " for writing: "
                      + "Parent directory does not exist.");
      }
    }
    stream.open(file_path.file_string().c_str(), mode);
    if (!stream) {
      throw IOError("Failed to open " + file_path.file_string() + " for writing.");
    }
  }
  catch (fs::filesystem_error& e) {
    throw IOError("Failed to open " + filename + " for writing: " + e.what());
  }
}


void NRLib::OpenWrite(std::fstream&          stream,
                       const std::string&      filename,
                       std::ios_base::openmode mode,
                       bool                    create_dir)
{
  namespace fs = boost::filesystem;
  try {
    fs::path file_path(filename);
    fs::path dir = file_path.parent_path();
    if (fs::is_directory(file_path)) {
      throw IOError("Failed to open " + file_path.file_string() + " for writing: " +
                    " It is a directory.");
    }
    if (!dir.empty()) {
      if (!fs::exists(dir) && create_dir) {
        create_directories(dir);
      }
      else if (!fs::exists(dir)) {
        throw IOError("Failed to open " + file_path.file_string() + " for writing: "
                      + "Parent directory does not exist.");
      }
    }
    stream.open(file_path.file_string().c_str(), mode);
    if (!stream) {
      throw IOError("Failed to open " + file_path.file_string() + " for writing.");
    }
  }
  catch (fs::filesystem_error& e) {
    throw IOError("Failed to open " + filename + " for writing: " + e.what());
  }
}


void NRLib::CopyFile(const std::string & from_path,
                     const std::string & to_path,
                     bool                allow_overwrite)
{
  if (allow_overwrite)
    RemoveFile(to_path);
  else {
    if (boost::filesystem::exists(to_path))
      throw IOError("Failed to open " + to_path + " for writing: The file already exists.");
  }

  boost::filesystem::copy_file(from_path, to_path);
}


void NRLib::RemoveFile(const std::string & filename)
{
  if (boost::filesystem::exists(filename))
    boost::filesystem::remove(filename);
}


bool NRLib::FileExists(const std::string & filename)
{
  return boost::filesystem::exists(filename);
}


void NRLib::CreateDirIfNotExists(const std::string & filename)
{
  namespace fs = boost::filesystem;

  fs::path file_path(filename);
  fs::path dir = file_path.parent_path();
  if (!dir.empty()){
    if (!fs::exists(dir))
      create_directories(dir);
  }
}

std::istream& NRLib::ReadNextToken(std::istream & stream,
                                   std::string  & s,
                                   int          & line_num)
{
  std::locale loc = std::locale();
  char c;
  if (!stream.good()) {
    stream.setstate(std::ifstream::failbit);
    s = "";
    return stream;
  }
  while(stream.get(c) && std::isspace(c,loc) == true ) {
    if (c == '\n')
      line_num++;
  }
  if (stream.good()) {
    s = c;
    while(stream.get(c) && std::isspace(c, loc) == false) {
      s = s+c;
    }
  }
  if (stream.good() == true) // Do not insert anything at eof.
    stream.putback(c);       // Put back to make sure that line-numbers are OK.
  return stream;
}

void NRLib::DiscardRestOfLine(std::istream& stream,
                               int&          line_num,
                               bool          throw_if_non_whitespace)
{
  std::locale loc;
  std::string s;
  std::getline(stream, s);
  line_num++;
  if (throw_if_non_whitespace) {
    std::string::iterator it = s.begin();
    while(it != s.end() && std::isspace(*it, loc))
      ++it;
    if (it != s.end())
      throw IOError("Non-whitespace characters encountered.");
  }
}


std::istream& NRLib::GetNextNonEmptyLine(std::istream & stream,
                                         int          & line_num,
                                         std::string  & line)
{
  while (std::getline(stream, line)) {
    ++line_num;
    if (line.find_first_not_of(Whitespace()) != std::string::npos) {
      return stream;
    }
  }
  line = "";
  return stream;
}


std::string NRLib::FindLastNonEmptyLine(std::istream & stream, const std::ios::pos_type & max_line_len)
{
  stream.seekg(0, std::ios::end);
  std::ios::pos_type file_len = stream.tellg();
  std::ios::pos_type offset = std::min(file_len, max_line_len);
  stream.seekg(static_cast<std::ios::off_type>(file_len) - offset);

  std::string line = "";
  std::string last_line = "";
  int dummy = 0;
  while (GetNextNonEmptyLine(stream, dummy, line))
  {
    last_line = line;
  }

  return last_line;
}


void NRLib::SkipComments(std::istream & stream,
                         char           comment_symbol,
                         int          & line_num)
{
  std::locale loc = std::locale();

  std::string line;
  bool comment = true;

  while (comment == true && stream.good()) {
    char c;

    while(stream.get(c) && std::isspace(c, loc)) {
      if (c == '\n')
        line_num++;
    }

    if (c == comment_symbol) {
      std::getline(stream, line);
      line_num++;
    }
    else {
      stream.unget();
      comment = false;
    }
  }
}


bool NRLib::CheckEndOfFile(std::istream& stream)
{
  char c;
  stream >> c;
  if (stream.eof()) {
    return true;
  }
  else {
  //  printf("end of file? %s\n", c);
    stream.putback(c);
    return false;
  }
}


unsigned long long
NRLib::FindFileSize(const std::string& filename)
{
  if ( !boost::filesystem::exists(filename) ) {
    throw IOError("File " + filename + " does not exist.");
  }

  return static_cast<unsigned long long>(boost::filesystem::file_size(filename));
}


int NRLib::FindGridFileType(const std::string& filename )
{
  unsigned long long length = FindFileSize(filename);
  std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
  if (!file) {
    throw IOError("Error opening " + filename);
  }

  char buffer[3201];
  if (length > 161) {
    file.read(buffer, 3200);
    buffer[3200] = '\0';
  }
  else {
    file.read(buffer, static_cast<std::streamsize>(length));
    buffer[length] = '\0';
  }

  std::string stringbuffer = std::string(buffer);
  std::istringstream i(stringbuffer);
  std::string token;
  i>>token;

  if (token == format_desc[STORM_PETRO_BINARY]) {
      return STORM_PETRO_BINARY;
  }
  else if (token == format_desc[STORM_PETRO_ASCII]) {
      return STORM_PETRO_ASCII;
  }
  else if (token == format_desc[STORM_FACIES_BINARY]) {
      return STORM_FACIES_BINARY;
  }
  else if (token == format_desc[STORM_FACIES_ASCII]) {
      return STORM_FACIES_ASCII;
  }
  else if (token == format_desc[SGRI]) {
    //std::getline(i, token);
    i>>token;
    i>>token;
    i>>token;
    i>>token;
    if(token == "v1.0" ||token == "v2.0" )
      return SGRI;
  }
  else if (NRLib::IsType<double>(token)) {
    return PLAIN_ASCII;
  }
  else if (length>3200) //Check SEGY: Looking for EBCDIC header.
  {
    unsigned char * buf = reinterpret_cast<unsigned char *>(buffer);
    std::vector<int> frequency(256,0);
    int i;
    for(i=0;i<3200;i++)
      frequency[buf[i]]++;

    bool segy_ok = true;
    int  low_count = 0;
    for(i=0;i<64;i++)
      low_count += frequency[i];

    if(low_count > 5)
      segy_ok = false;

    i++;
    for(;(i<256) && (segy_ok == true);i++) {
      if(frequency[i] > frequency[64]) //Assumption is that EBCDIC 64 (space) is most common in header.
        segy_ok = false;
    }
    if(segy_ok == true)
    {
      return SEGY;
    }
  }
  return(UNKNOWN);
}

void NRLib::WriteBinaryShort(std::ostream& stream,
                              short s,
                              Endianess file_format)
{
  char buffer[2];

  switch (file_format) {
  case END_BIG_ENDIAN:
    WriteUInt16BE(buffer, static_cast<unsigned short>(s));
    break;
  case END_LITTLE_ENDIAN:
    WriteUInt16LE(buffer, static_cast<unsigned short>(s));
    break;
  default:
    throw Exception("Invalid file format.");
  }

  if (!stream.write(buffer, 2)) {
    throw Exception("Error writing to stream.");
  }
}


short NRLib::ReadBinaryShort(std::istream& stream,
                              Endianess file_format)
{
  unsigned short us;
  char buffer[2];

  if (!stream.read(buffer, 2)) {
    if(stream.eof())
      throw EndOfFile();
    else
      throw Exception("Error reading from stream (a).");
  }

  switch (file_format) {
  case END_BIG_ENDIAN:
    ParseUInt16BE(buffer, us);
    break;
  case END_LITTLE_ENDIAN:
    ParseUInt16LE(buffer, us);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  return static_cast<short>(us);
}


void NRLib::WriteBinaryInt(std::ostream& stream,
                            int i,
                            Endianess file_format)
{
  char buffer[4];

  switch (file_format) {
  case END_BIG_ENDIAN:
    WriteUInt32BE(buffer, static_cast<unsigned int>(i));
    break;
  case END_LITTLE_ENDIAN:
    WriteUInt32LE(buffer, static_cast<unsigned int>(i));
    break;
  default:
    throw Exception("Invalid file format.");
  }

  if (!stream.write(buffer, 4)) {
    throw Exception("Error writing to stream.");
  }
}


int NRLib::ReadBinaryInt(std::istream& stream,
                          Endianess file_format)
{
  unsigned int ui;
  char buffer[4];

  if (!stream.read(buffer, 4)) {
    if(stream.eof())
      throw EndOfFile();
    else
      throw Exception("Error reading from stream (b).");
  }

  switch (file_format) {
  case END_BIG_ENDIAN:
    ParseUInt32BE(buffer, ui);
    break;
  case END_LITTLE_ENDIAN:
    ParseUInt32LE(buffer, ui);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  return static_cast<int>(ui);
}


void NRLib::WriteBinaryFloat(std::ostream& stream,
                              float f,
                              Endianess file_format)
{
  char buffer[4];

  switch (file_format) {
  case END_BIG_ENDIAN:
    WriteIEEEFloatBE(buffer, f);
    break;
  case END_LITTLE_ENDIAN:
    WriteIEEEFloatLE(buffer, f);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  if (!stream.write(buffer, 4)) {
    throw Exception("Error writing to stream.");
  }
}


float NRLib::ReadBinaryFloat(std::istream& stream,
                              Endianess file_format)
{
  float f;
  char buffer[4];

  if (!stream.read(buffer, 4)) {
    if(stream.eof())
      throw EndOfFile();
    else
      throw Exception("Error reading from stream (c).");
  }

  switch (file_format) {
  case END_BIG_ENDIAN:
    ParseIEEEFloatBE(buffer, f);
    break;
  case END_LITTLE_ENDIAN:
    ParseIEEEFloatLE(buffer, f);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  return f;
}


void NRLib::WriteBinaryDouble(std::ostream& stream,
                               double d,
                               Endianess file_format)
{
  char buffer[8];

  switch (file_format) {
  case END_BIG_ENDIAN:
    WriteIEEEDoubleBE(buffer, d);
    break;
  case END_LITTLE_ENDIAN:
    WriteIEEEDoubleLE(buffer, d);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  if (!stream.write(buffer, 8)) {
    throw Exception("Error writing to stream.");
  }
}


double NRLib::ReadBinaryDouble(std::istream& stream,
                                Endianess file_format)
{
  double d;
  char buffer[8];

  if (!stream.read(buffer, 8)) {
    if(stream.eof())
      throw EndOfFile();
    else
      throw Exception("Error reading from stream (d).");
  }

  switch (file_format) {
  case END_BIG_ENDIAN:
    ParseIEEEDoubleBE(buffer, d);
    break;
  case END_LITTLE_ENDIAN:
    ParseIEEEDoubleLE(buffer, d);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  return d;
}


void NRLib::WriteBinaryIbmFloat(std::ostream& stream,
                                 float f,
                                 Endianess file_format)
{
  char buffer[4];

  switch (file_format) {
  case END_BIG_ENDIAN:
    WriteIBMFloatBE(buffer, f);
    break;
  case END_LITTLE_ENDIAN:
    WriteIBMFloatLE(buffer, f);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  if (!stream.write(buffer, 4)) {
    throw Exception("Error writing to stream.");
  }
}


float NRLib::ReadBinaryIbmFloat(std::istream& stream,
                                 Endianess file_format)
{
  float f;
  char buffer[4];

  if (!stream.read(buffer, 4)) {
    if(stream.eof())
      throw EndOfFile();
    else
      throw Exception("Error reading from stream (e).");
  }

  switch (file_format) {
  case END_BIG_ENDIAN:
    ParseIBMFloatBE(buffer, f);
    break;
  case END_LITTLE_ENDIAN:
    ParseIBMFloatLE(buffer, f);
    break;
  default:
    throw Exception("Invalid file format.");
  }

  return f;
}

void NRLib::ReadNextQuoted(std::istream& stream, char quote, std::string&  s, int& line)
{
  char c = 0;

  int found = 0;
  while( found == 0 && !stream.eof() ){
    stream.get(c);
    if (c == '\n'){
      line++;
    }
    if (c == quote){
      found++;
      stream.get(c);
    }
  }
  s ="";
  while( found == 1  && !stream.eof() ) {
    s = s+c;
    stream.get(c);
    if (c == '\n'){
      line++;
    }
    if (c == quote){
      found++;
    }
  }
  if (s == "")              //We are at end of file if nothing is read.
    throw EndOfFile();

}

bool
NRLib::IgnoreComment(std::ifstream& file, char chin){
  char ch;
  do {
    if(!file.get(ch)) {
      throw Exception("Unexpected end of file.");
     // NRLib::LogKit::LogMessage(NRLib::LogKit::Error, "Unexpected end of file. \n\n");
     // Havana::Exit(EXIT_FAILURE);
    }
  } while(isspace(ch));

  std::string dummy;
  if (ch == chin){
    getline(file, dummy);
    return true;
  }
  else {
    file.unget();
    return false;
  }
}


int NRLib::Seek(FILE * file, long long offset, int origin)
{
  int ret;
#if defined(_MSC_VER)
  ret = _fseeki64(file, offset, origin);
#else
  ret = fseek(file, offset, origin);
#endif
  return(ret);
}

