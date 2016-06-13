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

#ifndef SEGMENTSET_HPP_HEADER_INCLUDED
#define SEGMENTSET_HPP_HEADER_INCLUDED

#include <vector>
#include <map>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Segment.hpp>

namespace Opm {

    class DeckKeyword;

    class SegmentSet {
    public:
        SegmentSet();

        std::string wellName() const;
        int numberBranch() const;
        int numberSegment() const;
        double depthTopSegment() const;
        double lengthTopSegment() const;
        double volumeTopSegment() const;

        WellSegment::LengthDepthEnum lengthDepthType() const;
        WellSegment::CompPressureDropEnum compPressureDrop() const;
        WellSegment::MultiPhaseModelEnum multiPhaseModel() const;

        int numberToLocation(const int segment_number) const;
        void addSegment(std::shared_ptr< const Segment > new_segment);

        void segmentsFromWELSEGSKeyword( const DeckKeyword& welsegsKeyword);

        SegmentSet* shallowCopy() const;

        std::shared_ptr< const Segment > operator[](size_t idx) const;
        void orderSegments();
        void processABS();
        void processINC(const bool first_time);

    private:

        // name of the well
        std::string m_well_name;
        // number of the branches
        int m_number_branch;
        // depth of the nodal point of the top segment
        // it is taken as the BHP reference depth of the well
        // BHP reference depth data from elsewhere will be ignored for multi-segmented wells
        double m_depth_top;
        // length down the tubing to the nodal point of the top segment
        double m_length_top;
        // effective wellbore volume of the top segment
        double m_volume_top;
        // type of the tubing length and depth information
        WellSegment::LengthDepthEnum m_length_depth_type;
        // components of the pressure drop to be included
        WellSegment::CompPressureDropEnum m_comp_pressure_drop;
        // multi-phase flow model
        WellSegment::MultiPhaseModelEnum m_multiphase_model;
        // There are X and Y cooridnate of the nodal point of the top segment
        // Since they are not used for simulations and we are not supporting plotting,
        // we are not handling them at the moment.
        // There are other three properties for segment related to thermal conduction,
        // while they are not supported by the keyword at the moment.

        std::vector<std::shared_ptr< const Segment >> m_segments;
        // the mapping from the segment number to the
        // storage location in the vector
        std::map<int, int> m_number_to_location;
    };

    typedef std::shared_ptr<SegmentSet> SegmentSetPtr;
    typedef std::shared_ptr<const SegmentSet> SegmentSetConstPtr;
}

#endif
