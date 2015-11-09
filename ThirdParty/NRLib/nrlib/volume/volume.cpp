// $Id: volume.cpp 1061 2012-09-13 09:09:57Z georgsen $

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

#include "volume.hpp"

#include <algorithm>
#include <cmath>
#include "../iotools/fileio.hpp"
#include "../iotools/stringtools.hpp"
#include "../math/constants.hpp"
#include "../surface/regularsurface.hpp"
#include "../surface/regularsurfacerotated.hpp"
#include "../surface/surface.hpp"
#include "../surface/surfaceio.hpp"

using namespace NRLib;

Volume::Volume()
{
  x_min_       = 0.0;
  y_min_       = 0.0;
  lx_          = 0.0;
  ly_          = 0.0;
  lz_          = 0.0;
  angle_       = 0.0;
  z_top_       = new ConstantSurface<double>(0.0);
  z_bot_       = new ConstantSurface<double>(0.0);
  tolerance_   = 1e-6;
}

Volume::Volume(double x_min, double y_min, double z_min, double lx, double ly, double lz, double angle)
: x_min_(x_min),
  y_min_(y_min),
  lx_(lx),
  ly_(ly),
  lz_(lz),
  angle_(angle)
{
  z_top_       = new ConstantSurface<double>(z_min);
  z_bot_       = new ConstantSurface<double>(z_min+lz);
  tolerance_   = 1e-6;
}


Volume::Volume(double                  x_min,
               double                  y_min,
               double                  lx,
               double                  ly,
               const Surface<double> & top,
               const Surface<double> & bot,
               double                  angle)
: x_min_(x_min),
  y_min_(y_min),
  lx_(lx),
  ly_(ly),
  angle_(angle)
{
  z_top_ = top.Clone(),
  z_bot_ = bot.Clone(),
  lz_ = RecalculateLZ();
  tolerance_   = 1e-6;
}


Volume::Volume(const Volume & volume)
{
  x_min_ = volume.x_min_;
  y_min_ = volume.y_min_;
  lx_    = volume.lx_;
  ly_    = volume.ly_;
  lz_    = volume.lz_;
  angle_ = volume.angle_;
  if (volume.z_top_ != 0) {
    z_top_ = volume.z_top_->Clone();
  }
  else {
    z_top_ = 0;
  }
  if (volume.z_bot_ != 0) {
    z_bot_ = volume.z_bot_->Clone();
  }
  else {
    z_bot_ = 0;
  }

  tolerance_   = 1e-6;
}


Volume::~Volume()
{
  delete z_top_;
  delete z_bot_;
}


Volume& Volume::operator=(const Volume& rhs)
{
  /// \todo Use "copy and swap" for exception safety.

  if (this == &rhs) return *this;

  x_min_ = rhs.x_min_;
  y_min_ = rhs.y_min_;
  lx_    = rhs.lx_;
  ly_    = rhs.ly_;
  lz_    = rhs.lz_;
  angle_ = rhs.angle_;
  tolerance_=rhs.tolerance_;

  delete z_top_;
  if (rhs.z_top_ != 0) {
    z_top_ = rhs.z_top_->Clone();
  }
  else {
    z_top_ = 0;
  }

  delete z_bot_;
  if (rhs.z_bot_ != 0) {
    z_bot_ = rhs.z_bot_->Clone();
  }
  else {
    z_bot_ = 0;
  }

  /*
  delete erosion_top_;
  if (rhs.erosion_top_ != 0) {
    erosion_top_ = rhs.erosion_top_->Clone();
  }
  else {
    erosion_top_ = 0;
  }

  delete erosion_bot_;
  if (rhs.erosion_bot_ != 0) {
    erosion_bot_ = rhs.erosion_bot_->Clone();
  }
  else {
    erosion_bot_ = 0;
  }
  */

  return *this;
}


void Volume::SetDimensions(double x_min, double y_min,
                           double lx, double ly)
{
  x_min_ = x_min;
  y_min_ = y_min;
  lx_ = lx;
  ly_ = ly;

  CheckSurfaces();
  lz_ = RecalculateLZ();
}

void Volume::SetAngle(double angle)
{
  angle_ = angle;

  CheckSurfaces();
  lz_ = RecalculateLZ();
}


