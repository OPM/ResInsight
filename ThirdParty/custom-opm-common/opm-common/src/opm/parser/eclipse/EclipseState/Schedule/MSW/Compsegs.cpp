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

#include <cmath>
#include <iostream>
#include <sstream>

#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/io/eclipse/rst/well.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/WellSegments.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>

#include "Compsegs.hpp"

namespace Opm {
namespace Compsegs {

struct Record {
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

    Record(int i_in, int j_in, int k_in, int branch_number_in, double distance_start_in, double distance_end_in,
           Connection::Direction dir_in, double center_depth_in, int segment_number_in, std::size_t seqIndex_in);

    void calculateCenterDepthWithSegments(const WellSegments& segment_set);


};

    Record::Record(int i_in, int j_in, int k_in, int branch_number_in, double distance_start_in, double distance_end_in,
                       Connection::Direction dir_in, double center_depth_in, int segment_number_in, size_t seqIndex_in)
    : m_i(i_in),
      m_j(j_in),
      m_k(k_in),
      m_branch_number(branch_number_in),
      m_distance_start(distance_start_in),
      m_distance_end(distance_end_in),
      m_dir(dir_in),
      center_depth(center_depth_in),
      segment_number(segment_number_in),
      m_seqIndex(seqIndex_in)
    {
    }


    void Record::calculateCenterDepthWithSegments(const WellSegments& segment_set) {

        // the depth and distance of the segment to the well head
        const Segment& segment = segment_set.getFromSegmentNumber(segment_number);
        const double segment_depth = segment.depth();
        const double segment_distance = segment.totalLength();

        // for top segment, no interpolation is needed
        if (segment_number == 1) {
            center_depth = segment_depth;
            return;
        }

        // for other cases, interpolation between two segments is needed.
        // looking for the other segment needed for interpolation
        // by default, it uses the outlet segment to do the interpolation
        int interpolation_segment_number = segment.outletSegment();

        const double center_distance = (m_distance_start + m_distance_end) / 2.0;
        // if the perforation is further than the segment and the segment has inlet segments in the same branch
        // we use the inlet segment to do the interpolation
        if (center_distance > segment_distance) {
            for (const int inlet : segment.inletSegments()) {
                const int inlet_index = segment_set.segmentNumberToIndex(inlet);
                if (segment_set[inlet_index].branchNumber() == m_branch_number) {
                    interpolation_segment_number = inlet;
                    break;
                }
            }
        }

        if (interpolation_segment_number == 0) {
            throw std::runtime_error("Failed in finding a segment to do the interpolation with segment "
                                      + std::to_string(segment_number));
        }

        // performing the interpolation
        const Segment& interpolation_segment = segment_set.getFromSegmentNumber(interpolation_segment_number);
        const double interpolation_detph = interpolation_segment.depth();
        const double interpolation_distance = interpolation_segment.totalLength();

        const double depth_change_segment = segment_depth - interpolation_detph;
        const double segment_length = segment_distance - interpolation_distance;

        if (segment_length == 0.) {
            throw std::runtime_error("Zero segment length is botained when doing interpolation between segment "
                                      + std::to_string(segment_number) + " and segment " + std::to_string(interpolation_segment_number) );
        }

        center_depth = segment_depth + (center_distance - segment_distance) / segment_length * depth_change_segment;
    }



namespace {

    void processCOMPSEGS__(std::vector< Record >& compsegs, const WellSegments& segment_set) {
        // for the current cases we have at the moment, the distance information is specified explicitly,
        // while the depth information is defaulted though, which need to be obtained from the related segment
        for( auto& compseg : compsegs ) {

            // need to determine the related segment number first
            if (compseg.segment_number != 0) continue;

            const double center_distance = (compseg.m_distance_start + compseg.m_distance_end) / 2.0;
            const int branch_number = compseg.m_branch_number;

            int segment_number = 0;
            double min_distance_difference = 1.e100; // begin with a big value
            for (std::size_t i_segment = 0; i_segment < segment_set.size(); ++i_segment) {
                const Segment& current_segment = segment_set[i_segment];
                if( branch_number != current_segment.branchNumber() ) continue;

                const double distance = current_segment.totalLength();
                const double distance_difference = std::abs(center_distance - distance);
                if (distance_difference < min_distance_difference) {
                    min_distance_difference = distance_difference;
                    segment_number = current_segment.segmentNumber();
                }
            }

            if (segment_number == 0) {
                std::ostringstream sstr;
                sstr << "The connection specified in COMPSEGS with index of " << compseg.m_i + 1 << " "
                     << compseg.m_j + 1 << " " << compseg.m_k + 1 << " failed in finding a related segment";
                throw std::runtime_error(sstr.str());
            }

            compseg.segment_number = segment_number;

            // when depth is default or zero, we obtain the depth of the connection based on the information
            // of the related segments
            if (compseg.center_depth == 0.) {
                compseg.calculateCenterDepthWithSegments(segment_set);
            }
        }
    }

