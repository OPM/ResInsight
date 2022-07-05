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

#include <optional>
#include <variant>
#include <vector>

#include <opm/input/eclipse/Schedule/MSW/Valve.hpp>
#include <opm/input/eclipse/Schedule/MSW/SICD.hpp>
#include <opm/input/eclipse/Schedule/MSW/AICD.hpp>


namespace Opm {
    namespace RestartIO {
        struct RstSegment;
    }
}

namespace Opm {

    /*
      The current serialization of std::variant<> requires that all the types in
      the variant have serializeOp() method, that is why this RegularSegment
      type is introduced, ideally the icd variant should just have
      std::monostate to represent the regular non ICD segment.
    */
    struct RegularSegment : std::monostate {

        template<class Serializer>
        void serializeOp(Serializer&) {
        }

        static RegularSegment serializeObject() {
            return RegularSegment();
        }


        bool operator==(const RegularSegment& ) {
            return true;
        }
    };


    class Segment {
    public:

        enum class SegmentType {
            REGULAR,
            SICD,
            AICD, // Not really supported - just included to complete the enum
            VALVE,
        };

        Segment();

        Segment(const Segment& src, double new_depth, double new_length, double new_volume);
        Segment(const Segment& src, double new_depth, double new_length);
        Segment(const Segment& src, double new_volume);
        Segment(int segment_number_in, int branch_in, int outlet_segment_in, double length_in, double depth_in,
                double internal_diameter_in, double roughness_in, double cross_area_in, double volume_in, bool data_ready_in);
        Segment(const RestartIO::RstSegment& rst_segment);

        static Segment serializeObject();

        int segmentNumber() const;
        int branchNumber() const;
        int outletSegment() const;
        double perfLength() const;
        double totalLength() const;
        double depth() const;
        double internalDiameter() const;
        double roughness() const;
        double crossArea() const;
        double volume() const;
        bool dataReady() const;

        SegmentType segmentType() const;
        int ecl_type_id() const;


        const std::vector<int>& inletSegments() const;

        static double invalidValue();
        static SegmentType type_from_int(int ecl_id);

        bool operator==( const Segment& ) const;
        bool operator!=( const Segment& ) const;

        const SICD& spiralICD() const;
        const AutoICD& autoICD() const;
        const Valve& valve() const;

        void updatePerfLength(double perf_length);
        void updateSpiralICD(const SICD& spiral_icd);
        void updateAutoICD(const AutoICD& aicd);
        void updateValve(const Valve& valve, const double segment_length);
        void updateValve(const Valve& valve);
        void addInletSegment(const int segment_number);

        bool isRegular() const
        {
            return std::holds_alternative<RegularSegment>(this->m_icd);
        }

        inline bool isSpiralICD() const
        {
            return std::holds_alternative<SICD>(this->m_icd);
        }

        inline bool isAICD() const
        {
            return std::holds_alternative<AutoICD>(this->m_icd);
        }

        inline bool isValve() const
        {
            return std::holds_alternative<Valve>(this->m_icd);
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_segment_number);
            serializer(m_branch);
            serializer(m_outlet_segment);
            serializer(m_inlet_segments);
            serializer(m_total_length);
            serializer(m_depth);
            serializer(m_internal_diameter);
            serializer(m_roughness);
            serializer(m_cross_area);
            serializer(m_volume);
            serializer(m_data_ready);
            serializer(m_perf_length);
            serializer(m_icd);
        }

    private:
        void updateValve__(Valve& valve, const double segment_length);
        // segment number
        // it should work as a ID.
        int m_segment_number;
        // branch number
        // for top segment, it should always be 1
        int m_branch;
        // the outlet junction segment
        // for top segment, it should be -1
        int m_outlet_segment;
        // the segments whose outlet segments are the current segment
        std::vector<int> m_inlet_segments;
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

        std::optional<double> m_perf_length;
        std::variant<RegularSegment, SICD, AutoICD, Valve> m_icd;

        // We are not handling the length of segment projected onto the X-axis and Y-axis.
        // They are not used in the simulations and we are not supporting the plotting.
        // There are other three properties for segment related to thermal conduction,
        // while they are not supported by the keyword at the moment.
    };

}

#endif
