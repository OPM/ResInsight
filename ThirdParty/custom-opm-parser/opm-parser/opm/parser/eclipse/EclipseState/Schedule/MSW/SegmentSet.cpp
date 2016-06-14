/*
  Copyright 2016 Statoil ASA.

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
#include <iostream>
#include <cassert>
#include <cmath>
#include <map>

#ifdef _WIN32
#define _USE_MATH_DEFINES 
#include <math.h>
#endif

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Segment.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/SegmentSet.hpp>


namespace Opm {

    SegmentSet::SegmentSet() {
    }

    std::string SegmentSet::wellName() const {
        return m_well_name;
    }

    int SegmentSet::numberBranch() const {
        return m_number_branch;
    }

    int SegmentSet::numberSegment() const {
        return m_segments.size();
    }

    double SegmentSet::depthTopSegment() const {
        return m_depth_top;
    }

    double SegmentSet::lengthTopSegment() const {
        return m_length_top;
    }

    double SegmentSet::volumeTopSegment() const {
        return m_volume_top;
    }

    WellSegment::LengthDepthEnum SegmentSet::lengthDepthType() const {
        return m_length_depth_type;
    }

    WellSegment::CompPressureDropEnum SegmentSet::compPressureDrop() const {
        return m_comp_pressure_drop;
    }

    WellSegment::MultiPhaseModelEnum SegmentSet::multiPhaseModel() const {
        return m_multiphase_model;
    }

    SegmentConstPtr SegmentSet::operator[](size_t idx) const {
        return m_segments[idx];
    }

    int SegmentSet::numberToLocation(const int segment_number) const {
        auto it = m_number_to_location.find(segment_number);
        if (it != m_number_to_location.end()) {
            return it->second;
        } else {
            return -1;
        }
    }

    void SegmentSet::addSegment(SegmentConstPtr new_segment) {
       // decide whether to push_back or insert
       int segment_number = new_segment->segmentNumber();

       const int segment_location = numberToLocation(segment_number);

       if (segment_location < 0) { // it is a new segment
           m_number_to_location[segment_number] = numberSegment();
           m_segments.push_back(new_segment);
       } else { // the segment already exists
           m_segments[segment_location] = new_segment;
       }
   }

    SegmentSet* SegmentSet::shallowCopy() const {
        SegmentSet* copy = new SegmentSet();
        copy->m_well_name = m_well_name;
        copy->m_number_branch = m_number_branch;
        copy->m_depth_top = m_depth_top;
        copy->m_length_top = m_length_top;
        copy->m_volume_top = m_volume_top;
        copy->m_length_depth_type = m_length_depth_type;
        copy->m_comp_pressure_drop = m_comp_pressure_drop;
        copy->m_multiphase_model = m_multiphase_model;
        copy->m_number_to_location = m_number_to_location;
        copy->m_segments.resize(m_segments.size());
        for (int i = 0; i < int(m_segments.size()); ++i) {
            copy->m_segments[i] = m_segments[i];
        }
        return copy;
    }

    void SegmentSet::segmentsFromWELSEGSKeyword( const DeckKeyword& welsegsKeyword ) {

        // for the first record, which provides the information for the top segment
        // and information for the whole segment set
        const auto& record1 = welsegsKeyword.getRecord(0);
        m_well_name = record1.getItem("WELL").getTrimmedString(0);

        m_segments.clear();

        const double invalid_value = Segment::invalidValue(); // meaningless value to indicate unspecified values

        m_depth_top = record1.getItem("DEPTH").getSIDouble(0);
        m_length_top = record1.getItem("LENGTH").getSIDouble(0);
        m_length_depth_type = WellSegment::LengthDepthEnumFromString(record1.getItem("INFO_TYPE").getTrimmedString(0));
        m_volume_top = record1.getItem("WELLBORE_VOLUME").getSIDouble(0);
        m_comp_pressure_drop = WellSegment::CompPressureDropEnumFromString(record1.getItem("PRESSURE_COMPONENTS").getTrimmedString(0));
        m_multiphase_model = WellSegment::MultiPhaseModelEnumFromString(record1.getItem("FLOW_MODEL").getTrimmedString(0));

        // the main branch is 1 instead of 0
        // the segment number for top segment is also 1
        if (m_length_depth_type == WellSegment::INC) {
            SegmentConstPtr top_segment(new Segment(1, 1, 0, 0., 0., invalid_value, invalid_value, invalid_value,
                                                    m_volume_top, false));
            m_segments.push_back(top_segment);
        } else if (m_length_depth_type == WellSegment::ABS) {
            SegmentConstPtr top_segment(new Segment(1, 1, 0, m_length_top, m_depth_top, invalid_value, invalid_value,
                                                    invalid_value, m_volume_top, true));
            m_segments.push_back(top_segment);
        }

        // read all the information out from the DECK first then process to get all the required information
        for (size_t recordIndex = 1; recordIndex < welsegsKeyword.size(); ++recordIndex) {
            const auto& record = welsegsKeyword.getRecord(recordIndex);
            const int segment1 = record.getItem("SEGMENT1").get< int >(0);
            const int segment2 = record.getItem("SEGMENT2").get< int >(0);
            if ((segment1 < 2) || (segment2 < segment1)) {
                throw std::logic_error("illegal segment number input is found in WELSEGS!\n");
            }

            // how to handle the logical relations between lateral branches and parent branches.
            // so far, the branch number has not been used.
            const int branch = record.getItem("BRANCH").get< int >(0);
            if ((branch < 1)) {
                throw std::logic_error("illegal branch number input is found in WELSEGS!\n");
            }
            const int outlet_segment_readin = record.getItem("JOIN_SEGMENT").get< int >(0);
            double diameter = record.getItem("DIAMETER").getSIDouble(0);
            const auto& itemArea = record.getItem("AREA");
            double area;
            if (itemArea.hasValue(0)) {
                area = itemArea.getSIDouble(0);
            } else {
                area = M_PI * diameter * diameter / 4.0;
            }

            // if the values are incremental values, then we can just use the values
            // if the values are absolute values, then we need to calculate them during the next process
            // only the value for the last segment in the range is recorded
            const double segment_length = record.getItem("SEGMENT_LENGTH").getSIDouble(0);
            // the naming is a little confusing here.
            // naming following the definition from the current keyword for the moment
            const double depth_change = record.getItem("DEPTH_CHANGE").getSIDouble(0);

            const auto& itemVolume = record.getItem("VOLUME");
            double volume;
            if (itemVolume.hasValue(0)) {
                volume = itemVolume.getSIDouble(0);
            } else if (m_length_depth_type == WellSegment::INC) {
                volume = area * segment_length;
            } else {
                volume = invalid_value; // A * L, while L is not determined yet
            }

            const double roughness = record.getItem("ROUGHNESS").getSIDouble(0);

            for (int i = segment1; i <= segment2; ++i) {
                // for the first or the only segment in the range is the one specified in the WELSEGS
                // from the second segment in the range, the outlet segment is the previous segment in the range
                int outlet_segment = -1;
                if (i == segment1) {
                    outlet_segment = outlet_segment_readin;
                } else {
                    outlet_segment = i - 1;
                }

                if (m_length_depth_type == WellSegment::INC) {
                    m_segments.push_back(std::make_shared<const Segment>(i, branch, outlet_segment, segment_length, depth_change,
                                                                   diameter, roughness, area, volume, false));
                } else if (i == segment2) {
                    m_segments.push_back(std::make_shared<const Segment>(i, branch, outlet_segment, segment_length, depth_change,
                                                                   diameter, roughness, area, volume, true));
                } else {
                    m_segments.push_back(std::make_shared<const Segment>(i, branch, outlet_segment, invalid_value, invalid_value,
                                                                   diameter, roughness, area, volume, false));
                }
            }
        }

        for (size_t i_segment = 0; i_segment < m_segments.size(); ++i_segment){
            const int segment_number = m_segments[i_segment]->segmentNumber();
            const int location = numberToLocation(segment_number);
            if (location >= 0) { // found in the existing m_segments already
                throw std::logic_error("Segments with same segment number are found!\n");
            }
            m_number_to_location[segment_number] = i_segment;
        }

    }

    void SegmentSet::processABS() {
        const double invalid_value = Segment::invalidValue(); // meaningless value to indicate unspecified/uncompleted values

        orderSegments();

        int current_loc = 1;
        while (current_loc < numberSegment()) {
            if (m_segments[current_loc]->dataReady()) {
                current_loc ++;
                continue;
            }

            const int range_begin = current_loc;
            const int outlet_segment = m_segments[range_begin]->outletSegment();
            const int outlet_loc = numberToLocation(outlet_segment);

            assert(m_segments[outlet_loc]->dataReady() == true);

            int range_end = range_begin + 1;
            for (; range_end < numberSegment(); ++range_end) {
                if (m_segments[range_end]->dataReady() == true) {
                    break;
                }
            }

            if (range_end >= numberSegment()) {
                throw std::logic_error(" One range records in WELSEGS is wrong. ");
            }

            // set the length and depth values in the range.
            int number_segments = range_end - range_begin + 1;
            assert(number_segments > 1); //if only 1, the information should be complete

            const double length_outlet = m_segments[outlet_loc]->totalLength();
            const double depth_outlet = m_segments[outlet_loc]->depth();

            const double length_last = m_segments[range_end]->totalLength();
            const double depth_last = m_segments[range_end]->depth();

            // incremental length and depth for the segments within the range
            const double length_inc = (length_last - length_outlet) / number_segments;
            const double depth_inc = (depth_last - depth_outlet) / number_segments;
            const double volume_segment = m_segments[range_end]->crossArea() * length_inc;

            for (int k = range_begin; k <= range_end; ++k) {
                SegmentPtr new_segment = std::make_shared<Segment>(m_segments[k]);
                const double temp_length = length_outlet + (k - range_begin + 1) * length_inc;
                const double temp_depth = depth_outlet + (k - range_end + 1) * depth_inc;
                if (k != range_end) {
                    new_segment->setDepthAndLength(temp_depth, temp_length);
                }

                if (new_segment->volume() < 0.5 * invalid_value) {
                    new_segment->setVolume(volume_segment);
                }
                addSegment(new_segment);
            }
            current_loc = range_end + 1;
        }

        // then update the volume for all the segments except the top segment
        // this is for the segments specified individually while the volume is not specified.
        for (int i = 1; i < numberSegment(); ++i) {
            assert(m_segments[i]->dataReady());
            if (m_segments[i]->volume() == invalid_value) {
                SegmentPtr new_segment = std::make_shared<Segment>(m_segments[i]);
                const int outlet_segment = m_segments[i]->outletSegment();
                const int outlet_location = numberToLocation(outlet_segment);
                const double segment_length = m_segments[i]->totalLength() - m_segments[outlet_location]->totalLength();
                const double segment_volume = m_segments[i]->crossArea() * segment_length;
                new_segment->setVolume(segment_volume);
                addSegment(new_segment);
            }
        }
    }

    void SegmentSet::processINC(const bool first_time) {

        // update the information inside the SegmentSet to be in ABS way
        if (first_time) {
            SegmentPtr new_top_segment = std::make_shared<Segment>((*this)[0]);
            new_top_segment->setDepthAndLength(depthTopSegment(), lengthTopSegment());
            this->addSegment(new_top_segment);
        }

        orderSegments();

        // begin with the second segment
        for (int i_loc = 1; i_loc < numberSegment(); ++i_loc) {
            if (m_segments[i_loc]->dataReady() == false) {
                // find its outlet segment
                const int outlet_segment = m_segments[i_loc]->outletSegment();
                const int outlet_loc = numberToLocation(outlet_segment);

                // assert some information of the outlet_segment
                assert(outlet_loc >= 0);
                assert(m_segments[outlet_loc]->dataReady());

                const double outlet_depth = m_segments[outlet_loc]->depth();
                const double outlet_length = m_segments[outlet_loc]->totalLength();
                const double temp_depth = outlet_depth + m_segments[i_loc]->depth();
                const double temp_length = outlet_length + m_segments[i_loc]->totalLength();

                // applying the calculated length and depth to the current segment
                SegmentPtr new_segment = std::make_shared<Segment>(m_segments[i_loc]);
                new_segment->setDepthAndLength(temp_depth, temp_length);
                addSegment(new_segment);
            }
        }
    }

    void SegmentSet::orderSegments() {
        // re-ordering the segments to make later use easier.
        // two principles
        // 1. the location of the outlet segment will be stored in the lower location than the segment.
        // 2. the segments belong to the same branch will be continuously stored.

        // top segment will always be the first one
        // before this location, the reordering is done.
        int current_loc = 1;

        // clear the mapping from segment number to store location
        m_number_to_location.clear();
        // for the top segment
        m_number_to_location[1] = 0;

        while (current_loc < numberSegment()) {
            // the branch number of the last segment that is done re-ordering
            const int last_branch_number = m_segments[current_loc-1]->branchNumber();
            // the one need to be swapped to the current_loc.
            int target_segment_loc = -1;

            // looking for target_segment_loc
            for (int i_loc = current_loc; i_loc < numberSegment(); ++i_loc) {
                const int outlet_segment_number = m_segments[i_loc]->outletSegment();
                const int outlet_segment_location = numberToLocation(outlet_segment_number);
                if (outlet_segment_location < 0) { // not found the outlet_segment in the done re-ordering segments
                    continue;
                }
                if (target_segment_loc < 0) { // first time found a candidate
                    target_segment_loc = i_loc;
                } else { // there is already a candidate, chosing the one with the same branch number with last_branch_number
                    const int old_target_segment_loc_branch = m_segments[target_segment_loc]->branchNumber();
                    const int new_target_segment_loc_branch = m_segments[i_loc]->branchNumber();
                    if (new_target_segment_loc_branch == last_branch_number) {
                        if (old_target_segment_loc_branch != last_branch_number) {
                            target_segment_loc = i_loc;
                        } else {
                            throw std::logic_error("two segments in the same branch share the same outlet segment !!\n");
                        }
                    }
                }
            }

            if (target_segment_loc < 0) {
                throw std::logic_error("could not find candidate segment to swap in before the re-odering process get done !!\n");
            }
            assert(target_segment_loc >= current_loc);
            if (target_segment_loc > current_loc) {
                std::swap(m_segments[current_loc], m_segments[target_segment_loc]);
            }
            const int segment_number = m_segments[current_loc]->segmentNumber();
            m_number_to_location[segment_number] = current_loc;
            current_loc++;
        }
    }

}