    std::vector< Record > compsegsFromCOMPSEGSKeyword(const DeckKeyword& compsegsKeyword,
                                                      const WellSegments& segments,
                                                      const EclipseGrid& grid,
                                                      const ParseContext& parseContext,
                                                      ErrorGuard& errors) {

        std::vector< Record > compsegs;

        // The first record in the keyword only contains the well name
        // looping from the second record in the keyword
        for (size_t recordIndex = 1; recordIndex < compsegsKeyword.size(); ++recordIndex) {
            const auto& record = compsegsKeyword.getRecord(recordIndex);
            // following the coordinate rule for connections
            const int I = record.getItem<ParserKeywords::COMPSEGS::I>().get< int >(0) - 1;
            const int J = record.getItem<ParserKeywords::COMPSEGS::J>().get< int >(0) - 1;
            const int K = record.getItem<ParserKeywords::COMPSEGS::K>().get< int >(0) - 1;
            const int branch = record.getItem<ParserKeywords::COMPSEGS::BRANCH>().get< int >(0);

            const std::string& well_name = compsegsKeyword.getRecord(0).getItem("WELL").getTrimmedString(0);

            double distance_start = 0.0;
            double distance_end = -1.0;
            if (record.getItem<ParserKeywords::COMPSEGS::DISTANCE_START>().hasValue(0)) {
                distance_start = record.getItem<ParserKeywords::COMPSEGS::DISTANCE_START>().getSIDouble(0);
            } else if (recordIndex == 1) {
                distance_start = 0.;
            } else {
                // TODO: the end of the previous connection or range
                // 'previous' should be in term of the input order
                // since basically no specific order for the connections
                const std::string msg = "This way to obtain DISTANCE_START in keyword COMPSEGS "
                                        "is not implemented yet for well " + well_name;
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_NOT_SUPPORTED, msg, errors);
            }
            if (record.getItem<ParserKeywords::COMPSEGS::DISTANCE_END>().hasValue(0)) {
                distance_end = record.getItem<ParserKeywords::COMPSEGS::DISTANCE_END>().getSIDouble(0);
            } else {
                // TODO: the distance_start plus the thickness of the grid block
                const std::string msg = "This way to obtain DISTANCE_END in keyword COMPSEGS "
                                        "is not implemented yet for well " + well_name;
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_NOT_SUPPORTED, msg, errors);
            }

            if (distance_end <= distance_start) {
                std::ostringstream sstr;
                sstr << " The end of the perforations need be to further down than the start of the perforations\n "
                     << " well " << well_name << " " << I + 1 << " " << J + 1 << " " << K + 1 << " in keyword COMPSEGS\n";
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_INVALID, sstr.str(), errors);
            }

            if( !record.getItem< ParserKeywords::COMPSEGS::DIRECTION >().hasValue( 0 ) &&
                !record.getItem< ParserKeywords::COMPSEGS::DISTANCE_END >().hasValue( 0 ) ) {
                const std::string msg = "The direction has to be specified when DISTANCE_END "
                                        "is not specified in keyword COMPSEGS for well " + well_name;
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_INVALID, msg, errors);
            }

