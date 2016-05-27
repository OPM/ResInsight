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

#ifndef SEGMENT_HPP_HEADER_INCLUDED
#define SEGMENT_HPP_HEADER_INCLUDED

#include <memory>

namespace Opm {

    class Segment {
    public:
        Segment();

        Segment(int segment_number_in, int branch_in, int outlet_segment_in, double length_in, double depth_in,
                double internal_diameter_in, double roughness_in, double cross_area_in, double volume_in, bool data_ready_in);
        Segment(std::shared_ptr<const Segment> segment_in);

        int segmentNumber() const;
        int branchNumber() const;
        int outletSegment() const;
        double totalLength() const;
        double depth() const;
        double internalDiameter() const;
        double roughness() const;
        double crossArea() const;
        double volume() const;
        bool dataReady() const;

        void setVolume(const double volume_in);
        void setDepthAndLength(const double depth_in, const double length_in);

        static double invalidValue();


    private:
        // segment number
        // it should work as a ID.
        int m_segment_number;
        // branch number
        // for top segment, it should always be 1
        int m_branch;
        // the outlet junction segment
        // for top segment, it should be -1
        int m_outlet_segment;
        // length of the segment node to the bhp reference point.
        // when reading in from deck, with 'INC',
        // it will be incremental length before processing.
        // After processing and in the class Well, it always stores the 'ABS' value.
        // which means the total_length
        double m_total_length;
        // depth of the nodes to the bhp reference point
        // when reading in from deck, with 'INC',
        // it will be the incremental depth before processing.
        // in the class Well, it always stores the 'ABS' value.
        // TODO: to check if it is good to use 'ABS' always.
        double m_depth;
        // tubing internal diameter
        // or the equivalent diameter for annular cross-sections
        // for top segment, it is UNDEFINED
        // we use invalid_value for the top segment
        double m_internal_diameter;
        // effective roughness of the tubing
        // used to calculate the Fanning friction factor
        // for top segment, it is UNDEFINED
        // we use invalid_value for the top segment
        double m_roughness;
        // cross-sectional area for fluid flow
        // not defined for the top segment,
        // we use invalid_value for the top segment.
        double m_cross_area;
        // valume of the segment;
        // it is defined for top segment.
        // TODO: to check if the definition is the same with other segments.
        double m_volume;
        // indicate if the data related to 'INC' or 'ABS' is ready
        // the volume will be updated at a final step.
        bool m_data_ready;

        static constexpr double invalid_value = -1.e100;
        // We are not handling the length of segment projected onto the X-axis and Y-axis.
        // They are not used in the simulations and we are not supporting the plotting.
        // There are other three properties for segment related to thermal conduction,
        // while they are not supported by the keyword at the moment.

    };

    typedef std::shared_ptr<Segment> SegmentPtr;
    typedef std::shared_ptr<const Segment> SegmentConstPtr;

}

#endif