void Volume::SetSurfaces(const Surface<double>& top_surf,
                         const Surface<double>& bot_surf,
                         bool  skip_check)
{
  delete z_top_;
  z_top_ = top_surf.Clone();

  delete z_bot_;
  z_bot_ = bot_surf.Clone();

  if ((lx_ > 0.0 || ly_ > 0.0 ) && skip_check == false) { //Make sure area is set, and we need to check
    CheckSurfaces();
  }

  lz_ = RecalculateLZ();
}


/*
void Volume::SetSurfaces(const Surface<double>& top_surf,
                         const Surface<double>& bot_surf)
                         //const Surface<double>& erosion_top,
                         //const Surface<double>& erosion_bot)
{
  delete z_top_;
  z_top_ = top_surf.Clone();

  delete z_bot_;
  z_bot_ = bot_surf.Clone();

  delete erosion_top_;
  erosion_top_ = erosion_top.Clone();

  delete erosion_bot_;
  erosion_bot_ = erosion_bot.Clone();

  if (lx_ > 0.0 || ly_ > 0.0 ) { //Check that area is set.
    CheckSurfaces();
  }

  lz_ = RecalculateLZ();
}
*/


double
Volume::GetZMin(size_t nx, size_t ny) const
{
  return(GetTopZMin(nx, ny));
}

double
Volume::GetZMax(size_t nx, size_t ny) const
{
  return(GetBotZMax(nx, ny));
}

double
Volume::GetTopZMin(size_t nx, size_t ny) const
{
  return(GetZExtreme(nx,ny,z_top_,true));
}


double
Volume::GetTopZMax(size_t nx, size_t ny) const
{
  return(GetZExtreme(nx,ny,z_top_,false));
}

double
Volume::GetBotZMin(size_t nx, size_t ny) const
{
  return(GetZExtreme(nx,ny,z_bot_,true));
}

double
Volume::GetBotZMax(size_t nx, size_t ny) const
{
  return(GetZExtreme(nx,ny,z_bot_,false));
}


double
Volume::GetZExtreme(size_t nx, size_t ny, const Surface<double> * surf, bool getmin) const
{
  if (surf == NULL)
    surf = z_top_;
  double result;
  if (lx_ > 0.0 || ly_ > 0.0 ) {//Check that area is set.
    size_t i, j;
    double di = lx_/static_cast<double>(nx);
    double dj = ly_/static_cast<double>(ny);
    double dxi = cos(angle_)*di;
    double dyi = sin(angle_)*di;
    double dxj = -sin(angle_)*dj;
    double dyj = cos(angle_)*dj;
    double x, y, z;
    double x0 = x_min_+0.5*(dxi+dxj); //Cell center x
    double y0 = y_min_+0.5*(dyi+dyj); //Cell center y
    result = surf->GetZ(x0,y0);
    for (j = 0; j < ny; j++) {
      x = x0 + dxj*j;
      y = y0 + dyj*j;
      for (i = 0; i < nx; i++) {
        z = surf->GetZ(x, y);
        if (surf->IsMissing(z) == false && (    //Found actual value.
           surf->IsMissing(result) == true ||   //Had no value
           (getmin == true  && z < result) ||   //Looking for min, found new.
           (getmin == false && z > result)))    //Looking for max, found new.
          result = z;
        x += dxi;
        y+= dyi;
      }
    }
  }
  else {
    if (getmin == true)
      result = surf->Min();
    else
      result = surf->Max();
  }
  return(result);
}

void
Volume::FindCenter(double & x, double & y, double & z) const
{
  x = GetXMin() + 0.5 * GetLX();
  y = GetYMin() + 0.5 * GetLY();
  z = 0.5 * (z_top_->GetZ(x,y) + z_bot_->GetZ(x,y));
}

