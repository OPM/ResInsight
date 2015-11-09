// $Id: stormcontgrid.hpp 1190 2013-07-03 10:57:27Z ulvmoen $

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

#ifndef NRLIB_STORMCONTGRID_HPP
#define NRLIB_STORMCONTGRID_HPP

#include <string>

#include "../volume/volume.hpp"
#include "../grid/grid.hpp"
#include "../iotools/fileio.hpp"

namespace NRLib {
  class StormContGrid : public Grid<float>, public Volume {
  public:
    enum FileFormat {STORM_BINARY = 0, STORM_ASCII};

    explicit StormContGrid(const std::string& filename, Endianess file_format = END_BIG_ENDIAN);
    explicit StormContGrid(size_t nx = 0, size_t ny = 0, size_t nz = 0);
    explicit StormContGrid(const Volume &vol, const Grid<float> & grid);
    explicit StormContGrid(const Volume &vol, size_t nx = 0, size_t ny = 0, size_t nz = 0);

    void SetMissingCode(float missing_code)
    { missing_code_ = missing_code; }

    float GetMissingCode() const
    { return missing_code_; }

    bool IsMissing(float val) const
    { return val == missing_code_; }

    void SetFormat(FileFormat format)
    { file_format_ = format; }

    FileFormat GetFormat() const
    { return file_format_; }

    void SetModelFileName(const std::string& filename)
    { model_file_name_ = filename; }

    std::string GetModelFileName() const
    { return model_file_name_; }

    void SetVariableName(const std::string& name)
    { variable_name_ = name; }

    std::string GetVariableName() const
    { return variable_name_; }

    void SetZoneNumber(int zone_number)
    { zone_number_ = zone_number; }

    int GetZoneNumber() const
    { return zone_number_; }

    /// Write to file. If predefinedHeader is not empty, this header is written instead
    ///                of standard, and surfaces are not written.
    void WriteToFile(const std::string& filename,
                     const std::string& predefinedHeader = "",
                     bool plainAscii=false,
                     Endianess file_format = END_BIG_ENDIAN,
                     bool remove_path = true) const;

    void WriteToSgriFile(const std::string & file_name,
                         const std::string & file_name_header,
                         const std::string & label,
                         double              simbox_dz,
                         Endianess           file_format = END_BIG_ENDIAN) const;

    /// \throw IOError if the file can not be opened.
    /// \throw FileFormatError if file format is not either storm_binary or storm_ascii, or if grid contains barriers.
    void ReadFromFile(const std::string& filename, bool commonPath = true, Endianess file_format = END_BIG_ENDIAN);

    double GetDX() const       { return GetLX() / GetNI(); }
    double GetDY() const       { return GetLY() / GetNJ(); }
    size_t FindIndex(double x, double y, double z) const;
    void   FindIndex( double x, double y, double z, size_t& i_out, size_t& j_out, size_t& k_out) const;
    void   FindXYIndex(double x, double y, size_t& i_out, size_t& j_out) const;

    void   FindZInterpolatedIndex(const double & x,
                                  const double & y,
                                  const double & z,
                                  size_t       & ind1,
                                  size_t       & ind2,
                                  double       & t) const;

    float GetValueZInterpolatedFromIndex(const size_t & ind1,
                                         const size_t & ind2,
                                         const double & t) const;

    double GetValueZInterpolatedFromIndexNoMissing(const size_t & ind1,
                                                   const size_t & ind2,
                                                   const double & t) const;

    float GetValueInterpolated(const int   & i,
                               const int   & j,
                               const float & k_value) const;

    double GetAvgRelThick() const;

    float  GetValueZInterpolated(double x, double y, double z)const;
    float  GetValueClosestInZ(double x, double y, double z)const;
    double GetZMin()const      { return Volume::GetZMin(GetNI(), GetNJ()); }
    double GetZMax()const      { return Volume::GetZMax(GetNI(), GetNJ()); }

    void   FindCenterOfCell(size_t  i, size_t  j, size_t  k,
                            double& x, double& y, double& z) const;

    void WriteCravaFile(const std::string & file_name,
                        double              inline_0,
                        double              crossline_0,
                        double              il_step_x,
                        double              il_step_y,
                        double              xl_step_x,
                        double              xl_step_y);

  private:
    double RecalculateLZ();
    void ReadSgriHeader(std::ifstream &headerFile, std::string &binFileName);
    void ReadSgriBinaryFile(const std::string& filename);

    double GetRelThick(int i, int j) const;
    double GetRelThick(double x, double y) const;

    FileFormat file_format_;
    float missing_code_;
    int zone_number_;
    std::string model_file_name_;
    std::string variable_name_;
};

}

#endif // NRLIB_STORMCONTGRID_HPP
