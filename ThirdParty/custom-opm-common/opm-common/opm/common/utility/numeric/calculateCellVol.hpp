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

#include <vector>
#include <array>
#include <math.h>  

#ifndef CALCULATE_CELLVOL
#define CALCULATE_CELLVOL

double calculateCellVol(const std::array<double,8>& X, const std::array<double,8>& Y, const std::array<double,8>& Z);
double calculateCylindricalCellVol(const double R1, const double R2, const double dTheta, const double dZ);

#endif