// Writes surface to file if non-constant. Returns filename or
// surface level if surface is constant.
static std::string WriteSingleSurface(const Surface<double>* surf,
                                      const std::string& grid_filename,
                                      const std::string& surface_name,
                                      bool  remove_path
)
{
  if (surf == 0) {
    return "0";
  }

  if (typeid(*surf) == typeid(ConstantSurface<double>)) {
    return (ToString((dynamic_cast<const ConstantSurface<double>*>(surf))->GetZ()));
  }
  else if (typeid(*surf) == typeid(RegularSurface<double>)) {
    std::string filename = grid_filename + surface_name;
    /// \todo Fix this.
    // std::string filename = MainPart(grid_filename) + surface_name + ".s";
    const RegularSurface<double>* rsurf
      = dynamic_cast<const RegularSurface<double>*>(surf);
    rsurf->WriteToFile(filename, SURF_STORM_BINARY);
    if(remove_path)
      return RemovePath(filename);
    else
      return filename;
  }
  else if (typeid(*surf) == typeid(RegularSurfaceRotated<double>)) {
    std::string filename = grid_filename + surface_name;
    /// \todo Fix this.
    // std::string filename = MainPart(grid_filename) + surface_name + ".s";
    const RegularSurfaceRotated<double>* rsurf
      = dynamic_cast<const RegularSurfaceRotated<double>*>(surf);
    rsurf->WriteToFile(filename, SURF_STORM_BINARY);
    if(remove_path)
      return RemovePath(filename);
    else
      return filename;
  }
  else {
    throw Exception("Bug: Trying to write unsupported surface type to file.");
  }
}


void Volume::WriteVolumeToFile(std::ofstream& file,
                               const std::string& filename, bool remove_path) const
{
  file << x_min_ << " " << lx_ << " " << y_min_ << " " << ly_ << " "
       << WriteSingleSurface(z_top_, filename, "_top", remove_path) << " "
       << WriteSingleSurface(z_bot_, filename, "_bot", remove_path) << " "
//       << WriteSingleSurface(erosion_top_, filename, "_erosion_top", remove_path) << " "
//       << WriteSingleSurface(erosion_bot_, filename, "_erosion_bot", remove_path) << "\n"
       << GetLZ() << " " << (180.0*angle_)/NRLib::Pi << "\n";
}


void Volume::ReadVolumeFromFile(std::ifstream& file, int line, const std::string& path)
{
  x_min_ = ReadNext<double>(file, line);
  lx_    = ReadNext<double>(file, line);
  y_min_ = ReadNext<double>(file, line);
  ly_    = ReadNext<double>(file, line);
  bool topfile, botfile;//, toperofile, boterofile;
  std::string token = ReadNext<std::string>(file, line);
  delete z_top_;
  z_top_ = NULL;
  if (IsType<double>(token)) {
    z_top_ = new ConstantSurface<double>(ParseType<double>(token));
    topfile = false;
  } else {
    std::string path_file_name = NRLib::PrependDir(path, token);
    z_top_ = new RegularSurface<double>(path_file_name);
    topfile = true;
  }
  token = ReadNext<std::string>(file, line);
  delete z_bot_;
  if (IsType<double>(token)) {
    z_bot_ = new ConstantSurface<double>(ParseType<double>(token));
    botfile = false;
  } else {
    std::string path_file_name = NRLib::PrependDir(path, token);
    z_bot_ = new RegularSurface<double>(path_file_name);
    botfile = true;
  }
  token = ReadNext<std::string>(file, line);
  /*
  delete erosion_top_;
  if (IsType<double>(token)) {
    erosion_top_ = new ConstantSurface<double>(ParseType<double>(token));
    toperofile = false;
  } else {
    std::string path_file_name = NRLib::PrependDir(path, token);
    erosion_top_ = new RegularSurface<double>(path_file_name);
    toperofile = true;
  }*/
  token = ReadNext<std::string>(file, line);
  /*
  delete erosion_bot_;
  if (IsType<double>(token)) {
    erosion_bot_ = new ConstantSurface<double>(ParseType<double>(token));
    boterofile = false;
  } else {
    std::string path_file_name = NRLib::PrependDir(path, token);
    erosion_bot_ = new RegularSurface<double>(path_file_name);
    boterofile = true;
  }
  */

  lz_ = ReadNext<double>(file, line);
  angle_ = (NRLib::Pi * ReadNext<double>(file, line)) / 180.0;
  if(topfile == true){
    if (!CheckSurface(*z_top_)) {
      throw Exception("The top surface does not fit with the volume.");
    }
  }
  if(botfile == true){
    if (!CheckSurface(*z_bot_)) {
      throw Exception("The bottom surface does not fit with the volume.");
    }
  }
  /*
  if(toperofile == true){
    if (!CheckSurface(*erosion_top_)) {
      throw Exception("The erosion top surface does not fit with the volume.");
    }
  }
  if(boterofile == true){
    if (!CheckSurface(*erosion_bot_)) {
      throw Exception("The erosion bottom surface does not fit with the volume.");
    }
  }
  */

}


