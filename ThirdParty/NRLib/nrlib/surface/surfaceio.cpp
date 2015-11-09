// $Id: surfaceio.cpp 882 2011-09-23 13:10:16Z perroe $

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

#include <cmath>
#include <fstream>
#include <string>

#include "surfaceio.hpp"

#include "regularsurface.hpp"
#include "regularsurfacerotated.hpp"
#include "surface.hpp"
#include "../exception/exception.hpp"
#include "../iotools/fileio.hpp"
#include "../iotools/stringtools.hpp"

namespace NRLib {

//const double IRAP_MISSING  = 9999900.0;
//const double STORM_MISSING =    -999.0;


namespace NRLibPrivate {
/// \todo Move to a suitable place
bool Equal(double a, double b)
{
  if ( fabs(a-b) <= 0.0001 * a ) {
    return true;
  }
  return false;
}
} // namespace NRLibPrivate

SurfaceFileFormat FindSurfaceFileType(const std::string& filename)
{
  std::ifstream file;
  OpenRead(file, filename, std::ios::in | std::ios::binary);

  std::string first_token;
  if (!(file >> first_token)) {
    // Empty file.
    return SURF_UNKNOWN;
  }

  if (first_token == "-996")
    return SURF_IRAP_CLASSIC_ASCII;
  else if (first_token == "STORMGRID_BINARY")
    return SURF_STORM_BINARY;
  else if (first_token == "NORSAR") {
    std::string token;
    file >> token;
    file >> token;
    file >> token;
    file >> token;
    if(token == "v1.0" ||token == "v2.0" )
      return SURF_SGRI;
  }
  else if (IsType<double>(first_token)) {
    file.seekg(0, std::ios_base::beg);
    std::string line;
    std::getline(file, line);
    std::vector<std::string> tokens = GetTokens(line);
    if (tokens.size() == 3) {
      // Check last line of file.
      file.seekg(0, std::ios_base::end);
      std::streampos file_len = file.tellg();
      std::streampos offset = std::min(static_cast<std::streampos>(50), file_len);
      file.seekg(-offset, std::ios_base::end);

      tokens.clear();

      while (std::getline(file, line)) {
        if (GetTokens(line).size() > 0) {
          tokens = GetTokens(line);
        }
      }

      if (tokens.size() == 3) {
        try {
          if (ParseType<double>(tokens[0]) == 999.0 &&
              ParseType<double>(tokens[1]) == 999.0 &&
              ParseType<double>(tokens[2]) == 999.0)   {
            return SURF_RMS_POINTS_ASCII;
          }
        }
        catch (Exception& ) {
          // Do nothing - failed to parse tokens as doubles.
        }
      }
    }
    return SURF_UNKNOWN;
  }

  //Try multicolum ascii
  int dummy;
  bool is_multicolumn_ascii = FindMulticolumnAsciiLine(filename, dummy);

  if (is_multicolumn_ascii == true)
    return SURF_MULT_ASCII;

  return SURF_UNKNOWN;
}


std::string GetSurfFormatString(NRLib::SurfaceFileFormat format)
{
  switch (format) {
    case SURF_UNKNOWN:
      return "Unknown surface format";
    case SURF_IRAP_CLASSIC_ASCII:
      return "IRAP classic ASCII";
    case SURF_STORM_BINARY:
      return "Storm binary";
    case SURF_SGRI:
      return "NORSAR SGRI";
    case SURF_RMS_POINTS_ASCII:
      return "RMS XYZ point set surface";
    case SURF_MULT_ASCII:
      return "Multicolumn ASCII";
    default:
      throw Exception("Missing format description for format: " + ToString(format));
  }
}


std::vector<RegularSurfaceRotated<float> >
ReadMultipleSgriSurf(const std::string& filename, const std::vector<std::string> & labels)
{
  std::ifstream header_file;
  OpenRead(header_file, filename.c_str(), std::ios::in | std::ios::binary);
  int i, dim;
  std::string tmp_str;
  try {
    //Reading record 1: Version header
    getline(header_file, tmp_str);
    //Reading record 2: Grid dimension
    header_file >> dim;
    if(dim!=2)
      throw Exception("Wrong dimension of Sgri file. We expect a surface, dimension should be 2.\n");

    getline(header_file, tmp_str);
    //Reading record 3 ... 3+dim: Axis labels + grid value label
    std::vector<std::string> axis_labels(dim);
    std::string err_txt;
    for (i=0; i<dim; i++) {
      getline(header_file, axis_labels[i]);
      if(labels.size() > static_cast<size_t>(i)) {
        NRLib::Uppercase(axis_labels[i]);
        NRLib::Uppercase(labels[i]);
        if (axis_labels[i].find(labels[i]) == std::string::npos)
          err_txt += "Wrong axis labels. Label for axis "+NRLib::ToString(i)+" should be '"+labels[i]+"'.\n";
      }
    }
    getline(header_file, tmp_str);

    //Reading record 4+dim: Number of grids
    int n_grid;
    header_file >> n_grid;
    if (n_grid < 1) {
      throw Exception("Error: Number of grids read from sgri file must be > 0");
    }
    getline(header_file, tmp_str);

    //Reading record 5+dim ... 5+dim+ngrid-1: Grid labels
    for (i=0; i<n_grid; i++)
      getline(header_file, tmp_str);

    std::vector<float> d_values1(dim);
    std::vector<float> d_values2(dim);
    std::vector<int>   i_values(dim);
    //Reading record 5+dim+ngrid: Scaling factor of grid values
    for (i=0; i<dim; i++)
      header_file >> d_values1[i];
    getline(header_file,tmp_str);
    //Reading record 6+dim+ngrid: Number of samples in each dir.
    for (i=0; i<dim; i++)
      header_file >> i_values[i];
    getline(header_file,tmp_str);
    //Reading record 7+dim+ngrid: Grid sampling in each dir.
    for (i=0; i<dim; i++)
      header_file >> d_values2[i];

    getline(header_file,tmp_str);
    //Reading record 8+dim+ngrid: First point coord.
    std::vector<float> min_values(dim);
    for (i=0; i<dim; i++)
      header_file >> min_values[i];

    int nx=1;
    int ny=1;
    double dx, dy;
    nx      = i_values[0];
    dx      = d_values2[0];
    ny      = i_values[1];
    dy      = d_values2[1];

    if (nx < 1)
      throw Exception("Error: Number of samples on first axis must be >= 1.\n");
    if (ny < 1)
      throw Exception("Error: Number of samples on second axis must be >= 1.\n");

    if (dx <= 0.0)
      throw Exception("Error: Grid sampling on first axis must be > 0.0.\n");
    if (dy <= 0.0)
      throw Exception("Error: Grid sampling on second axis must be > 0.0.\n");

    double lx = (nx-1)*dx;
    double ly = (ny-1)*dy;

    double x_min = min_values[0]-0.5*dx;
    double y_min = min_values[1]-0.5*dy;

    double angle;
    header_file >> angle;

    getline(header_file, tmp_str);
    //Reading record 10+dim+ngrid: Undef value
    float missing_code;
    header_file >> missing_code;

    getline(header_file, tmp_str);
    //Reading record 11+dim+ngrid: Filename of binary file
    std::string bin_file_name;
    getline(header_file, tmp_str);
    if (!tmp_str.empty()) {
      std::locale loc;
      int i = 0;
      char c = tmp_str[i];
      while (!std::isspace(c,loc)) {
        i++;
        c = tmp_str[i];
      }
      tmp_str.erase(tmp_str.begin()+i, tmp_str.end());
    }
    if (tmp_str.empty())
      bin_file_name = NRLib::ReplaceExtension(filename, "Sgri");
    else {
      std::string path = GetPath(filename);
      bin_file_name = path + "/" + tmp_str;
    }
    //Reading record 12+dim+ngrid: Complex values
    bool has_complex;
    header_file >> has_complex;
    if (has_complex != 0 ) {
      throw Exception("Error: Can not read Sgri binary file. Complex values?");
    }

    std::vector<RegularSurfaceRotated<float> > surfaces(n_grid);
    std::ifstream bin_file;
    OpenRead(bin_file, bin_file_name.c_str(), std::ios::in | std::ios::binary);
    for (i=0; i<n_grid; i++) {
      surfaces[i] = RegularSurfaceRotated<float>(x_min,y_min,lx,ly,nx,ny,angle,0.0f);
      surfaces[i].SetMissingValue(missing_code);
      ReadBinaryFloatArray(bin_file, surfaces[i].begin(), surfaces[i].GetN());
    }

    return surfaces;
  }
  catch (Exception& e) {
    throw FileFormatError("Error parsing \"" + filename + "\" as a "
      "Sgri surface file " + e.what() + "\n");
  }
}

bool FindMulticolumnAsciiLine(const std::string& filename, int & header_start_line)
{
  //Contains five columns: X, Y, IL, CL, Attribute
  //File starts with several information lines
  std::ifstream file;
  NRLib::OpenRead(file, filename);
  int line = 0;

  //Find first line with five numbers
  bool found_mult_ascii_line = false;
  while (found_mult_ascii_line == false) {

    //Read line
    std::string line_string;
    std::getline(file, line_string);
    std::vector<std::string> tokens = NRLib::GetTokens(line_string);

    //Check if the line has five number elements
    if (tokens.size() == 5) {
      for (int i = 0; i < 5; i++) {
        if (!NRLib::IsNumber(tokens[i])) {
          found_mult_ascii_line = false;
          break;
        }
        else
          found_mult_ascii_line = true;
      }
    }

    line++;
  }
  file.close();

  header_start_line = line -2;

  return found_mult_ascii_line;
}


//void WritePointAsciiSurf(const RegularSurface<double>& surf,
//                         const std::string& filename);

} // namespace NRLib
