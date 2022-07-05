/*
  Copyright (c) 2021 Equinor ASA

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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_ACTION_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_ACTION_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {

    namespace IACN {
        enum index : std::vector<int>::size_type {
            LHSQuantityType = 10,
            RHSQuantityType = 11,
            FirstGreater    = 12,
            TerminalLogic   = 13,
            Paren           = 15,
            Comparator      = 16,
            BoolLink        = 17
        };

        // The same enum is used for both lefthand side and righthand side quantities;
        // although not all values can be used on both sides.
        namespace Value {
        enum QuantityType {
            Field     = 1,
            Well      = 2,
            Group     = 3,
            Const     = 8,
            Day       = 10,
            Month     = 11,
            Year      = 12
        };

        enum ParenType {
            None = 0,
            Open = 1,
            Close = 2
        };

        }

        constexpr std::size_t ConditionSize = 26;
    }


    namespace SACN {

    enum index : std::vector<int>::size_type {
        LHSValue0 = 0,
        RHSValue0 = 2,
        LHSValue1 = 4,
        RHSValue1 = 5,
        LHSValue2 = 6,
        RHSValue2 = 7,
        LHSValue3 = 8,
        RHSValue3 = 9
    };

    constexpr std::size_t ConditionSize = 16;
    }


    namespace ZACN {
    enum index : std::vector<int>::size_type {
        Quantity = 0,
        LHSQuantity = 0,
        RHSQuantity = 1,
        Comparator = 2,
        Well = 3,
        LHSWell = 3,
        RHSWell = 4,
        Group = 5,
        LHSGroup = 5,
        RHSGroup = 6
    };

    constexpr std::size_t RHSOffset = 1;
    constexpr std::size_t ConditionSize = 13;
    }

    namespace ZLACT {

    constexpr std::size_t max_line_length = 128;
    }



}}}} // Opm::RestartIO::Helpers::VectorItems

#endif
