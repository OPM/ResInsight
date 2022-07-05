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
        enum index : std::vector<float>::size_type {
            EfficiencyFactor = 92, // Group's efficiency factor (Item 2 of GEFAC)
        };

        enum prod_index : std::vector<float>::size_type {
            GuideRate     =  2, // Group's guide rate value
            OilRateLimit  =  6, // Group's oil production target/limit
            WatRateLimit  =  7, // Group's water production target/limit
            GasRateLimit  =  8, // Group's gas production target/limit
            LiqRateLimit  =  9, // Group's liquid production target/limit

            GLOMaxSupply  = 34, // Group's maximum supply of lift gas

            GasRateLimit_2 = 39, // Copy of group's gas production target/limit
            OilRateLimit_2 = 52, // Copy of group's oil production target/limit
            WatRateLimit_2 = 53, // Copy of group's water production target/limit
            LiqRateLimit_2 = 54, // Copy of group's liquid production target/limit

            GLOMaxRate    = 91, // Group's maximum lift gas rate
        };

        enum inj_index : std::vector<float>::size_type {
            oilSurfRateLimit        =  10, // Group's oil surface volume injection rate target/limit
            oilResRateLimit         =  11, // Group's oil reservoir volume injection rate target/limit
            oilReinjectionLimit     =  12, // Group's oil reinjection fraction target/limit
            oilVoidageLimit         =  13, // Group's oil voidage injection fraction target/limit

            waterSurfRateLimit      =  15, // Group's water surface volume injection rate target/limit
            waterResRateLimit       =  16, // Group's water reservoir volume injection rate target/limit
            waterReinjectionLimit   =  17, // Group's water reinjection fraction target/limit
            waterVoidageLimit       =  18, // Group's water voidage injection fraction target/limit
            waterGuideRate          =  19, // Group's water injection guide rate

            gasSurfRateLimit        =  20, // Group's gas surface volume injection rate target/limit
            gasResRateLimit         =  21, // Group's gas reservoir volume injection rate target/limit
            gasReinjectionLimit     =  22, // Group's gas reinjection fraction target/limit
            gasVoidageLimit         =  23, // Group's gas voidage injection fraction target/limit
            gasGuideRate            =  24, // Group's gas injection guide rate

            oilSurfRateLimit_2      =  57, // Copy of group's oil surface volume injection rate target/limit
            oilResRateLimit_2       =  58, // Copy of group's oil reservoir volume injection rate target/limit
            oilReinjectionLimit_2   =  59, // Copy of group's oil reinjection fraction target/limit
            oilVoidageLimit_2       =  60, // Copy of group's oil voidage injection fraction target/limit

            waterSurfRateLimit_2    =  61, // Copy of group's water surface volume injection rate target/limit
            waterResRateLimit_2     =  62, // Copy of group's water reservoir volume injection rate target/limit
            waterReinjectionLimit_2 =  63, // Copy of group's water reinjection fraction target/limit
            waterVoidageLimit_2     =  64, // Copy of group's water voidage injection fraction target/limit

            gasSurfRateLimit_2      =  65, // Copy of group's gas surface volume injection rate target/limit
            gasResRateLimit_2       =  66, // Copy of group's gas reservoir volume injection rate target/limit
            gasReinjectionLimit_2   =  67, // Copy of group's gas reinjection fraction target/limit
            gasVoidageLimit_2       =  68, // Copy of group's gas voidage injection fraction target/limit
        };

        namespace Value {
            constexpr auto NoGLOLimit = -10.0f;
        } // namespace Value
    } // SGroup


    namespace IGroup {
    // Observe that these value should not be used as ordinary indices into
    // the the IGRP vector, they should all be used as IGRP[NWGMAX + $index]
    enum index : std::vector<int>::size_type {
        NoOfChildGroupsWells = 0,
        ProdActiveCMode = 1,
        ProdHighLevCtrl = 5,
        GuideRateDef = 6,
        ExceedAction = 7,
        GConProdCMode = 10,
        WInjActiveCMode = 16,
        WInjHighLevCtrl = 17,
        GConInjeWInjCMode = 19,
        GConInjeWaterGuideRateMode = 20,
        GInjActiveCMode = 21,
        GInjHighLevCtrl = 22,
        GConInjeGInjCMode = 24,
        GConInjeGasGuideRateMode = 25,
        GroupType = 26,
        GroupLevel = 27,
        ParentGroup = 28,
        FlowingWells = 33,
        NodeNumber = 39
    };

    namespace Value {
    enum GuideRateMode : int {
        None = 0,
        Oil = 1,
        Water = 2,
        Gas = 3,
        Liquid = 4,
        Resv = 6,  // need to be verified!!!
        Potn = 7,
        Form = 8,
        Comb = 9,
    };

    enum GroupType : int {
        WellGroup = 0,
        TreeGroup = 1,
    };

    }

    }


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

            OilPrTotalSolution = 73, // Group's total cumulative oil production in solution
            GasPrTotalSolution = 74, // Group's total cumulative gas production in solution

            OilPrGuideRate  = 85, // Group's producer guide rate for oil.
            WatPrGuideRate  = 86, // Group's producer guide rate for water.
            GasPrGuideRate  = 87, // Group's producer guide rate for gas.
            VoidPrGuideRate = 88, // Group's producer guide rate for reservoir voidage volume.

            OilInjGuideRate   = 89, // Group's injection guide rate for oil.
            WatInjGuideRate   = 91, // Group's injection guide rate for water.
            WatInjGuideRate_2 = 92, // Second copy of group's injection guide rate for water.
                                    // Not fully characterised.
            GasInjGuideRate   = 93, // Groups injection guide rate for gas.

            OilPrGuideRate_2  = 127, // Second copy of group's producer guide rate for oil.
                                     // Not fully characterised.
            WatPrGuideRate_2  = 128, // Second copy of group's producer guide rate for water.
                                     // Not fully characterised.
            GasPrGuideRate_2  = 129, // Second copy of group's producer guide rate for gas.
                                     // Not fully characterised.
            VoidPrGuideRate_2 = 130, // Second copy of group's producer guide rate for
                                     // reservoir voidage volume.  Not fully characterised.

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

            TracerOffset = 180,    // Tracer data starts here
        };
    } // XGroup

}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_GROUP_HPP
