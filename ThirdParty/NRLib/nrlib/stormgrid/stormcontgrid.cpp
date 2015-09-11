// $Id: stormcontgrid.cpp 1190 2013-07-03 10:57:27Z ulvmoen $

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

#include "stormcontgrid.hpp"
#include <cmath>
#include <fstream>
#include <math.h>
#include <locale>
#include <cassert>

#include "../exception/exception.hpp"
#include "../iotools/fileio.hpp"
#include "../iotools/stringtools.hpp"
#include "../surface/surface.hpp"
#include "../surface/regularsurface.hpp"
#include "../surface/regularsurfacerotated.hpp"

using namespace NRLib;

const float STD_MISSING_CODE = -999.0F;
const std::string format_desc[2] = {"storm_petro_binary",
                                    "storm_petro_ascii"};


StormContGrid::StormContGrid(size_t nx, size_t ny, size_t nz)
  : Grid<float>(nx, ny, nz, STD_MISSING_CODE)
{
  // Default values
  file_format_ = STORM_BINARY;
  missing_code_ = STD_MISSING_CODE;
  zone_number_ = 0;
  model_file_name_ = "ModelFile";
  variable_name_ = "UNKNOWN";
}

StormContGrid::StormContGrid(const Volume &vol, size_t nx, size_t ny, size_t nz)
:Volume(vol)
{
  file_format_ = STORM_BINARY;
  missing_code_ = STD_MISSING_CODE;
  zone_number_ = 0;
  model_file_name_ = "ModelFile";
  variable_name_ = "UNKNOWN";
  Resize(nx,ny,nz);
}

StormContGrid::StormContGrid(const Volume &vol, const Grid<float> & grid)
:Grid<float>(grid),
 Volume(vol)
{
  file_format_ = STORM_BINARY;
  missing_code_ = STD_MISSING_CODE;
  zone_number_ = 0;
  model_file_name_ = "ModelFile";
  variable_name_ = "UNKNOWN";
}

StormContGrid::StormContGrid(const std::string& filename, Endianess file_format)
{
  ReadFromFile(filename, true, file_format);
}


void StormContGrid::ReadFromFile(const std::string& filename, bool commonPath, Endianess number_representation)
{
  std::ifstream file;
  OpenRead(file, filename, std::ios::in | std::ios::binary);

  std::string path = "";
  if (commonPath == true)
    path = GetPath(filename);

  if (!file) {
    throw new IOError("Error opening " + filename);
  }

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
    else if(token=="NORSAR")
    {
      std::string binfilename;
      ReadSgriHeader(file, binfilename);
      if (binfilename.empty())
        binfilename = NRLib::ReplaceExtension(filename, "Sgri");
      else
        binfilename = path + "/" + binfilename;
      std::ifstream binFile(binfilename.c_str(), std::ios::in | std::ios::binary);
      if(!binFile) {
        throw Exception("Error: Could not open Sgri binary file"+binfilename+"for reading.\n");
        return;
      }
      binFile.close();
      ReadSgriBinaryFile(binfilename);
      file_format_ = STORM_BINARY; // when writing to file, use binary format.
      zone_number_ = 0;
      model_file_name_ = "ModelFile";
      variable_name_ = "UNKNOWN";
      return;
    }
    else {
      throw FileFormatError("Unknown format: " + token);
    }

    zone_number_ = ReadNext<int>(file, line);
    model_file_name_ = ReadNext<std::string>(file, line);
    missing_code_    = ReadNext<float>(file, line);
    variable_name_   = ReadNext<std::string>(file, line);

    ReadVolumeFromFile(file, line, path);

    int nx = ReadNext<int>(file, line);
    int ny = ReadNext<int>(file, line);
    int nz = ReadNext<int>(file, line);

    Resize(nx, ny, nz);

    switch (file_format_) {
    case STORM_BINARY:
      DiscardRestOfLine(file, line, true);
      ReadBinaryFloatArray(file, begin(), GetN(), number_representation);
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
      "STORM file : " + e.what()+"\n");
  }
}