void Volume::GlobalToLocalCoord(double global_x,
                                double global_y,
                                double& local_x,
                                double& local_y) const
{
  double x_rel = global_x - x_min_;
  double y_rel = global_y - y_min_;

  local_x =   std::cos(angle_)*x_rel + std::sin(angle_)*y_rel;
  local_y = - std::sin(angle_)*x_rel + std::cos(angle_)*y_rel;
}


void Volume::LocalToGlobalCoord(double local_x,
                                     double local_y,
                                     double& global_x,
                                     double& global_y) const
{
  global_x = std::cos(angle_)*local_x - std::sin(angle_)*local_y + x_min_;
  global_y = std::sin(angle_)*local_x + std::cos(angle_)*local_y + y_min_;
}


double Volume::RecalculateLZ()
{
  double lz = 0.0;
  if (lx_ > 0.0 || ly_ > 0.0) { //Only do if area is initialized.
    // Just using a arbitary grid resolution.
    int nx = 100;
    int ny = 100;

    double dx = lx_ / nx;
    double dy = ly_ / ny;

    double z_value_bot = 0.0;
    double z_value_top = 0.0;

    for (int i = 0; i < nx; ++i) {
      for (int j = 0; j < ny; ++j) {
        double x, y;
        LocalToGlobalCoord(dx * i, dy * j, x, y);

        z_value_bot = z_bot_->GetZ(x, y);
        z_value_top = z_top_->GetZ(x, y);

        if (z_bot_->IsMissing(z_value_bot) == false && z_top_->IsMissing(z_value_top) == false)
          lz = std::max(lz, z_bot_->GetZ(x, y) - z_top_->GetZ(x, y));
      }
    }
  }
  return lz;
}


bool Volume::CheckSurface(const Surface<double>& surface) const
{
  std::vector<double> x(4);
  std::vector<double> y(4);

  LocalToGlobalCoord(  0,   0, x[0], y[0]);
  LocalToGlobalCoord(lx_,   0, x[1], y[1]);
  LocalToGlobalCoord(  0, ly_, x[2], y[2]);
  LocalToGlobalCoord(lx_, ly_, x[3], y[3]);

  double x_min = *(std::min_element(x.begin(), x.end()));
  double y_min = *(std::min_element(y.begin(), y.end()));
  double x_max = *(std::max_element(x.begin(), x.end()));
  double y_max = *(std::max_element(y.begin(), y.end()));

  return surface.EnclosesRectangle(x_min, x_max, y_min, y_max);
}


bool Volume::CheckSurfaces() const
{
  if (!CheckSurface(*z_top_)) {
    throw Exception("The top surface does not cover the volume.");
  }
  if (!CheckSurface(*z_bot_)) {
    throw Exception("The bottom surface does not cover the volume.");
  }
  /*
  if (erosion_top_ && !CheckSurface(*erosion_top_)) {
    throw Exception("The erosion top surface does not cover the volume.");
  }
  if (erosion_bot_ && !CheckSurface(*erosion_bot_)) {
    throw Exception("The erosion bottom surface does not cover the volume.");
  }
  */
  return true;
}


int Volume::IsInsideTolerance(double x, double y)const
{

  double rx = (x-x_min_)*cos(angle_)+(y-y_min_)*sin(angle_);
  double ry = -(x-x_min_)*sin(angle_) + (y-y_min_)*cos(angle_);
  if (rx+tolerance_ < 0.0 || rx-tolerance_ > lx_ || ry+tolerance_ <0.0 || ry-tolerance_ > ly_)
    return(0);
  else
    return(1);
}


int Volume::IsInside(double x, double y)const
{

  double rx = (x-x_min_)*cos(angle_)+(y-y_min_)*sin(angle_);
  double ry = -(x-x_min_)*sin(angle_) + (y-y_min_)*cos(angle_);
  if (rx < 0.0 || rx > lx_ || ry <0.0 || ry > ly_)
    return(0);
  else
    return(1);
}



bool Volume::IsInside(double x, double y, double z)const
{
  if (IsInside(x, y) &&
      z > z_top_->GetZ(x, y) && z < z_bot_->GetZ(x, y)) {
    return true;
  }
  return false;
}

bool Volume::IsInsideZTolerance(double x, double y, double z, double tolerance)const
{
  if (IsInside(x, y) &&
      z > (z_top_->GetZ(x, y) - tolerance) && z < (z_bot_->GetZ(x, y) + tolerance)) {
    return true;
  }
  return false;
}
