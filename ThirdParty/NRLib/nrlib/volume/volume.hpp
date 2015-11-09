// $Id: volume.hpp 1061 2012-09-13 09:09:57Z georgsen $

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

#ifndef NRLIB_VOLUME_HPP
#define NRLIB_VOLUME_HPP

#include <fstream>

namespace NRLib {
  template <class A>
  class Surface;

  class Volume {
  public:
    Volume();
    Volume(double x_min, double y_min, double z_min, double lx, double ly, double lz, double angle);

    /// \brief Constructor with surfaces.
    Volume(double                  x_min,
           double                  y_min,
           double                  lx,
           double                  ly,
           const Surface<double> & top,
           const Surface<double> & bot,
           double                  angle);

    Volume(const Volume& volume);
    virtual ~Volume();
    Volume& operator=(const Volume& rhs);

    void SetDimensions(double x_min, double y_min,
                       double lx, double ly);
    void SetAngle(double angle);
    void SetTolerance(double tolerance){tolerance_=tolerance;}
    double GetTolerance(){return tolerance_;}

    double GetXMin() const {return x_min_;}
    double GetYMin() const {return y_min_;}

    /// \brief Get extreme values of z for volume, with nx, ny resolution.
    double GetZMin(size_t nx, size_t ny) const;
    double GetZMax(size_t nx, size_t ny) const;

    double GetLX() const {return lx_;}
    double GetLY() const {return ly_;}
    double GetAngle() const {return angle_;}

    /// \brief Maximum height of grid.
    double GetLZ() const {return lz_;}

    /// rel_x and rel_y in [0,1].
    void GetXYFromRelative(double rel_x, double rel_y, double &x, double &y) const{
      LocalToGlobalCoord(rel_x*lx_, rel_y*ly_, x, y);
    }

    /// \brief Get z-range for top and bottom surfaces, with nx, ny resolution.
    double GetTopZMin(size_t nx, size_t ny) const; //Equal to GetZMin
    double GetTopZMax(size_t nx, size_t ny) const;
    double GetBotZMin(size_t nx, size_t ny) const;
    double GetBotZMax(size_t nx, size_t ny) const; //Equal to GetZMax

    /// \brief Set surfaces.
    void SetSurfaces(const Surface<double>& top_surf,
                     const Surface<double>& bot_surf,
                     bool  skip_check = true); //Sometimes, we do not want an area cover check here.

    /*
    void SetSurfaces(const Surface<double>& top_surf,
                     const Surface<double>& bot_surf);
                     const Surface<double>& erosion_top,
                     const Surface<double>& erosion_bot);
                     */

    const Surface<double> & GetTopSurface()       const {return *z_top_;}
    const Surface<double> & GetBotSurface()       const {return *z_bot_;}
    //const Surface<double>& GetErosionTop() const {return *erosion_top_;}
    //const Surface<double>& GetErosionBot() const {return *erosion_bot_;}
    Surface<double>       & GetTopSurface()             {return *z_top_;}
    Surface<double>       & GetBotSurface()             {return *z_bot_;}

    int IsInside(double x, double y) const;
    bool IsInside(double x, double y, double z)const;
    int IsInsideTolerance(double x, double y)const;
    bool IsInsideZTolerance(double x, double y, double z, double tolerance) const;

    void FindCenter(double & x, double & y, double & z) const;

    /// \brief Checks if surface covers the whole volume.
    bool CheckSurface(const Surface<double>& surface) const;

  protected:
    /// \brief Reader and writer on storm-format.
    /// \todo  Maybe move to storm-specific files.
    void WriteVolumeToFile(std::ofstream& file,
                           const std::string& filename,
                           bool remove_path = true) const;
    void ReadVolumeFromFile(std::ifstream& file, int line, const std::string& path);

    /// \brief The local coorinates are (0,0) in (x_min, y_min), and
    ///        have the same orientation as the volume.
    void GlobalToLocalCoord(double global_x, double global_y,
                            double& local_x, double& local_y) const;

    void LocalToGlobalCoord(double local_x, double local_y,
                            double& global_x, double& global_y) const;

  private:
    virtual double RecalculateLZ();
    /// \brief Check all surfaces.
    bool CheckSurfaces() const;
    /// \brief Returns min or max of surface on nx by ny grid in volume.
    double GetZExtreme(size_t nx, size_t ny, const Surface<double> * surf, bool getmin) const;

    double x_min_;
    double y_min_;
    double lx_;
    double ly_;
    double lz_;
    Surface<double>* z_top_;
    Surface<double>* z_bot_;
    //Surface<double>* erosion_top_;
    //Surface<double>* erosion_bot_;
    double angle_;
    double tolerance_;
  };
} // namespace NRLib

#endif // NRLIB_VOLUME_HPP
