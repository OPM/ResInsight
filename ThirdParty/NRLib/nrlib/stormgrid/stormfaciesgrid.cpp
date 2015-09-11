// $Id: stormfaciesgrid.cpp 882 2011-09-23 13:10:16Z perroe $

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

#include "stormfaciesgrid.hpp"

#include <fstream>

#include "../exception/exception.hpp"
#include "../iotools/fileio.hpp"
#include "../iotools/stringtools.hpp"
#include "../surface/surface.hpp"

using namespace NRLib;

const int STD_MISSING_CODE = -999;
const std::string format_desc[2] = {"storm_facies_binary",
                                    "storm_facies_ascii"};

StormFaciesGrid::StormFaciesGrid(const std::string& filename,Endianess file_format )
{
  ReadFromFile(filename, true, file_format);
}


StormFaciesGrid::StormFaciesGrid(size_t nx, size_t ny, size_t nz)
  : Grid<int>(nx, ny, nz, STD_MISSING_CODE)
{
  // Default values
  file_format_ = STORM_BINARY;
  missing_code_ = STD_MISSING_CODE;
  zone_number_ = 0;
  model_file_name_ = "ModelFile";
}

StormFaciesGrid::StormFaciesGrid(const Volume &vol, size_t nx, size_t ny, size_t nz)
:Volume(vol)
{
  file_format_ = STORM_BINARY;
  missing_code_ = STD_MISSING_CODE;
  zone_number_ = 0;
  model_file_name_ = "ModelFile";
  Resize(nx,ny,nz);

}


int StormFaciesGrid::GetFaciesCode(int body_index) const
{
  if (body_index < 0 || body_index >= static_cast<int>(body_facies_.size())) {
    throw Exception("Body index " + ToString(body_index)
      + " is outside the defined range [0, " + ToString(body_facies_.size()) );
  }

  return body_facies_[body_index];
}


int StormFaciesGrid::GetFaciesFromCell(size_t cell_index) const
{
  return GetFaciesCode((*this)(cell_index));
}


const std::string& StormFaciesGrid::GetFaciesName(int facies_code) const
{
  if (facies_code < 0 || facies_code >= static_cast<int>(facies_names_.size())) {
    throw Exception("Body index " + ToString(facies_code) + " is outside "
      + "the valid range [0, " + ToString(facies_names_.size()-1) + "]." );
  }

  return facies_names_[facies_code];
}

void StormFaciesGrid::SetBodyFacies(const std::vector<int>& body_facies)
{
  body_facies_ = body_facies;
}

void StormFaciesGrid::SetFaciesNames(const std::vector<std::string>& facies_names)
{
  facies_names_ = facies_names;
}


void StormFaciesGrid::WriteToFile(const std::string& filename, Endianess file_format) const
{
  CheckConsistency();

  std::ofstream file;
  OpenWrite(file, filename, std::ios::out | std::ios::binary);

  file << format_desc[file_format_] << "\n\n"
       << zone_number_ << " " << model_file_name_ << " "
       << missing_code_ << "\n\n";

  file << facies_names_.size() << "\n";
  /// \todo Print on several lines, if there are many facies.
  for (size_t i = 0; i < facies_names_.size(); ++i) {
    file << facies_names_[i] << " ";
  }
  file << "\n\n";

  file << body_facies_.size() << "\n";
  for (size_t i = 0; i < body_facies_.size(); ++i) {
    file << body_facies_[i] << " ";
  }
  file << "\n\n";

  WriteVolumeToFile(file, filename);
  file << "\n";
  file << GetNI() << " " << GetNJ() << " " << GetNK() << "\n";

  // Data
  int n_data = 0;
  switch (file_format_) {
  case STORM_BINARY:
    WriteBinaryIntArray(file, begin(), end(), file_format);
    break;
  case STORM_ASCII:
    for (const_iterator it = begin(); it != end(); ++it) {
      file << *it << " ";
      ++n_data;
      if (n_data % 10 == 0) {
        file << "\n";
      }
    }
   break;
  default:
    throw Exception("Unknown fileformat");
  }

  // Final 0 (Number of barriers)
  file << 0;
}


