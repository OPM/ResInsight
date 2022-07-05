/*
  Copyright 2017 SINTEF Digital, Mathematics and Cybernetics.
  Copyright 2019 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/MSW/icd.hpp>
#include <opm/input/eclipse/Schedule/MSW/SICD.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <limits>
#include <cmath>

#include "src/opm/input/eclipse/Schedule/MSW/icd_convert.hpp"
#include "src/opm/input/eclipse/Schedule/MSW/FromWSEG.hpp"



namespace Opm {

    SICD::SICD()
        : SICD(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, {}, ICDStatus::SHUT, 1.0)
    {
    }

    SICD::SICD(double strength,
               double length,
               double densityCalibration,
               double viscosityCalibration,
               double criticalValue,
               double widthTransitionRegion,
               double maxViscosityRatio,
               int flowScaling,
               const std::optional<double>& maxAbsoluteRate,
               ICDStatus status,
               double scalingFactor)
        : m_strength(strength),
          m_length(length),
          m_density_calibration(densityCalibration),
          m_viscosity_calibration(viscosityCalibration),
          m_critical_value(criticalValue),
          m_width_transition_region(widthTransitionRegion),
          m_max_viscosity_ratio(maxViscosityRatio),
          m_method_flow_scaling(flowScaling),
          m_max_absolute_rate(maxAbsoluteRate),
          m_status(status),
          m_scaling_factor(scalingFactor)
    {
    }


    SICD::SICD(const DeckRecord& record)
            : m_strength(record.getItem("STRENGTH").getSIDouble(0)),
              m_length(record.getItem("LENGTH").getSIDouble(0)),
              m_density_calibration(record.getItem("DENSITY_CALI").getSIDouble(0)),
              m_viscosity_calibration(record.getItem("VISCOSITY_CALI").getSIDouble(0)),
              m_critical_value(record.getItem("CRITICAL_VALUE").getSIDouble(0)),
              m_width_transition_region(record.getItem("WIDTH_TRANS").get<double>(0)),
              m_max_viscosity_ratio(record.getItem("MAX_VISC_RATIO").get<double>(0)),
              m_method_flow_scaling(record.getItem("METHOD_SCALING_FACTOR").get<int>(0))
    {
        if (record.getItem("MAX_ABS_RATE").hasValue(0))
            this->m_max_absolute_rate = record.getItem("MAX_ABS_RATE").getSIDouble(0);

        if (record.getItem("STATUS").getTrimmedString(0) == "OPEN") {
            m_status = ICDStatus::OPEN;
        } else {
            m_status = ICDStatus::SHUT;
        }
    }


    SICD SICD::serializeObject()
    {
        SICD result;
        result.m_strength = 1.0;
        result.m_length = 2.0;
        result.m_density_calibration = 3.0;
        result.m_viscosity_calibration = 4.0;
        result.m_critical_value = 5.0;
        result.m_width_transition_region = 6.0;
        result.m_max_viscosity_ratio = 7.0;
        result.m_method_flow_scaling = 8;
        result.m_max_absolute_rate = 9.0;
        result.m_status = ICDStatus::OPEN;
        result.m_scaling_factor = 10.0;

        return result;
    }




    std::map<std::string, std::vector<std::pair<int, SICD> > >
    SICD::fromWSEGSICD(const DeckKeyword& wsegsicd)
    {
        return fromWSEG<SICD>(wsegsicd);
    }

    const std::optional<double>& SICD::maxAbsoluteRate() const {
        return m_max_absolute_rate;
    }

    ICDStatus SICD::status() const {
        return m_status;
    }

    double SICD::strength() const {
        return m_strength;
    }

    double SICD::length() const {
        return m_length;
    }

    double SICD::densityCalibration() const {
        return m_density_calibration;
    }

    double SICD::viscosityCalibration() const
    {
        return m_viscosity_calibration;
    }

    double SICD::criticalValue() const {
        return m_critical_value;
    }

    double SICD::widthTransitionRegion() const
    {
        return m_width_transition_region;
    }

    double SICD::maxViscosityRatio() const
    {
        return m_max_viscosity_ratio;
    }

    int SICD::methodFlowScaling() const
    {
        return m_method_flow_scaling;
    }

    double SICD::scalingFactor() const
    {
        if (!this->m_scaling_factor.has_value())
            throw std::runtime_error("The scaling factor has not been updated with updateScalingFactor()");

        return m_scaling_factor.value();
    }

    void SICD::updateScalingFactor(const double outlet_segment_length, const double completion_length)
    {
        if (m_method_flow_scaling < 0) {
            if (m_length > 0.) { // icd length / outlet segment length
                m_scaling_factor = m_length / outlet_segment_length;
            } else if (m_length < 0.) {
                m_scaling_factor = std::abs(m_length);
            } else { // icd length is zero, not sure the proper way to handle this yet
                throw std::logic_error("Zero-value length of SICD is found when calcuating scaling factor");
            }
        } else if (m_method_flow_scaling == 0) {
            if (m_length  <= 0.)
                throw std::logic_error("Non positive length of SICD if found when method of scaling is zero");

            m_scaling_factor = m_length / outlet_segment_length;
        } else if (m_method_flow_scaling == 1) {
            m_scaling_factor = std::abs(m_length);
        } else if (m_method_flow_scaling == 2) {
            if (completion_length == 0.) {
                throw std::logic_error("Zero connection length is found. No way to update scaling factor for this SICD segment");
            }
            m_scaling_factor = m_length / completion_length;
        } else {
            throw std::logic_error(" invalid method specified to calculate flow scaling factor for SICD");
        }
    }


    bool SICD::operator==(const SICD& data) const {
        return this->strength() == data.strength() &&
               this->length() == data.length() &&
               this->densityCalibration() == data.densityCalibration() &&
               this->viscosityCalibration() == data.viscosityCalibration() &&
               this->criticalValue() == data.criticalValue() &&
               this->widthTransitionRegion() == data.widthTransitionRegion() &&
               this->maxViscosityRatio() == data.maxViscosityRatio() &&
               this->methodFlowScaling() == data.methodFlowScaling() &&
               this->maxAbsoluteRate() == data.maxAbsoluteRate() &&
               this->status() == data.status() &&
               this->scalingFactor() == data.scalingFactor();
    }

int SICD::ecl_status() const {
    return to_int(this->m_status);
}


}
