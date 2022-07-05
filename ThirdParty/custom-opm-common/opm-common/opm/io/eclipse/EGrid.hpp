/*
   Copyright 2019 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#ifndef OPM_IO_EGRID_HPP
#define OPM_IO_EGRID_HPP

#include <opm/io/eclipse/EclFile.hpp>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <map>

namespace Opm { namespace EclIO {

class EGrid : public EclFile
{
public:
    explicit EGrid(const std::string& filename, std::string grid_name = "global");

    int global_index(int i, int j, int k) const;
    int active_index(int i, int j, int k) const;

    const std::array<int, 3>& dimension() const { return nijk; }

    std::array<int, 3> ijk_from_active_index(int actInd) const;
    std::array<int, 3> ijk_from_global_index(int globInd) const;

    void getCellCorners(int globindex, std::array<double,8>& X, std::array<double,8>& Y, std::array<double,8>& Z);
    void getCellCorners(const std::array<int, 3>& ijk, std::array<double,8>& X, std::array<double,8>& Y, std::array<double,8>& Z);

    std::vector<std::array<float, 3>> getXYZ_layer(int layer, bool bottom=false);
    std::vector<std::array<float, 3>> getXYZ_layer(int layer, const std::array<int, 4>& box, bool bottom=false);

    int activeCells() const { return nactive; }
    int totalNumberOfCells() const { return nijk[0] * nijk[1] * nijk[2]; }

    void load_grid_data();
    void load_nnc_data();
    bool is_radial() const { return m_radial; }

    const std::vector<int>& hostCellsGlobalIndex() const { return host_cells; }
    std::vector<std::array<int, 3>> hostCellsIJK();

    // zero based: i1, j1,k1, i2,j2,k2, transmisibility
    using NNCentry = std::tuple<int, int, int, int, int, int, float>;
    std::vector<NNCentry> get_nnc_ijk();

    const std::vector<std::string>& list_of_lgrs() const { return lgr_names; }

    const std::vector<float>& get_mapaxes() const { return m_mapaxes; }
    const std::string& get_mapunits() const { return m_mapunits; }

private:
    std::filesystem::path inputFileName, initFileName;
    std::string m_grid_name;
    bool m_radial;

    std::vector<float> m_mapaxes;
    std::string m_mapunits;

    std::array<int, 3> nijk;
    std::array<int, 3> host_nijk;

    int nactive;
    mutable bool m_nncs_loaded;

    std::vector<int> act_index;
    std::vector<int> glob_index;

    std::vector<float> coord_array;
    std::vector<float> zcorn_array;

    std::vector<int> nnc1_array;
    std::vector<int> nnc2_array;
    std::vector<float> transnnc_array;
    std::vector<int> host_cells;

    std::vector<std::string> lgr_names;

    int zcorn_array_index;
    int coord_array_index;
    int actnum_array_index;
    int nnc1_array_index;
    int nnc2_array_index;

    std::vector<float> get_zcorn_from_disk(int layer, bool bottom);

    void getCellCorners(const std::array<int, 3>& ijk, const std::vector<float>& zcorn_layer,
                           std::array<double,4>& X, std::array<double,4>& Y, std::array<double,4>& Z);

};

}} // namespace Opm::EclIO

#endif // OPM_IO_EGRID_HPP
