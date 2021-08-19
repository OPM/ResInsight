/*
   Copyright 2019 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/EInit.hpp>
#include <opm/io/eclipse/EclUtil.hpp>

#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Opm
{
namespace EclIO
{

    using NNCentry = std::tuple<int, int, int, int, int, int, float>;

    EGrid::EGrid(const std::string& filename, std::string grid_name)
        : EclFile(filename)
        , inputFileName {filename}
        , m_grid_name {grid_name}
    {
        initFileName = inputFileName.parent_path() / inputFileName.stem();

        if (this->formattedInput())
            initFileName += ".FINIT";
        else
            initFileName += ".INIT";

        std::string lgrname = "global";

        m_nncs_loaded = false;
        actnum_array_index = -1;
        nnc1_array_index = -1;
        nnc2_array_index = -1;
        m_radial = false;
        m_mapunits = "";

        int hostnum_index = -1;

        for (size_t n = 0; n < array_name.size(); n++) {

            if (array_name[n] == "ENDLGR")
                lgrname = "global";

            if (array_name[n] == "LGR") {
                auto lgr = this->get<std::string>(n);
                lgrname = lgr[0];
                lgr_names.push_back(lgr[0]);
            }

            if (array_name[n] == "NNCHEAD") {
                auto nnchead = this->get<int>(n);

                if (nnchead[1] == 0)
                    lgrname = "global";
                else
                    lgrname = lgr_names[nnchead[1] - 1];
            }

            if (array_name[n] == "MAPUNITS") {
                auto mapunits = this->get<std::string>(n);
                m_mapunits = mapunits[0];
            }

            if (array_name[n] == "MAPAXES")
                m_mapaxes = this->get<float>(n);

            if (lgrname == grid_name) {
                if (array_name[n] == "GRIDHEAD") {
                    auto gridhead = get<int>(n);
                    nijk[0] = gridhead[1];
                    nijk[1] = gridhead[2];
                    nijk[2] = gridhead[3];

                    if (gridhead.size() > 26)
                        m_radial = gridhead[26] > 0 ? true : false;
                }

                if (array_name[n] == "COORD")
                    coord_array_index = n;
                else if (array_name[n] == "ZCORN")
                    zcorn_array_index = n;
                else if (array_name[n] == "ACTNUM")
                    actnum_array_index = n;
                else if (array_name[n] == "NNC1")
                    nnc1_array_index = n;
                else if (array_name[n] == "NNC2")
                    nnc2_array_index = n;
                else if (array_name[n] == "HOSTNUM")
                    hostnum_index = n;
            }

            if ((lgrname == "global") && (array_name[n] == "GRIDHEAD")) {
                auto gridhead = get<int>(n);
                host_nijk[0] = gridhead[1];
                host_nijk[1] = gridhead[2];
                host_nijk[2] = gridhead[3];
            }
        }

        if (actnum_array_index != -1) {
            auto actnum = this->get<int>(actnum_array_index);
            nactive = 0;
            for (size_t i = 0; i < actnum.size(); i++) {
                if (actnum[i] > 0) {
                    act_index.push_back(nactive);
                    glob_index.push_back(i);
                    nactive++;
                } else {
                    act_index.push_back(-1);
                }
            }
        } else {
            int nCells = nijk[0] * nijk[1] * nijk[2];
            act_index.resize(nCells);
            glob_index.resize(nCells);
            std::iota(act_index.begin(), act_index.end(), 0);
            std::iota(glob_index.begin(), glob_index.end(), 0);
        }

        if (hostnum_index > -1) {
            auto hostnum = getImpl(hostnum_index, INTE, inte_array, "integer");
            host_cells.reserve(hostnum.size());

            for (auto val : hostnum)
                host_cells.push_back(val - 1);
        }
    }

    std::vector<std::array<int, 3>> EGrid::hostCellsIJK()
    {
        std::vector<std::array<int, 3>> res_vect;
        res_vect.reserve(host_cells.size());

        for (auto val : host_cells) {
            std::array<int, 3> tmp;
            tmp[2] = val / (host_nijk[0] * host_nijk[1]);
            int rest = val % (host_nijk[0] * host_nijk[1]);

            tmp[1] = rest / host_nijk[0];
            tmp[0] = rest % host_nijk[0];

            res_vect.push_back(tmp);
        }

        return res_vect;
    }

    std::vector<NNCentry> EGrid::get_nnc_ijk()
    {
        if (!m_nncs_loaded)
            load_nnc_data();

        std::vector<NNCentry> res_vect;
        res_vect.reserve(nnc1_array.size());

        for (size_t n = 0; n < nnc1_array.size(); n++) {
            auto ijk1 = ijk_from_global_index(nnc1_array[n] - 1);
            auto ijk2 = ijk_from_global_index(nnc2_array[n] - 1);

            if (transnnc_array.size() > 0)
                res_vect.push_back({ijk1[0], ijk1[1], ijk1[2], ijk2[0], ijk2[1], ijk2[2], transnnc_array[n]});
            else
                res_vect.push_back({ijk1[0], ijk1[1], ijk1[2], ijk2[0], ijk2[1], ijk2[2], -1.0});
        }

        return res_vect;
    }


    void EGrid::load_grid_data()
    {
        coord_array = getImpl(coord_array_index, REAL, real_array, "float");
        zcorn_array = getImpl(zcorn_array_index, REAL, real_array, "float");
    }

    void EGrid::load_nnc_data()
    {
        if ((nnc1_array_index > -1) && (nnc2_array_index > -1)) {

            nnc1_array = getImpl(nnc1_array_index, Opm::EclIO::INTE, inte_array, "inte");
            nnc2_array = getImpl(nnc2_array_index, Opm::EclIO::INTE, inte_array, "inte");

            if ((Opm::filesystem::exists(initFileName)) && (nnc1_array.size() > 0)) {
                Opm::EclIO::EInit init(initFileName.string());

                auto init_dims = init.grid_dimension(m_grid_name);
                int init_nactive = init.activeCells(m_grid_name);

                if (init_dims != nijk) {
                    std::string message = "Dimensions of Egrid differ from dimensions found in init file. ";
                    std::string grid_str
                        = std::to_string(nijk[0]) + "x" + std::to_string(nijk[1]) + "x" + std::to_string(nijk[2]);
                    std::string init_str = std::to_string(init_dims[0]) + "x" + std::to_string(init_dims[1]) + "x"
                        + std::to_string(init_dims[2]);
                    message = message + "Egrid: " + grid_str + ". INIT file: " + init_str;
                    OPM_THROW(std::invalid_argument, message);
                }

                if (init_nactive != nactive) {
                    std::string message = "Number of active cells are different in Egrid and Init file.";
                    message = message + " Egrid: " + std::to_string(nactive)
                        + ". INIT file: " + std::to_string(init_nactive);
                    OPM_THROW(std::invalid_argument, message);
                }

                auto trans_data = init.getInitData<float>("TRANNNC", m_grid_name);

                if (trans_data.size() != nnc1_array.size()) {
                    std::string message = "inconsistent size of array TRANNNC in init file. ";
                    message = message + " Size of NNC1 and NNC2: " + std::to_string(nnc1_array.size());
                    message = message + " Size of TRANNNC: " + std::to_string(trans_data.size());
                    OPM_THROW(std::invalid_argument, message);
                }

                transnnc_array = trans_data;
            }

            m_nncs_loaded = true;
        }
    }

    int EGrid::global_index(int i, int j, int k) const
    {
        if (i < 0 || i >= nijk[0] || j < 0 || j >= nijk[1] || k < 0 || k >= nijk[2]) {
            OPM_THROW(std::invalid_argument, "i, j or/and k out of range");
        }

        return i + j * nijk[0] + k * nijk[0] * nijk[1];
    }


    int EGrid::active_index(int i, int j, int k) const
    {
        int n = i + j * nijk[0] + k * nijk[0] * nijk[1];

        if (i < 0 || i >= nijk[0] || j < 0 || j >= nijk[1] || k < 0 || k >= nijk[2]) {
            OPM_THROW(std::invalid_argument, "i, j or/and k out of range");
        }

        return act_index[n];
    }


    std::array<int, 3> EGrid::ijk_from_active_index(int actInd) const
    {
        if (actInd < 0 || actInd >= nactive) {
            OPM_THROW(std::invalid_argument, "active index out of range");
        }

        int _glob = glob_index[actInd];

        std::array<int, 3> result;
        result[2] = _glob / (nijk[0] * nijk[1]);

        int rest = _glob % (nijk[0] * nijk[1]);

        result[1] = rest / nijk[0];
        result[0] = rest % nijk[0];

        return result;
    }


    std::array<int, 3> EGrid::ijk_from_global_index(int globInd) const
    {
        if (globInd < 0 || globInd >= nijk[0] * nijk[1] * nijk[2]) {
            OPM_THROW(std::invalid_argument, "global index out of range");
        }

        std::array<int, 3> result;
        result[2] = globInd / (nijk[0] * nijk[1]);

        int rest = globInd % (nijk[0] * nijk[1]);

        result[1] = rest / nijk[0];
        result[0] = rest % nijk[0];

        return result;
    }


    void EGrid::getCellCorners(const std::array<int, 3>& ijk,
                               std::array<double, 8>& X,
                               std::array<double, 8>& Y,
                               std::array<double, 8>& Z)
    {
        if (coord_array.empty())
            load_grid_data();

        std::vector<int> zind;
        std::vector<int> pind;

        // calculate indices for grid pillars in COORD arrray
        pind.push_back(ijk[1] * (nijk[0] + 1) * 6 + ijk[0] * 6);
        pind.push_back(pind[0] + 6);
        pind.push_back(pind[0] + (nijk[0] + 1) * 6);
        pind.push_back(pind[2] + 6);

        // get depths from zcorn array in ZCORN array
        zind.push_back(ijk[2] * nijk[0] * nijk[1] * 8 + ijk[1] * nijk[0] * 4 + ijk[0] * 2);
        zind.push_back(zind[0] + 1);
        zind.push_back(zind[0] + nijk[0] * 2);
        zind.push_back(zind[2] + 1);

        for (int n = 0; n < 4; n++)
            zind.push_back(zind[n] + nijk[0] * nijk[1] * 4);

        for (int n = 0; n < 8; n++)
            Z[n] = zcorn_array[zind[n]];

        for (int n = 0; n < 4; n++) {
            double xt;
            double yt;
            double xb;
            double yb;

            double zt = coord_array[pind[n] + 2];
            double zb = coord_array[pind[n] + 5];

            if (m_radial) {
                xt = coord_array[pind[n]] * cos(coord_array[pind[n] + 1] / 180.0 * M_PI);
                yt = coord_array[pind[n]] * sin(coord_array[pind[n] + 1] / 180.0 * M_PI);
                xb = coord_array[pind[n] + 3] * cos(coord_array[pind[n] + 4] / 180.0 * M_PI);
                yb = coord_array[pind[n] + 3] * sin(coord_array[pind[n] + 4] / 180.0 * M_PI);
            } else {
                xt = coord_array[pind[n]];
                yt = coord_array[pind[n] + 1];
                xb = coord_array[pind[n] + 3];
                yb = coord_array[pind[n] + 4];
            }

            X[n] = xt + (xb - xt) / (zt - zb) * (zt - Z[n]);
            X[n + 4] = xt + (xb - xt) / (zt - zb) * (zt - Z[n + 4]);

            Y[n] = yt + (yb - yt) / (zt - zb) * (zt - Z[n]);
            Y[n + 4] = yt + (yb - yt) / (zt - zb) * (zt - Z[n + 4]);
        }
    }



    void
    EGrid::getCellCorners(int globindex, std::array<double, 8>& X, std::array<double, 8>& Y, std::array<double, 8>& Z)
    {
        return getCellCorners(ijk_from_global_index(globindex), X, Y, Z);
    }


    std::vector<std::array<float, 3>> EGrid::getXYZ_layer(int layer, std::array<int, 4>& box, bool bottom)
    {
        // layer is layer index, zero based. The box array is i and j range (i1,i2,j1,j2), also zero based

        if ((layer < 0) || (layer > (nijk[2] - 1))) {
            std::string message = "invalid layer index " + std::to_string(layer) + ". Valied range [0, ";
            message = message + std::to_string(nijk[2] - 1) + "]";
            throw std::invalid_argument(message);
        }

        if ((box[0] < 0) || (box[0] + 1 > nijk[0]) || (box[1] < 0) || (box[1] + 1 > nijk[0]) || (box[2] < 0)
            || (box[2] + 1 > nijk[1]) || (box[3] < 0) || (box[3] + 1 > nijk[1]) || (box[0] > box[1])
            || (box[2] > box[3])) {

            throw std::invalid_argument("invalid box input, i1,i2,j1 or j2 out of valied range ");
        }

        int nodes_pr_surf = nijk[0] * nijk[1] * 4;
        int zcorn_offset = nodes_pr_surf * layer * 2;

        if (bottom)
            zcorn_offset += nodes_pr_surf;

        std::vector<float> layer_zcorn;
        layer_zcorn.reserve(nodes_pr_surf);

        std::vector<std::array<float, 3>> xyz_vector;

        if (coord_array.size() == 0)
            coord_array = getImpl(coord_array_index, REAL, real_array, "float");

        if (zcorn_array.size() > 0) {
            for (size_t n = 0; n < static_cast<size_t>(nodes_pr_surf); n++)
                layer_zcorn.push_back(zcorn_array[zcorn_offset + n]);

        } else {
            layer_zcorn = get_zcorn_from_disk(layer, bottom);
        }

        std::array<double, 4> X;
        std::array<double, 4> Y;
        std::array<double, 4> Z;

        std::array<int, 3> ijk;
        ijk[2] = 0;

        for (int j = box[2]; j < (box[3] + 1); j++) {
            for (int i = box[0]; i < (box[1] + 1); i++) {

                ijk[0] = i;
                ijk[1] = j;

                this->getCellCorners(ijk, layer_zcorn, X, Y, Z);

                for (size_t n = 0; n < 4; n++) {
                    std::array<float, 3> xyz;
                    xyz[0] = X[n];
                    xyz[1] = Y[n];
                    xyz[2] = Z[n];

                    xyz_vector.push_back(xyz);
                }
            }
        }

        return xyz_vector;
    }


    std::vector<std::array<float, 3>> EGrid::getXYZ_layer(int layer, bool bottom)
    {
        std::array<int, 4> box = {0, nijk[0] - 1, 0, nijk[1] - 1};
        return this->getXYZ_layer(layer, box, bottom);
    }

    std::vector<float> EGrid::get_zcorn_from_disk(int layer, bool bottom)
    {
        if (formatted)
            throw std::invalid_argument("partial loading of zcorn arrays not possible when using formatted input");

        std::vector<float> zcorn_layer;
        std::fstream fileH;

        int nodes_pr_surf = nijk[0] * nijk[1] * 4;
        int zcorn_offset = nodes_pr_surf * layer * 2;

        if (bottom)
            zcorn_offset += nodes_pr_surf;

        fileH.open(inputFileName, std::ios::in | std::ios::binary);

        if (!fileH)
            throw std::runtime_error("Can not open EGrid file" + this->inputFilename);

        std::string arrName(8, ' ');
        eclArrType arrType;
        int64_t num;
        int sizeOfElement;

        uint64_t zcorn_pos = 0;

        while (!isEOF(&fileH)) {

            readBinaryHeader(fileH, arrName, num, arrType, sizeOfElement);

            if (arrName == "ZCORN   ") {
                zcorn_pos = fileH.tellg();
                break;
            }

            uint64_t sizeOfNextArray = sizeOnDiskBinary(num, arrType, sizeOfElement);
            fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);
        }

        int elements_pr_block = Opm::EclIO::MaxBlockSizeReal / Opm::EclIO::sizeOfReal;
        int num_blocks_start = zcorn_offset / elements_pr_block;

        // adding size of zcorn real data before to ignored
        uint64_t start_pos = zcorn_pos + Opm::EclIO::sizeOfReal * zcorn_offset;

        // adding size of blocks (head and tail flags)
        start_pos = start_pos + (1 + num_blocks_start * 2) * Opm::EclIO::sizeOfInte;

        fileH.seekg(start_pos, std::ios_base::beg);

        float value;
        int zcorn_to = zcorn_offset + nodes_pr_surf;

        for (int ind = zcorn_offset; ind < zcorn_to; ind++) {

            if ((ind > 0) && ((ind % elements_pr_block) == 0))
                fileH.seekg(static_cast<std::streamoff>(2 * Opm::EclIO::sizeOfInte), std::ios_base::cur);

            fileH.read(reinterpret_cast<char*>(&value), Opm::EclIO::sizeOfReal);
            value = Opm::EclIO::flipEndianFloat(value);

            zcorn_layer.push_back(value);
        }

        fileH.close();

        return zcorn_layer;
    }

    void EGrid::getCellCorners(const std::array<int, 3>& ijk,
                               const std::vector<float>& zcorn_layer,
                               std::array<double, 4>& X,
                               std::array<double, 4>& Y,
                               std::array<double, 4>& Z)
    {
        std::vector<int> zind;
        std::vector<int> pind;

        // calculate indices for grid pillars in COORD arrray
        pind.push_back(ijk[1] * (nijk[0] + 1) * 6 + ijk[0] * 6);
        pind.push_back(pind[0] + 6);
        pind.push_back(pind[0] + (nijk[0] + 1) * 6);
        pind.push_back(pind[2] + 6);

        // get depths from zcorn array in ZCORN array
        zind.push_back(ijk[2] * nijk[0] * nijk[1] * 8 + ijk[1] * nijk[0] * 4 + ijk[0] * 2);
        zind.push_back(zind[0] + 1);
        zind.push_back(zind[0] + nijk[0] * 2);
        zind.push_back(zind[2] + 1);

        for (int n = 0; n < 4; n++)
            Z[n] = zcorn_layer[zind[n]];

        for (int n = 0; n < 4; n++) {
            double xt;
            double yt;
            double xb;
            double yb;

            double zt = coord_array[pind[n] + 2];
            double zb = coord_array[pind[n] + 5];

            xt = coord_array[pind[n]];
            yt = coord_array[pind[n] + 1];
            xb = coord_array[pind[n] + 3];
            yb = coord_array[pind[n] + 4];

            if (zt == zb) {
                X[n] = xt;
                Y[n] = yt;
            } else {
                X[n] = xt + (xb - xt) / (zt - zb) * (zt - Z[n]);
                Y[n] = yt + (yb - yt) / (zt - zb) * (zt - Z[n]);
            }
        }
    }



} // namespace EclIO
} // namespace Opm