void StormFaciesGrid::ReadFromFile(const std::string& filename, bool commonPath,Endianess file_format)
{
  std::ifstream file;
  OpenRead(file, filename, std::ios::in | std::ios::binary);

  std::string path = "";
  if (commonPath == true)
    path = GetPath(filename);
    // Current line number
  int line = 0;

  // Header
  try {
    std::string token = ReadNext<std::string>(file, line);
    if (token == format_desc[STORM_BINARY]) {
      file_format_ = STORM_BINARY;
    }
    else if (token == format_desc[STORM_ASCII]) {
      file_format_ = STORM_ASCII;
    }
    else {
      throw FileFormatError("Unknown format: " + token);
    }

    zone_number_     = ReadNext<int>(file, line);
    model_file_name_ = ReadNext<std::string>(file, line);
    missing_code_    = ReadNext<int>(file, line);

    int size = ReadNext<int>(file, line);
    facies_names_.resize(size);
    for (int i = 0; i < size; ++i) {
      facies_names_[i] = ReadNext<std::string>(file, line);
    }

    size = ReadNext<int>(file, line);
    body_facies_.resize(size);
    for (int i = 0; i < size; ++i) {
      body_facies_[i] = ReadNext<int>(file, line);
    }

    ReadVolumeFromFile(file, line, path);

    int nx = ReadNext<int>(file, line);
    int ny = ReadNext<int>(file, line);
    int nz = ReadNext<int>(file, line);

    Resize(nx, ny, nz);

    switch (file_format_) {
    case STORM_BINARY:
      DiscardRestOfLine(file, line, true);
      ReadBinaryIntArray(file, begin(), GetN(), file_format);
      break;
    case STORM_ASCII:
      ReadAsciiArrayFast(file, begin(), GetN());
      break;
    default:
      throw Exception("Bug in STORM grid parser: unknown fileformat");
    }

    try {
      int n_barriers = ReadNext<int>(file, line); // line is now wrong.
      if (n_barriers != 0) {
        throw FileFormatError("Number of barriers greater than 0 found. "
          "Only grids without barriers are supported.");
      }
    }
    catch (EndOfFile& ) {
      // Number of barriers not present in file.
    }
  }
  catch (EndOfFile& ) {
    throw FileFormatError("Unexcpected end of file found while parsing "
      " \"" + filename + "\"");
  }
  catch (Exception& e) {
    throw FileFormatError("Error parsing \"" + filename + "\" as a "
      "STORM file at line " + ToString(line) + ":" + e.what());
  }

  CheckConsistency();
}


void StormFaciesGrid::CheckConsistency() const
{
  for (size_t i = 0; i < body_facies_.size(); ++i) {
    if (body_facies_[i] < 0 ||
        body_facies_[i] >= static_cast<int>(facies_names_.size())) {
      throw Exception("Consistency check failed: facies "
        + ToString(body_facies_[i]) + " in body " + ToString(i)
        + " is outside the valid range [0, "
        + ToString(facies_names_.size()-1) + "].");
    }
  }

  for (size_t ind = 0; ind < GetN(); ++ind) {
    int body = (*this)(ind);
    if ( !IsMissing(body)
         && (body < 0 || body > static_cast<int>(body_facies_.size())) ) {
      size_t i, j, k;
      GetIJK(ind, i, j, k);
      throw Exception("Consistency check failed: body number "
        + ToString(body) + " in cell (" + ToString(i) + ", " + ToString(j)
        + ", " + ToString(k) + ") is outside the valid range [0, "
        + ToString(body_facies_.size()-1) + "].");
    }
  }
}

size_t StormFaciesGrid::FindIndex(double x, double y, double z) const
{
  size_t i, j, k;
  FindIndex(x, y, z, i, j, k);
  return GetIndex(i, j, k);
}

void StormFaciesGrid::FindIndex(double x, double y, double z, size_t& i_out, size_t& j_out, size_t& k_out) const
{
  double local_x, local_y;
  GlobalToLocalCoord(x, y, local_x, local_y);

  i_out = static_cast<size_t>(local_x / GetDX());
  j_out = static_cast<size_t>(local_y / GetDY());
  if(i_out >= GetNI())
    throw Exception("Index in x direction is too large.\n");
  if(j_out >= GetNJ())
   throw Exception("Index in y direction is too large.\n");
  double z_top = GetTopSurface().GetZ(x, y);
  double z_bot = GetBotSurface().GetZ(x, y);
  double dz = (z_bot - z_top) / GetNK();

  if(z < z_top)
   throw Exception("Z value is above top.\n");
  if(z > z_bot)
    throw Exception("Z value is below bottom. \n");
  if (dz == 0) {
    k_out = 0;
  }
  else {
    k_out = static_cast<size_t>((z - z_top)/dz);
  }

}
/*
/// \todo Common implementation with StormContGrid
size_t StormFaciesGrid::FindIndex(double x, double y, double z) const
{
  double local_x, local_y;
  GlobalToLocalCoord(x, y, local_x, local_y);

  size_t i = static_cast<size_t>(local_x / GetDX());
  size_t j = static_cast<size_t>(local_y / GetDY());
  double z_top = GetTopSurface().GetZ(x, y);
  double z_bot = GetBotSurface().GetZ(x, y);
  double dz = (z_bot - z_top) / GetNK();
  size_t k;
  if (dz == 0) {
    k = 0;
  }
  else {
    k = static_cast<size_t>((z - z_top)/dz);
  }

  return GetIndex(i, j, k);
}
*/

double StormFaciesGrid::RecalculateLZ()
{
  double lz = 0;
  for (size_t i = 0; i < GetNI(); ++i) {
    for (size_t j = 0; j < GetNJ(); ++j) {
      double x, y;
      LocalToGlobalCoord(i * GetDX(), j * GetDY(), x, y);
      lz = std::max(lz, GetBotSurface().GetZ(x, y) -
                        GetTopSurface().GetZ(x, y));
    }
  }
  return lz;
}

void StormFaciesGrid::FindCenterOfCell(size_t  i, size_t  j, size_t  k,
                                     double& x, double& y, double& z) const
{
  double xloc = (i+0.5)*GetDX();
  double yloc = (j+0.5)*GetDY();
  LocalToGlobalCoord(xloc, yloc, x, y);
  double z_top = GetTopSurface().GetZ(x, y);
  double z_bot = GetBotSurface().GetZ(x, y);
  double dz = (z_bot - z_top) / GetNK();
  z = z_top + (k+0.5)*dz;
}
