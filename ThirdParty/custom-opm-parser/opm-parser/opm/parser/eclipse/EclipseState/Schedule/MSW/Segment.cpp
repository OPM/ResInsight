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
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Segment.hpp>


namespace Opm {

    Segment::Segment()
    : m_segment_number(-1),
      m_branch(-1),
      m_outlet_segment(-1),
      m_total_length(invalid_value),
      m_depth(invalid_value),
      m_internal_diameter(invalid_value),
      m_roughness(invalid_value),
      m_cross_area(invalid_value),
      m_volume(invalid_value),
      m_data_ready(false)
    {
    }


    Segment::Segment(int segment_number_in, int branch_in, int outlet_segment_in, double length_in, double depth_in,
                     double internal_diameter_in, double roughness_in, double cross_area_in,
                     double volume_in, bool data_ready_in)
    : m_segment_number(segment_number_in),
      m_branch(branch_in),
      m_outlet_segment(outlet_segment_in),
      m_total_length(length_in),
      m_depth(depth_in),
      m_internal_diameter(internal_diameter_in),
      m_roughness(roughness_in),
      m_cross_area(cross_area_in),
      m_volume(volume_in),
      m_data_ready(data_ready_in)
    {
    }

    Segment::Segment(std::shared_ptr<const Segment> segment_in)
    : m_segment_number(segment_in->segmentNumber()),
      m_branch(segment_in->branchNumber()),
      m_outlet_segment(segment_in->outletSegment()),
      m_total_length(segment_in->totalLength()),
      m_depth(segment_in->depth()),
      m_internal_diameter(segment_in->internalDiameter()),
      m_roughness(segment_in->roughness()),
      m_cross_area(segment_in->crossArea()),
      m_volume(segment_in->volume()),
      m_data_ready(segment_in->dataReady())
    {
    }


    int Segment::segmentNumber() const {
        return m_segment_number;
    }


    int Segment::branchNumber() const {
        return m_branch;
    }


    int Segment::outletSegment() const {
        return m_outlet_segment;
    }


    double Segment::totalLength() const {
        return m_total_length;
    }


    double Segment::depth() const {
        return m_depth;
    }


    double Segment::internalDiameter() const {
        return m_internal_diameter;
    }


    double Segment::roughness() const {
        return m_roughness;
    }


    double Segment::crossArea() const {
        return m_cross_area;
    }

    double Segment::volume() const {
        return m_volume;
    }

    bool Segment::dataReady() const {
        return m_data_ready;
    }

    void Segment::setDepthAndLength(const double depth_in,  const double length_in) {
        m_total_length = length_in;
        m_depth = depth_in;
        m_data_ready = true;
    }

    void Segment::setVolume(const double volume_in) {
        m_volume = volume_in;
    }

    double Segment::invalidValue() {
        return invalid_value;
    }

}
