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

#include <opm/io/eclipse/EGrid.hpp>

#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <iomanip>
#include <numeric>
#include <string>
#include <sstream>

namespace Opm { namespace EclIO {

EGrid::EGrid(const std::string &filename) : EclFile(filename)
{
   EclFile file(filename);

   auto gridhead = get<int>("GRIDHEAD");

   nijk[0] = gridhead[1];
   nijk[1] = gridhead[2];
   nijk[2] = gridhead[3];

   if (file.hasKey("ACTNUM")) {
       auto actnum = get<int>("ACTNUM");

       nactive = 0;
       for (unsigned int i = 0; i < actnum.size(); i++) {
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

   coord_array = get<float>("COORD");
   zcorn_array = get<float>("ZCORN");
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
                           std::array<double,8>& X,
                           std::array<double,8>& Y,
                           std::array<double,8>& Z) const
{
    std::vector<int> zind;
    std::vector<int> pind;


   // calculate indices for grid pillars in COORD arrray
   pind.push_back(ijk[1]*(nijk[0]+1)*6 + ijk[0]*6);
   pind.push_back(pind[0] + 6);
   pind.push_back(pind[0] + (nijk[0]+1)*6);
   pind.push_back(pind[2] + 6);

   // get depths from zcorn array in ZCORN array
   zind.push_back(ijk[2]*nijk[0]*nijk[1]*8 + ijk[1]*nijk[0]*4 + ijk[0]*2);
   zind.push_back(zind[0] + 1);
   zind.push_back(zind[0] + nijk[0]*2);
   zind.push_back(zind[2] + 1);

   for (int n = 0; n < 4; n++) {
       zind.push_back(zind[n] + nijk[0]*nijk[1]*4);
   }

   for (int n = 0; n< 8; n++){
       Z[n] = zcorn_array[zind[n]];
   }

   for (int  n = 0; n < 4; n++) {
       double xt = coord_array[pind[n]];
       double yt = coord_array[pind[n] + 1];
       double zt = coord_array[pind[n] + 2];

       double xb = coord_array[pind[n] + 3];
       double yb = coord_array[pind[n] + 4];
       double zb = coord_array[pind[n]+5];

       X[n] = xt + (xb-xt) / (zt-zb) * (zt - Z[n]);
       X[n+4] = xt + (xb-xt) / (zt-zb) * (zt-Z[n+4]);

       Y[n] = yt+(yb-yt)/(zt-zb)*(zt-Z[n]);
       Y[n+4] = yt+(yb-yt)/(zt-zb)*(zt-Z[n+4]);
   }
}


void EGrid::getCellCorners(int globindex, std::array<double,8>& X,
                           std::array<double,8>& Y, std::array<double,8>& Z) const
{
    return getCellCorners(ijk_from_global_index(globindex),X,Y,Z);
}

}} // namespace Opm::ecl
