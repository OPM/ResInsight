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

#include <opm/io/eclipse/rst/segment.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Segment.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/SpiralICD.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Valve.hpp>

#include <cassert>

#include <string>
#include <stdexcept>

namespace Opm {
namespace {

static constexpr double invalid_value = -1.e100;

}



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

namespace {

    double if_invalid_value(double rst_value) {
        if (rst_value == 0)
            return invalid_value;

        return rst_value;
    }

}


    Segment::Segment(const RestartIO::RstSegment& rst_segment):
        m_segment_number(rst_segment.segment),
        m_branch(rst_segment.branch),
        m_outlet_segment(rst_segment.outlet_segment),
        m_total_length( rst_segment.dist_bhp_ref ),
        m_depth(rst_segment.bhp_ref_dz),
        m_internal_diameter(if_invalid_value(rst_segment.diameter)),
        m_roughness(if_invalid_value(rst_segment.roughness)),
        m_cross_area(if_invalid_value(rst_segment.area)),
        m_volume(rst_segment.volume),
        m_data_ready(true),
        m_segment_type(rst_segment.segment_type)
    {
        if (this->m_segment_type == SegmentType::SICD) {
            double scalingFactor = -1;  // The scaling factor will be and updated from the simulator.

            SpiralICD icd(rst_segment.base_strength,
                          rst_segment.icd_length,
                          rst_segment.fluid_density,
                          rst_segment.fluid_viscosity,
                          rst_segment.critical_water_fraction,
                          rst_segment.transition_region_width,
                          rst_segment.max_emulsion_ratio,
                          rst_segment.icd_scaling_mode,
                          rst_segment.max_valid_flow_rate,
                          rst_segment.icd_status,
                          scalingFactor);

            this->updateSpiralICD(icd);
        }

        if (this->m_segment_type == SegmentType::VALVE) {
            /*
              These three variables are currently not stored in the restart
              file; here we initialize with the default values, but if they have
              originally been assigned in the deck with non-default values, that
              will *not* be picked in a restarted run.
            */

            double pipeDiam = this->m_internal_diameter;
            double pipeRough = this->m_roughness;
            double pipeCrossA = this->m_cross_area;

            Valve valve(rst_segment.valve_flow_coeff,
                        rst_segment.valve_area,
                        rst_segment.valve_max_area,
                        rst_segment.valve_length,
                        pipeDiam,
                        pipeRough,
                        pipeCrossA,
                        rst_segment.icd_status);

            /*
              The segment length argument should be the length of this
              particular segment; in the input phase that is calculated from the
              WellSegments::segmentLength() function which also uses the outlet
              segment.
            */
            double segment_length = -1;
            throw std::logic_error("Sorry can not create a Valve segment from restart file");
            this->updateValve(valve, segment_length);
        }
    }



    Segment::Segment(int segment_number_in, int branch_in, int outlet_segment_in, double length_in, double depth_in,
                     double internal_diameter_in, double roughness_in, double cross_area_in,
                     double volume_in, bool data_ready_in, SegmentType segment_type_in)
    : m_segment_number(segment_number_in),
      m_branch(branch_in),
      m_outlet_segment(outlet_segment_in),
      m_total_length(length_in),
      m_depth(depth_in),
      m_internal_diameter(internal_diameter_in),
      m_roughness(roughness_in),
      m_cross_area(cross_area_in),
      m_volume(volume_in),
      m_data_ready(data_ready_in),
      m_segment_type(segment_type_in)
    {
    }

  Segment::Segment(const Segment& src, double new_depth, double new_length):
      Segment(src)
  {
      this->m_depth = new_depth;
      this->m_total_length = new_length;
      this->m_data_ready = true;
  }

  Segment::Segment(const Segment& src, double new_depth, double new_length, double new_volume):
    Segment(src, new_depth, new_length)
   {
       this->m_volume = new_volume;
   }

  Segment::Segment(const Segment& src, double new_volume):
      Segment(src)
  {
      this->m_volume = new_volume;
  }

