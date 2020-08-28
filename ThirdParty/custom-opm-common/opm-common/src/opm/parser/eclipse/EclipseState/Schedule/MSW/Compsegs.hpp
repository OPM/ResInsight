/*
  Copyright 2015 SINTEF ICT, Applied Mathematics.

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


#ifndef COMPSEGS_HPP_
#define COMPSEGS_HPP_

#include <memory>
#include <string>

#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

namespace Opm {

    class WellConnections;
    class DeckKeyword;
    class WellSegments;
    class EclipseGrid;

    struct Compsegs {
        int m_i;
        int m_j;
        int m_k;
        // the branch number on the main stem is always 1.
        // lateral branches should be numbered bigger than 1.
        // a suboridnate branch must have a higher branch number than parent branch.
        int m_branch_number;
        double m_distance_start;
        double m_distance_end;
        Connection::Direction m_dir;

        double center_depth;
        // we do not handle thermal length for the moment
        // double m_thermal_length;
        int segment_number;
        std::size_t m_seqIndex;

        Compsegs(int i_in, int j_in, int k_in, int branch_number_in, double distance_start_in, double distance_end_in,
                 Connection::Direction dir_in, double center_depth_in, int segment_number_in, std::size_t seqIndex_in);

        void calculateCenterDepthWithSegments(const WellSegments& segment_set);

        static std::vector< Compsegs > compsegsFromCOMPSEGSKeyword(const DeckKeyword& compsegsKeyword, const EclipseGrid& grid,
                                                                   const ParseContext& parseContext, ErrorGuard& errors);

        // get the segment number information and depth information based on the information from WellSegments
        static void processCOMPSEGS(std::vector< Compsegs >& compsegs, const WellSegments& segment_set);

        // update the segment related information for Connections
        static void updateConnectionsWithSegment(const std::vector< Compsegs >& compsegs,
                                                 const EclipseGrid& grid,
                                                 WellConnections& connection_set);

    };
}



#endif /* COMPSEGS_HPP_ */
