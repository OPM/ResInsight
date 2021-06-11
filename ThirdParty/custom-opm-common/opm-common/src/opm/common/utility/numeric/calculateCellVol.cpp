/*
  Copyright 2018 Statoil ASA.

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <algorithm>
#include <cassert>

#define _USE_MATH_DEFINES
#include <cmath>

#include <opm/common/utility/numeric/calculateCellVol.hpp>
#include <opm/common/ErrorMacros.hpp>

/*
    Cell volume calculation based on following publication:

    D. K Pointing, Corner Point Geometry in Reservoir Simulation ,
    ECMOR I - 1st European Conference on the Mathematics of Oil Recovery,
    1989
*/


/*
    The expressions {C(0,0,0),..C(1,1,1)} have a nice interpretation in
    terms of a type of multipole expansion - the last four terms are differences
    in the lengths of cell face diagonals and of the diagonals across the
    cell. For a cubical block only the first four terms would exist.

*/

double C(const double* r, int i1, int i2, int i3){
   int g = i1 + i2 * 2 + i3 * 4;

   if (g == 0)
       return r[0];

   if (g == 1)
       return r[1] - r[0];

   if (g == 2)
       return r[2] - r[0];

   if (g == 3)
       return r[3] + r[0] - r[2] - r[1];

   if (g == 4)
       return r[4] - r[0];

   if (g == 5)
       return r[5] + r[0] - r[4] - r[1];

   if (g == 6)
       return r[6] + r[0] - r[4] - r[2];

   return  r[7] + r[4] + r[2] + r[1] - r[6] - r[5] - r[3] - r[0];
}


struct pqr_t {
    int pb;
    int pg;
    int qa;
    int qg;
    int ra;
    int rb;
};


double calculateCellVol(const std::array<double,8>& X, const std::array<double,8>& Y, const std::array<double,8>& Z){
    /*
      The permutation array should be ordered so that the sign:

         sign = (-1)^N, N = # permutations

      is alternating - so that the sign can just be changed multiplying with -1.
    */
    static const std::array< std::array<std::size_t, 3>, 6 > permutation = {{{ 0, 1, 2},
                                                                             { 0, 2, 1},
                                                                             { 1, 2, 0},
                                                                             { 1, 0, 2},
                                                                             { 2, 0, 1},
                                                                             { 2, 1, 0}}};


    static const std::array<pqr_t, 64> pqr_array
        = {{{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 1, 1},
            {0, 0, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 1}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 1, 1, 1},
            {0, 0, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 1}, {0, 0, 1, 0, 1, 0}, {0, 0, 1, 0, 1, 1},
            {0, 0, 1, 1, 0, 0}, {0, 0, 1, 1, 0, 1}, {0, 0, 1, 1, 1, 0}, {0, 0, 1, 1, 1, 1},
            {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 1}, {0, 1, 0, 0, 1, 0}, {0, 1, 0, 0, 1, 1},
            {0, 1, 0, 1, 0, 0}, {0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 0}, {0, 1, 0, 1, 1, 1},
            {0, 1, 1, 0, 0, 0}, {0, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 1, 0}, {0, 1, 1, 0, 1, 1},
            {0, 1, 1, 1, 0, 0}, {0, 1, 1, 1, 0, 1}, {0, 1, 1, 1, 1, 0}, {0, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 1}, {1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 1, 1},
            {1, 0, 0, 1, 0, 0}, {1, 0, 0, 1, 0, 1}, {1, 0, 0, 1, 1, 0}, {1, 0, 0, 1, 1, 1},
            {1, 0, 1, 0, 0, 0}, {1, 0, 1, 0, 0, 1}, {1, 0, 1, 0, 1, 0}, {1, 0, 1, 0, 1, 1},
            {1, 0, 1, 1, 0, 0}, {1, 0, 1, 1, 0, 1}, {1, 0, 1, 1, 1, 0}, {1, 0, 1, 1, 1, 1},
            {1, 1, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 1}, {1, 1, 0, 0, 1, 0}, {1, 1, 0, 0, 1, 1},
            {1, 1, 0, 1, 0, 0}, {1, 1, 0, 1, 0, 1}, {1, 1, 0, 1, 1, 0}, {1, 1, 0, 1, 1, 1},
            {1, 1, 1, 0, 0, 0}, {1, 1, 1, 0, 0, 1}, {1, 1, 1, 0, 1, 0}, {1, 1, 1, 0, 1, 1},
            {1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 1}, {1, 1, 1, 1, 1, 0}, {1, 1, 1, 1, 1, 1}}};

    double volume = 0.0;
    const double* vect[3];
    const std::array<std::array<double,8>,3> data = {{X, Y, Z}};
    double perm_sign = 1;
    for (const auto& perm : permutation) {
        for (std::size_t perm_index = 0; perm_index < 3; perm_index++)
            vect[perm_index] = data[perm[perm_index]].data();

        for (const auto& pqr : pqr_array) {
            const double cprod = C(vect[0], 1, pqr.pb, pqr.pg)*C(vect[1], pqr.qa, 1, pqr.qg)*C(vect[2], pqr.ra, pqr.rb, 1);
            const double denom = (pqr.qa + pqr.ra + 1) * (pqr.pb + pqr.rb + 1) * (pqr.pg + pqr.qg + 1);
            volume += perm_sign * cprod / denom;
        }

        perm_sign *= -1;
    }
    return std::fabs(volume);
}


/* 
    Cell volume calculation for a cell from a cylindrical grid, given by the
    inner and outer radius of the cell, and its spans in the angle and Z.
*/
double calculateCylindricalCellVol(const double r_inner, const double r_outer, const double delta_theta, const double delta_z)
{
    return M_PI * std::abs((std::pow(r_outer,2) - std::pow(r_inner,2)) * delta_theta * delta_z) / 360.0;
}
