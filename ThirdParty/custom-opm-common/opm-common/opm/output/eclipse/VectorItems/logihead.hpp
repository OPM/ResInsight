/*
  Copyright (c) 2019 Equinor ASA

  This file is part of the Open Porous Media project (OPM).

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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_LOGIHEAD_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_LOGIHEAD_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {

    // This is a subset of the items in src/opm/output/eclipse/LogiHEAD.cpp .
    // Promote items from that list to this in order to make them public.
    enum logihead : std::vector<bool>::size_type {
        IsLiveOil  =  0,    // Oil phase w/dissolved gas
        IsWetGas   =  1,    // Gas phase w/vaporised oil
        DirKr      =  2,    // Directional relative permeability
        E100RevKr  =  3,    // Reversible rel. perm. (E100)
        E100Radial =  4,    // Radial model (E100)
        E300Radial =  3,    // Radial model (E300, others)
        E300RevKr  =  4,    // Reversible rel. perm. (E300, others)
        Hyster     =  6,    // Enable hysteresis
        DualPoro   = 14,    // Enable dual porosity
        EndScale   = 16,    // Enable end-point scaling
        DirEPS     = 17,    // Directional end-point scaling
        RevEPS     = 18,    // Reversible end-point scaling
        AltEPS     = 19,    // Alternative (3-pt) end-point scaling
        HasNetwork = 37,     // Indicates Network option used
        ConstCo    = 38,    // Constant oil compressibility (PVCDO)
        HasMSWells = 75,    // Whether or not model has MS Wells.
    };
}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_LOGIHEAD_HPP
