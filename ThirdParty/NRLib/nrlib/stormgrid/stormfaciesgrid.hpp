// $Id: stormfaciesgrid.hpp 883 2011-09-26 09:17:05Z perroe $

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

#ifndef NRLIB_STORMFACIESGRID_HPP
#define NRLIB_STORMFACIESGRID_HPP

#include "../volume/volume.hpp"
#include "../grid/grid.hpp"
#include "../iotools/fileio.hpp"

namespace NRLib {
  class StormFaciesGrid : public Grid<int>, public Volume {
  public:
    enum FileFormat {STORM_BINARY = 0, STORM_ASCII};

    explicit StormFaciesGrid(const std::string& filename,Endianess file_format = END_BIG_ENDIAN);
    explicit StormFaciesGrid(size_t nx = 0, size_t ny = 0, size_t nz = 0);
    explicit StormFaciesGrid(const Volume &vol, size_t nx, size_t ny, size_t nz);

    void SetMissingCode(int missing_code)
    { missing_code_ = missing_code; }
    int GetMissingCode() const
    { return missing_code_; }
    bool IsMissing(int val) const
    { return val == missing_code_; }

    void SetFormat(FileFormat format)
    { file_format_ = format; }

    FileFormat GetFormat() const
    { return file_format_; }

    void SetModelFileName(const std::string& filename)
    { model_file_name_ = filename; }

    std::string GetModelFileName() const
    { return model_file_name_; }

    void SetZoneNumber(int zone_number)
    { zone_number_ = zone_number; }

    int GetZoneNumber() const
    { return zone_number_; }

    int GetFaciesCode(int body_index) const;
    int GetFaciesFromCell(size_t cell_index) const;
    int GetFaciesFromCell(size_t i, size_t j, size_t k) const
    { return GetFaciesFromCell(GetIndex(i, j, k)); }
    const std::string& GetFaciesName(int facies_code) const;
    void SetBodyFacies(const std::vector<int>& body_facies);
    void SetFaciesNames(const std::vector<std::string>& facies_names);

    void WriteToFile(const std::string& filename, Endianess file_format = END_BIG_ENDIAN) const;
    void ReadFromFile(const std::string& filename, bool commonPath = true, Endianess file_format = END_BIG_ENDIAN);

    /// \brief Check that all the facies value for each body, and the
    ///        body values inside the grid are inside the given range.
    void CheckConsistency() const;

    double GetDX() const {return GetLX()/GetNI();}
    double GetDY() const {return GetLY()/GetNJ();}
    size_t FindIndex(double x, double y, double z) const;
    void   FindIndex( double x, double y, double z, size_t& i_out, size_t& j_out, size_t& k_out) const;
    void   FindCenterOfCell(size_t  i, size_t  j, size_t  k,
                            double& x, double& y, double& z) const;

  private:
    double RecalculateLZ();

    FileFormat file_format_;
    int missing_code_;
    int zone_number_;
    std::string model_file_name_;
    std::vector<std::string> facies_names_;
    std::vector<int> body_facies_;
  };
}

#endif // NRLIB_STORMFACIESGRID_HPP
