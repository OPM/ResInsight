/*
  Copyright (c) 2018 Equinor ASA

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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_GROUP_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_GROUP_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {

    
        namespace SGroup {
        enum prod_index : std::vector<float>::size_type {
            OilRateLimit  =  6, // Group's oil production target/limit
            WatRateLimit  =  7, // Group's water production target/limit
            GasRateLimit  =  8, // Group's gas production target/limit
            LiqRateLimit  =  9, // Group's liquid production target/limit
        };
        
        enum inj_index : std::vector<float>::size_type {
            waterSurfRateLimit      =  15, //i Group's water surface volume injection rate target/limit
            waterResRateLimit       =  16, // Group's water reservoir volume injection rate target/limit
            waterReinjectionLimit   =  17, // Group's water reinjection fraction target/limit
            waterVoidageLimit       =  18, // Group's water voidage injection fraction target/limit
            gasSurfRateLimit        =  20, // Group's gas surface volume injection rate target/limit
            gasResRateLimit         =  21, // Group's gas reservoir volume injection rate target/limit
            gasReinjectionLimit     =  22, // Group's gas reinjection fraction target/limit
            gasVoidageLimit         =  23, // Group's gas voidage injection fraction target/limit

        };
    } // SGroup

    
    
    namespace XGroup {
        enum index : std::vector<double>::size_type {
            OilPrRate  =  0, // Group's oil production rate
            WatPrRate  =  1, // Group's water production rate
            GasPrRate  =  2, // Group's gas production rate
            LiqPrRate  =  3, // Group's liquid production rate

            WatInjRate =  5, // Group's water injection rate
            GasInjRate =  6, // Group's gas injection rate

            WatCut     =  8, // Group's producing water cut
            GORatio    =  9, // Group's producing gas/oil ratio

            OilPrTotal  = 10, // Group's total cumulative oil production
            WatPrTotal  = 11, // Group's total cumulative water production
            GasPrTotal  = 12, // Group's total cumulative gas production
            VoidPrTotal = 13, // Group's total cumulative reservoir
                              // voidage production

            WatInjTotal  = 15, // Group's total cumulative water injection
            GasInjTotal  = 16, // Group's total cumulative gas injection
            VoidInjTotal = 17, // Group's total cumulative reservoir volume injection

            OilPrPot = 22, // Group's oil production potential
            WatPrPot = 23, // Group's water production potential

            HistOilPrTotal  = 135, // Group's total cumulative oil
                                   // production (observed/historical rates)
            HistWatPrTotal  = 139, // Group's total cumulative water
                                   // production (observed/historical rates)
            HistWatInjTotal = 140, // Group's total cumulative water
                                   // injection (observed/historical rates)
            HistGasPrTotal  = 143, // Group's total cumulative gas
                                   // production (observed/historical rates)
            HistGasInjTotal = 144, // Group's total cumulative gas injection
                                   // (observed/historical rates)
        };
    } // XGroup

}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_GROUP_HPP
