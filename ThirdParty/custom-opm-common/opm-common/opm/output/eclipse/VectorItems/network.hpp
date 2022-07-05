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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_NETWORK_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_NETWORK_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {



    namespace INode {
    enum index : std::vector<int>::size_type {
        NoBranchesConnToNode = 0,
        CumNoBranchesConnToNode = 1,
        Group = 2,
        FixedPresNode = 3,
    };
    }

    namespace IBran {
    enum index : std::vector<int>::size_type {
        DownTreeNode = 0,
        UpTreeNode = 1,
        VfpTableNo = 2,
    };
    }

    namespace RNode {
    enum index : std::vector<double>::size_type {
        NodePres = 0,
        FixedPresNode  = 1,
        PressureLimit = 2,
    };
    }

    namespace RBran {
    enum index : std::vector<double>::size_type {
        OilProdRate = 0,
        WaterProdRate = 1,
        GasProdRate = 2,
        OilDensity = 8,
        GasDensity = 9
    };
    }

    namespace ZNode {
        enum index : std::vector<const char*>::size_type {
            NodeName = 0, // Node name
        };
    } // ZNode

}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_NETWORK_HPP