            if( record.getItem< ParserKeywords::COMPSEGS::END_IJK >().hasValue( 0 ) &&
               !record.getItem< ParserKeywords::COMPSEGS::DIRECTION >().hasValue( 0 ) ) {
                const std::string msg = "The direction has to be specified when END_IJK "
                                        "is specified in keyword COMPSEGS for well " + well_name;
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_INVALID, msg, errors);
            }

            /*
             * Defaulted well connection. Must be non-defaulted if DISTANCE_END
             * is set or a range is specified. If not this is effectively ignored.
             */
            auto direction = Connection::Direction::X;
            if( record.getItem< ParserKeywords::COMPSEGS::DIRECTION >().hasValue( 0 ) ) {
                direction = Connection::DirectionFromString(record.getItem<ParserKeywords::COMPSEGS::DIRECTION>().get< std::string >(0));
            }

            double center_depth;
            if (!record.getItem<ParserKeywords::COMPSEGS::CENTER_DEPTH>().defaultApplied(0)) {
                center_depth = record.getItem<ParserKeywords::COMPSEGS::CENTER_DEPTH>().getSIDouble(0);
            } else {
                // 0.0 is also the defaulted value
                // which is used to indicate to obtain the final value through related segment
                center_depth = 0.;
            }

            if (center_depth < 0.) {
                //TODO: get the depth from COMPDAT data.
                const std::string msg = "This way to obtain CENTER_DISTANCE in keyword COMPSEGS "
                                        "is not implemented yet for well " + well_name;
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_NOT_SUPPORTED, msg, errors);
            }

            int segment_number;
            if (record.getItem<ParserKeywords::COMPSEGS::SEGMENT_NUMBER>().hasValue(0)) {
                segment_number = record.getItem<ParserKeywords::COMPSEGS::SEGMENT_NUMBER>().get< int >(0);
            } else {
                segment_number = 0;
                // will decide the segment number based on the distance in a process later.
            }
            if (!record.getItem<ParserKeywords::COMPSEGS::END_IJK>().hasValue(0)) { // only one compsegs

              if (grid.cellActive(I, J, K)) {
                std::size_t seqIndex = compsegs.size();
                compsegs.emplace_back( I, J, K,
                                       branch,
                                       distance_start, distance_end,
                                       direction,
                                       center_depth,
                                       segment_number,
                                       seqIndex);
              }
            } else { // a range is defined. genrate a range of Record
                std::ostringstream sstr;
                sstr << "COMPSEGS entries can only be input for single connection, not supporting COMPSEGS entries specified with a range yet.\n"
                     << " well " << well_name << " " << I + 1 << " " << J + 1 << " " << K + 1 << " in keyword COMPSEGS\n";
                parseContext.handleError(ParseContext::SCHEDULE_COMPSEGS_NOT_SUPPORTED, sstr.str(), errors);
            }
        }

        processCOMPSEGS__(compsegs, segments);
        return compsegs;
    }
}


    std::pair<WellConnections, WellSegments>
    processCOMPSEGS(const DeckKeyword& compsegs,
                    const WellConnections& input_connections,
                    const WellSegments& input_segments,
                    const EclipseGrid& grid,
                    const ParseContext& parseContext,
                    ErrorGuard& errors)
    {
            const auto& compsegs_vector = Compsegs::compsegsFromCOMPSEGSKeyword( compsegs, input_segments, grid, parseContext, errors);
            WellSegments new_segment_set = input_segments;
            WellConnections new_connection_set = input_connections;

            for (const auto& compseg : compsegs_vector) {
                const int i = compseg.m_i;
                const int j = compseg.m_j;
                const int k = compseg.m_k;
                if (grid.cellActive(i, j, k)) {
                    Connection& connection = new_connection_set.getFromIJK(i, j, k);
                    connection.updateSegment(compseg.segment_number,
                                             compseg.center_depth,
                                             compseg.m_seqIndex,
                                             { compseg.m_distance_start, compseg.m_distance_end });
                }
            }

            for (const auto& connection : new_connection_set) {
                if (!connection.attachedToSegment())
                    throw std::runtime_error("Not all the connections are attached with a segment. "
                                             "The information from COMPSEGS is not complete");
            }
            new_segment_set.updatePerfLength( new_connection_set );

            return std::make_pair( WellConnections( std::move( new_connection_set ) ),
                                   WellSegments( std::move(new_segment_set)));
    }

namespace {
    // Duplicated from Well.cpp
    Connection::Order order_from_int(int int_value) {
        switch(int_value) {
        case 0:
            return Connection::Order::TRACK;
        case 1:
            return Connection::Order::DEPTH;
        case 2:
            return Connection::Order::INPUT;
        default:
            throw std::invalid_argument("Invalid integer value: " + std::to_string(int_value) + " encountered when determining connection ordering");
        }
    }

}

    std::pair<WellConnections, WellSegments>
    rstUpdate(const RestartIO::RstWell& rst_well,
              std::vector<Connection> rst_connections,
              const std::unordered_map<int, Segment>& rst_segments)
    {
        for (auto& connection : rst_connections) {
            int segment_id = connection.segment();
            if (segment_id > 0) {
                const auto& segment = rst_segments.at(segment_id);
                connection.updateSegmentRST(segment.segmentNumber(),
                                            segment.depth());
            }
        }
        WellConnections connections(order_from_int(rst_well.completion_ordering),
                                    rst_well.ij[0],
                                    rst_well.ij[1],
                                    rst_connections);


        std::vector<Segment> segments_list;
        /*
          The ordering of the segments in the WellSegments structure seems a
          bit random; in some parts of the code the segment_number seems to
          be treated like a random integer ID, whereas in other parts it
          seems to be treated like a running index. Here the segments in
          WellSegments are sorted according to the segment number - observe
          that this is somewhat important because the first top segment is
          treated differently from the other segment.
        */
        for (const auto& segment_pair : rst_segments)
            segments_list.push_back( std::move(segment_pair.second) );

        std::sort( segments_list.begin(), segments_list.end(),[](const Segment& seg1, const Segment& seg2) { return seg1.segmentNumber() < seg2.segmentNumber(); } );
        auto comp_pressure_drop = WellSegments::CompPressureDrop::HFA;

        WellSegments segments( comp_pressure_drop, segments_list);
        segments.updatePerfLength( connections );

        return std::make_pair( std::move(connections), std::move(segments));
    }
}
}