  Segment Segment::serializeObject()
  {
      Segment result;
      result.m_segment_number = 1;
      result.m_branch = 2;
      result.m_outlet_segment = 3;
      result.m_inlet_segments = {4, 5};
      result.m_total_length = 6.0;
      result.m_depth = 7.0;
      result.m_internal_diameter = 8.0;
      result.m_roughness = 9.0;
      result.m_cross_area = 10.0;
      result.m_volume = 11.0;
      result.m_data_ready = true;
      result.m_segment_type = SegmentType::SICD;
      result.m_spiral_icd = std::make_shared<SpiralICD>(SpiralICD::serializeObject());
      result.m_valve = std::make_shared<Valve>(Valve::serializeObject());

      return result;
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

    Segment::SegmentType Segment::segmentType() const {
        return m_segment_type;
    }

    const std::vector<int>& Segment::inletSegments() const {
        return m_inlet_segments;
    }

    void Segment::addInletSegment(const int segment_number_in) {
        m_inlet_segments.push_back(segment_number_in);
    }

    double Segment::invalidValue() {
        return invalid_value;
    }

    bool Segment::operator==( const Segment& rhs ) const {
        return this->m_segment_number    == rhs.m_segment_number
            && this->m_branch            == rhs.m_branch
            && this->m_outlet_segment    == rhs.m_outlet_segment
            && this->m_total_length      == rhs.m_total_length
            && this->m_depth             == rhs.m_depth
            && this->m_internal_diameter == rhs.m_internal_diameter
            && this->m_roughness         == rhs.m_roughness
            && this->m_cross_area        == rhs.m_cross_area
            && this->m_volume            == rhs.m_volume
            && this->m_data_ready        == rhs.m_data_ready;
    }

    bool Segment::operator!=( const Segment& rhs ) const {
        return !this->operator==(rhs);
    }

    void Segment::updateSpiralICD(const SpiralICD& spiral_icd) {
        m_segment_type = SegmentType::SICD;
        m_spiral_icd = std::make_shared<SpiralICD>(spiral_icd);
    }

    const std::shared_ptr<SpiralICD>& Segment::spiralICD() const {
        return m_spiral_icd;
    }


    void Segment::updateValve(const Valve& valve, const double segment_length) {
        // we need to update some values for the vale
        auto valve_ptr = std::make_shared<Valve>(valve);

        if (valve_ptr->pipeAdditionalLength() < 0.) { // defaulted for this
            valve_ptr->setPipeAdditionalLength(segment_length);
        }

        if (valve_ptr->pipeDiameter() < 0.) {
            valve_ptr->setPipeDiameter(m_internal_diameter);
        } else {
            this->m_internal_diameter = valve_ptr->pipeDiameter();
        }

        if (valve_ptr->pipeRoughness() < 0.) {
            valve_ptr->setPipeRoughness(m_roughness);
        } else {
            this->m_roughness = valve_ptr->pipeRoughness();
        }

        if (valve_ptr->pipeCrossArea() < 0.) {
            valve_ptr->setPipeCrossArea(m_cross_area);
        } else {
            this->m_cross_area = valve_ptr->pipeCrossArea();
        }

        if (valve_ptr->conMaxCrossArea() < 0.) {
            valve_ptr->setConMaxCrossArea(valve_ptr->pipeCrossArea());
        }

        this->m_valve = valve_ptr;

        m_segment_type = SegmentType::VALVE;
    }


    const Valve* Segment::valve() const {
        return m_valve.get();
    }

    int Segment::ecl_type_id() const {
        switch (this->m_segment_type) {
        case SegmentType::REGULAR:
            return -1;
        case SegmentType::SICD:
            return -7;
        case SegmentType::AICD:
            return -8;
        case SegmentType::VALVE:
            return -5;
        default:
            throw std::invalid_argument("Unhanedled segment type");
        }
    }

    Segment::SegmentType Segment::type_from_int(int ecl_id) {
        switch(ecl_id) {
        case -1:
            return SegmentType::REGULAR;
        case -7:
            return SegmentType::SICD;
        case -8:
            return SegmentType::AICD;
        case -5:
            return SegmentType::VALVE;
        default:
            throw std::invalid_argument("Unhandeled segment type: " + std::to_string(ecl_id));
        }
    }
}