void StormContGrid::WriteToFile(const std::string& filename, const std::string& predefinedHeader, bool plainAscii, Endianess file_format, bool remove_path) const
{
  std::ofstream file;
  OpenWrite(file, filename, std::ios::out | std::ios::binary);

  //file.precision(14);
  file.precision(4);

  // Header
  if (predefinedHeader == "" && plainAscii==false) {
    file << format_desc[file_format_] << "\n\n"
         << zone_number_ << " " << model_file_name_ << " "
         << missing_code_ << "\n\n" << variable_name_ << "\n\n" ;

    WriteVolumeToFile(file, filename, remove_path);
    file << "\n";
    file << GetNI() << " " << GetNJ() << " " << GetNK() << "\n";
  }
  else
    file << predefinedHeader;
  // Data
  int n_data = 0;
  switch (file_format_) {
  case STORM_BINARY:
    WriteBinaryFloatArray(file, begin(), end(), file_format);
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
  if (plainAscii==false)
    file << 0;
}

void StormContGrid::WriteToSgriFile(const std::string & file_name,
                                    const std::string & file_name_header,
                                    const std::string & label,
                                    double              simbox_dz,
                                    Endianess           file_format) const
{
  // Header
  double vert_scale = 0.001;
  double hor_scale  = 0.001;

  std::ofstream header_file;
  NRLib::OpenWrite(header_file, file_name_header);

  header_file << "NORSAR General Grid Format v1.0\n";
  header_file << "3\n";
  header_file << "X (km)\n";
  header_file << "Y (km)\n";
  header_file << "T (s)\n";
  header_file << "FFT-grid\n";
  header_file << "1\n";
  header_file << label << std::endl;
  header_file << "1 1 1\n";

  double z_max = GetZMax();
  double z_min = GetZMin();

/*  float dz = static_cast<float> (floor(simbox->getdz()+0.5)); //To have the same sampling as in SegY
  if (dz == 0.0)
    dz = 1.0; */
  float dz = static_cast<float> (simbox_dz);
  int nz = static_cast<int> (ceil((z_max - z_min)/dz));
  int ny = static_cast<int>(GetNJ());
  int nx = static_cast<int>(GetNI());
  header_file << nx << " " << ny << " " << nz << std::endl;
  header_file << std::setprecision(10);
  header_file << GetDX()*hor_scale << " " << GetDY()*hor_scale << " " << dz*vert_scale << std::endl;
  double x0 = GetXMin() + 0.5 * GetDX();
  double y0 = GetYMin() + 0.5 * GetDY();
  double z0 = z_min + 0.5 * dz;
  header_file << x0*hor_scale << " " << y0*hor_scale << " " << z0*vert_scale << std::endl;
  header_file << GetAngle() << " 0\n";
  header_file << missing_code_ << std::endl;

  //fName = fileName + IO::SuffixSgri();
  header_file << file_name << std::endl;
  header_file << "0\n";

  std::ofstream file;
  OpenWrite(file, file_name, std::ios::out | std::ios::binary);

  file.precision(14);

  // Data
  WriteBinaryFloatArray(file, begin(), end(), file_format);

  // Final 0 (Number of barriers)
  file << 0;
}


/// \todo Common implementation with StormFaciesGrid
size_t StormContGrid::FindIndex(double x, double y, double z) const
{
  size_t i, j, k;
  FindIndex(x, y, z, i, j, k);
  return GetIndex(i, j, k);
}

void StormContGrid::FindXYIndex(double x, double y, size_t& i_out, size_t& j_out) const
{
  double local_x, local_y;

  GlobalToLocalCoord(x, y, local_x, local_y);

  i_out = static_cast<size_t>(local_x / GetDX());
  j_out = static_cast<size_t>(local_y / GetDY());

  if(i_out >= GetNI()) {
    std::string text ="Trying to look up an xy-position outside grid. Index in x direction is too large.\n";
    text += "  Index : "+ToString(i_out)+"\n";
    text += "  Max   : "+ToString(GetNI())+"\n";
    throw Exception(text);
  }
  if(j_out >= GetNJ()) {
    std::string text ="Trying to look up an xy-position outside grid: Index in y direction is too large.\n";
    text += "  Index : "+ToString(j_out)+"\n";
    text += "  Max   : "+ToString(GetNJ())+"\n";
    throw Exception(text);
  }
  if(local_x < 0.0) {
    std::string text ="Trying to look up an xy-position outside grid: Local x-coordinate is too small ("+NRLib::ToString(local_x, 2)+").\n";
    throw Exception(text);
  }
  if(local_y < 0.0) {
    std::string text ="Trying to look up an xy-position outside grid: Local y-coordinate is too small ("+NRLib::ToString(local_y, 2)+").\n";
    throw Exception(text);
  }
}

void StormContGrid::FindIndex(double x, double y, double z, size_t& i_out, size_t& j_out, size_t& k_out) const
{

  FindXYIndex(x, y, i_out, j_out);

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


void StormContGrid::FindZInterpolatedIndex(const double & x,
                                           const double & y,
                                           const double & z,
                                           size_t       & ind1,
                                           size_t       & ind2,
                                           double       & t) const
{

  size_t i;
  size_t j;
  size_t k;

  FindXYIndex(x, y, i, j);

  double z_top = GetTopSurface().GetZ(x, y);
  double z_bot = GetBotSurface().GetZ(x, y);
  double dz    = (z_bot - z_top) / GetNK();

  if (z<=z_top+0.5*dz) {
    k    = 0;
    t    = missing_code_;
    ind1 = GetIndex(i, j, k);
    ind2 = 0;
  }
  else if (z>=z_bot-0.5*dz) {
    k    = GetNK()-1;
    t    = missing_code_;
    ind1 = GetIndex(i, j, k);
    ind2 = 0;
  }
  else {
    k    = static_cast<size_t>(floor(((z - z_top) / dz) - 0.5));
    t    = (z-z_top)/dz - 0.5 - static_cast<double>(k);
    ind1 = GetIndex(i, j, k);
    ind2 = GetIndex(i, j, k+1);
  }
}

float StormContGrid::GetValueZInterpolatedFromIndex(const size_t & ind1,
                                                    const size_t & ind2,
                                                    const double & t) const
{
  float value;

  if (t == missing_code_)
    value = (*this)(ind1);

  else {
    float v1 = (*this) (ind1);
    float v2 = (*this) (ind2);

    if(v1 != missing_code_) {
      if(v2 != missing_code_)
        value = static_cast<float>(v1*(1-t) + v2*t);
      else
        value = v1;
    }
    else
      value = v2; //Ok even if v2 is missing, then both are missing so missing is result.
  }

  return(value);
}

double StormContGrid::GetValueZInterpolatedFromIndexNoMissing(const size_t & ind1,
                                                              const size_t & ind2,
                                                              const double & t) const
{ // Removed tests for missing from identical function
  // GetValueZInterpolatedFromIndex to decrease computation time for multizone background model
  double value;

  if (t == missing_code_)
    value = (*this)(ind1);

  else {
    float v1 = (*this) (ind1);
    float v2 = (*this) (ind2);

    value = v1*(1-t) + v2*t;
  }

  return(value);
}

float StormContGrid::GetValueInterpolated(const int   & i,
                                          const int   & j,
                                          const float & k_value) const
{
  float value, val1, val2;

  int k1 = int(floor(k_value));
  val1 = GetValue(i, j, k1);
  if (val1 == missing_code_)
    return(missing_code_);

  int k2 = k1+1;
  if (k1 == static_cast<int>(GetNK()-1))
    k2 = k1;

  val2 = GetValue(i, j, k2);
  if (val2 == missing_code_)
    return (val1);

  value = float(1.0-(k_value-k1))*val1+float(k_value-k1)*val2;
  return(value);

}

float StormContGrid::GetValueZInterpolated(double x, double y, double z)const
{

  size_t ind1;
  size_t ind2;
  double t;

  FindZInterpolatedIndex(x, y, z, ind1, ind2, t);

  float value = GetValueZInterpolatedFromIndex(ind1, ind2, t);

  return(value);
}


float StormContGrid::GetValueClosestInZ(double x, double y, double z)const
{
  double local_x, local_y;
  GlobalToLocalCoord(x, y, local_x, local_y);

  size_t i = static_cast<size_t>(local_x / GetDX());
  size_t j = static_cast<size_t>(local_y / GetDY());
  double z_top = GetTopSurface().GetZ(x, y);
  double z_bot = GetBotSurface().GetZ(x, y);
  double dz = (z_bot - z_top) / GetNK();
  if (local_x < 0.0 || i >= GetNI() || local_y < 0.0 || j >= GetNJ())
    return(missing_code_);

  if (z<z_top+0.5*dz)
    z = z_top+0.5*dz;
  else if (z>z_bot+0.5*dz)
    z = z_bot+0.5*dz;

  int zInd1 = static_cast<int>(floor((z-z_top)/dz)-0.5);
  double t = (z-z_top)/dz - 0.5 - static_cast<double>(zInd1);
  int zInd2 = zInd1+1;
  float value = static_cast<float>((1-t)*(*this)(GetIndex(i,j,zInd1))+t*(*this)(GetIndex(i,j,zInd2)));
  return(value);
}


void StormContGrid::FindCenterOfCell(size_t  i, size_t  j, size_t  k,
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


double StormContGrid::RecalculateLZ()
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

void StormContGrid::ReadSgriHeader(std::ifstream &headerFile, std::string &binFileName)
{
 // std::ifstream headerFile(filename.c_str(), std::ios::in); //checkFileOpen performed previously
  int i;
  std::string tmpStr;
  int dim;
  //Reading record 1: Version header
  getline(headerFile, tmpStr);
  //Reading record 2: Grid dimension
  headerFile >> dim;
  if(dim!=3)
    throw Exception("Wrong dimension of Sgri file. Must be 3.");

  getline(headerFile, tmpStr);
  //Reading record 3 ... 3+dim: Axis labels + grid value label
  std::vector<std::string> axisLabels(dim);
  for (i=0; i<dim; i++)
    getline(headerFile, axisLabels[i]);
  getline(headerFile, tmpStr);
  //int config = IMISSING;


  //Reading record 4+dim: Number of grids
  int nGrid;
  headerFile >> nGrid;
  if (nGrid < 1) {
    throw Exception("Error: Number of grids read from sgri file must be >0");
  }
  getline(headerFile, tmpStr);
  //Reading record 5+dim ... 5+dim+ngrid-1: Grid labels

  for (i=0; i<nGrid; i++)
    getline(headerFile, tmpStr);

  float *dValues1 = new float[dim];
  float *dValues2 = new float[dim];
  int *iValues = new int[dim];
  //Reading record 5+dim+ngrid: Scaling factor of grid values
  for (i=0; i<dim; i++)
    headerFile >> dValues1[i];
  getline(headerFile,tmpStr);
  //Reading record 6+dim+ngrid: Number of samples in each dir.
  for (i=0; i<dim; i++)
    headerFile >> iValues[i];
  getline(headerFile,tmpStr);
  //Reading record 7+dim+ngrid: Grid sampling in each dir.
  for (i=0; i<dim; i++) {
    headerFile >> dValues2[i];
  }
  getline(headerFile,tmpStr);
  //Reading record 8+dim+ngrid: First point coord.
  float *minValues = new float[dim];
  for (i=0; i<dim; i++)
  {
    headerFile >> minValues[i];
  }

  int nX=1;
  int nY=1;
  int nZ=1;
  double dX, dY, dZ;
   // scaleX_ = dValues1[0];
  nX      = iValues[0];
  dX     = dValues2[0];
 //   scaleY_ = dValues1[1];
  nY      = iValues[1];
  dY     = dValues2[1];
   // scaleZ_ = dValues1[2];
  nZ      = iValues[2];
  dZ     = dValues2[2];
  if (nX < 1) {
   throw Exception("Error: Number of samples in X-dir must be >= 1.\n");
  }
  if (nY < 1) {
    throw Exception("Error: Number of samples in Y-dir must be >= 1.\n");
  }
  if (nZ < 1) {
    throw Exception("Error: Number of samples in Z-dir must be >= 1.\n");
  }
  if (dX <= 0.0) {
    throw Exception("Error: Grid sampling in X-dir must be > 0.0.\n");

  }
  if (dY <= 0.0) {
    throw Exception("Error: Grid sampling in Y-dir must be > 0.0.\n");
  }
  if (dZ <= 0.0) {
    throw Exception("Error: Grid sampling in Z-dir must be > 0.0.\n");
  }

  double lx = nX*dX;
  double ly = nY*dY;
  //double lz = (nY-1)*dZ;

  double x_min = minValues[0]-0.5*dX;
  double y_min = minValues[1]-0.5*dY;
 // z0_ = -0.5f * (nZ-1)*dZ;
  SetDimensions(x_min,y_min,lx, ly);
  delete [] dValues1;
  delete [] dValues2;
  delete [] iValues;
  Resize(nX,nY,nZ);
  //Reading record 9+dim+ngrid: Angle of rotation

  double angle;
  headerFile >> angle;
 // SetAngle(angle*NRLib::PI/180.0);
  SetAngle(angle);
  double dipangle;
  headerFile >> dipangle;

  if(dipangle==0)
  {
    ConstantSurface<double> z_top(minValues[2]-0.5*dZ);
    ConstantSurface<double> z_bot(minValues[2]+nZ*dZ-0.5*dZ);
    SetSurfaces(z_top,z_bot);
    //z_top_ = new RegularSurface<double>(x_min_,y_min_,lx_,ly_,nX,nY,-0.5f*(nZ-1)*dZ);
   // z_bot_ = new RegularSurface<double>(x_min_,y_min_,lx_,ly_,nX,nY, 0.5f*(nZ-1)*dZ);
  }
  else
  {

    RegularSurfaceRotated<double> z_top(x_min,y_min,lx,ly,nX,nY,dipangle,minValues[2]-0.5*dZ);
    RegularSurfaceRotated<double> z_bot(x_min,y_min,lx,ly,nX,nY,dipangle,minValues[2]+nZ*dZ-0.5*dZ);
  /*  int i,j;
    for(i=0;i<nX;i++)
      for(j=0;j<nY;j++)
      {
        (*z_top)(i,j)+=i*nX*tan(dipangle);
        (*z_bot)(i,j)+=i*nX*tan(dipangle);
      }*/
    SetSurfaces(z_top,z_bot);
  }
  delete [] minValues;
  RecalculateLZ();
  //erosion_top_ = new ConstantSurface<double>(0.0);
  //erosion_bot_ = new ConstantSurface<double>(0.0);
  getline(headerFile, tmpStr);
  //Reading record 10+dim+ngrid: Undef value
  headerFile >> missing_code_;
  getline(headerFile, tmpStr);
  //Reading record 11+dim+ngrid: Filename of binary file.
  getline(headerFile, tmpStr);
  if (tmpStr.empty())
    binFileName = "";
  else {
    std::locale loc;
    int i = 0;
    char c = tmpStr[i];
    while (!std::isspace(c,loc)) {
      i++;
      c = tmpStr[i];
    }
    tmpStr.erase(tmpStr.begin()+i, tmpStr.end());
    binFileName = tmpStr;
  }
  //Reading record 12+dim+ngrid: Complex values
  bool hasComplex;
  headerFile >> hasComplex;
  if (hasComplex != 0 ) {
    throw Exception("Error: Can not read Sgri binary file. Complex values?");

  }
  //The remaining records are not relevant, stop reading


}
void StormContGrid::ReadSgriBinaryFile(const std::string& filename)
{
  std::ifstream binFile(filename.c_str(),std::ios::in | std::ios::binary); //Check opening of file before calling this function
  try {
    ReadBinaryFloatArray(binFile, begin(), GetN());
  }
  catch (Exception& e) {
    throw Exception("Error: Reading from binary sgri file " +filename + "." + e.what() +"\n");

  }

  return;

}

void
StormContGrid::WriteCravaFile(const std::string & file_name,
                              double              inline_0,
                              double              crossline_0,
                              double              il_step_x,
                              double              il_step_y,
                              double              xl_step_x,
                              double              xl_step_y)
{
  try {
    std::ofstream bin_file;
    //std::string f_name = file_name + IO::SuffixCrava();
    NRLib::OpenWrite(bin_file, file_name, std::ios::out | std::ios::binary);

    std::string file_type = "crava_fftgrid_binary";
    bin_file << file_type << "\n";

    NRLib::WriteBinaryDouble(bin_file, GetXMin());
    NRLib::WriteBinaryDouble(bin_file, GetYMin());
    NRLib::WriteBinaryDouble(bin_file, GetDX());
    NRLib::WriteBinaryDouble(bin_file, GetDY());
    NRLib::WriteBinaryInt(bin_file, static_cast<int>(GetNI()));
    NRLib::WriteBinaryInt(bin_file, static_cast<int>(GetNJ()));
    NRLib::WriteBinaryDouble(bin_file, inline_0);
    NRLib::WriteBinaryDouble(bin_file, crossline_0);
    NRLib::WriteBinaryDouble(bin_file, il_step_x);
    NRLib::WriteBinaryDouble(bin_file, il_step_y);
    NRLib::WriteBinaryDouble(bin_file, xl_step_x);
    NRLib::WriteBinaryDouble(bin_file, xl_step_y);
    NRLib::WriteBinaryDouble(bin_file, GetAngle());
    NRLib::WriteBinaryInt(bin_file, static_cast<int>(GetNI()));
    NRLib::WriteBinaryInt(bin_file, static_cast<int>(GetNJ()));
    NRLib::WriteBinaryInt(bin_file, static_cast<int>(GetNK()));

    //for(int i=0;i<rsize_;i++)
    //  NRLib::WriteBinaryFloat(binFile, rvalue_[i]);

    float value = 0.0f;
    for (size_t i = 0; i < GetNI(); i++) {
      for (size_t j = 0; j < GetNJ(); j++) {
        for (size_t k = 0; k < GetNK(); k++) {
          value = GetValue(i, j, k);
          NRLib::WriteBinaryFloat(bin_file, value);
        }
      }
    }

    bin_file.close();
  }
  catch (NRLib::Exception & e) {
    std::string message = "Error: "+std::string(e.what())+"\n";
    throw Exception(message);
  }
}

double StormContGrid::GetAvgRelThick(void) const
{
  double avgThick = 0.0f;
  for (int i = 0 ; i < static_cast<int>(GetNI()); i++) {
    for (int j = 0 ; j < static_cast<int>(GetNJ()) ; j++) {
      avgThick += GetRelThick(i, j);
    }
  }
  avgThick /= GetNI()*GetNJ();
  return avgThick;
}

double StormContGrid::GetRelThick(int i, int j) const
{
  double rx = (static_cast<double>(i) + 0.5)*GetDX();
  double ry = (static_cast<double>(j) + 0.5)*GetDY();
  double x = rx*cos(GetAngle())-ry*sin(GetAngle()) + GetXMin();
  double y = rx*sin(GetAngle())+ry*cos(GetAngle()) + GetYMin();
  return(GetRelThick(x, y));
}

double StormContGrid::GetRelThick(double x, double y) const
{
  double relThick = 1; //Default value to be used outside grid.
  double zTop = GetTopSurface().GetZ(x,y);
  double zBot = GetBotSurface().GetZ(x,y);
  if(GetTopSurface().IsMissing(zTop) == false &&
     GetBotSurface().IsMissing(zBot) == false)
    relThick = (zBot-zTop)/GetLZ();
  return(relThick);
}
