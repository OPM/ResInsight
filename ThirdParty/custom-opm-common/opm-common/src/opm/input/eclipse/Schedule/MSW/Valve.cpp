/*
  Copyright 2019 Equinor.

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

#include <opm/input/eclipse/Schedule/MSW/Valve.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <cassert>


namespace Opm {

    Valve::Valve()
        : Valve(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, ICDStatus::SHUT)
    {
    }

    Valve::Valve(double conFlowCoeff,
                 double conCrossA,
                 double conMaxCrossA,
                 double pipeAddLength,
                 double pipeDiam,
                 double pipeRough,
                 double pipeCrossA,
                 ICDStatus stat)
        : m_con_flow_coeff(conFlowCoeff)
        , m_con_cross_area(conCrossA)
        , m_con_max_cross_area(conMaxCrossA)
        , m_pipe_additional_length(pipeAddLength)
        , m_pipe_diameter(pipeDiam)
        , m_pipe_roughness(pipeRough)
        , m_pipe_cross_area(pipeCrossA)
        , m_status(stat)
    {
    }

    Valve::Valve(const DeckRecord& record)
        : m_con_flow_coeff(record.getItem("CV").get<double>(0))
        , m_con_cross_area(record.getItem("AREA").get<double>(0))
    {
        // we initialize negative values for the values are defaulted
        const double value_for_default = -1.e100;

        // TODO: we assume that the value input for this keyword is always positive
        if (record.getItem("EXTRA_LENGTH").defaultApplied(0)) {
            m_pipe_additional_length = value_for_default;
        } else {
            m_pipe_additional_length = record.getItem("EXTRA_LENGTH").get<double>(0);
        }

        if (record.getItem("PIPE_D").defaultApplied(0)) {
            m_pipe_diameter = value_for_default;
        } else {
            m_pipe_diameter = record.getItem("PIPE_D").get<double>(0);
        }

        if (record.getItem("ROUGHNESS").defaultApplied(0)) {
            m_pipe_roughness = value_for_default;
        } else {
            m_pipe_roughness = record.getItem("ROUGHNESS").get<double>(0);
        }

        if (record.getItem("PIPE_A").defaultApplied(0)) {
            m_pipe_cross_area = value_for_default;
        } else {
            m_pipe_cross_area = record.getItem("PIPE_A").get<double>(0);
        }

        if (record.getItem("STATUS").getTrimmedString(0) == "OPEN") {
            m_status = ICDStatus::OPEN;
        } else {
            m_status = ICDStatus::SHUT;
            // TODO: should we check illegal input here
        }

        if (record.getItem("MAX_A").defaultApplied(0)) {
            m_con_max_cross_area = value_for_default;
        } else {
            m_con_max_cross_area = record.getItem("MAX_A").get<double>(0);
        }
    }

    Valve Valve::serializeObject()
    {
        Valve result;
        result.m_con_flow_coeff = 1.0;
        result.m_con_cross_area = 2.0;
        result.m_con_max_cross_area = 3.0;
        result.m_pipe_additional_length = 4.0;
        result.m_pipe_diameter = 5.0;
        result.m_pipe_roughness = 6.0;
        result.m_pipe_cross_area = 7.0;
        result.m_status = ICDStatus::OPEN;

        return result;
    }

    std::map<std::string, std::vector<std::pair<int, Valve> > >
    Valve::fromWSEGVALV(const DeckKeyword& keyword)
    {
        std::map<std::string, std::vector<std::pair<int, Valve> > > res;

        for (const DeckRecord &record : keyword) {
            const std::string well_name = record.getItem("WELL").getTrimmedString(0);

            const int segment_number = record.getItem("SEGMENT_NUMBER").get<int>(0);

            const Valve valve(record);
            res[well_name].push_back(std::make_pair(segment_number, valve));
        }

        return res;
    }

    ICDStatus Valve::status() const {
        return m_status;
    }

    double Valve::conFlowCoefficient() const {
        return m_con_flow_coeff;
    }

    double Valve::conCrossArea() const {
        return m_con_cross_area;
    }

    double Valve::pipeAdditionalLength() const {
        return m_pipe_additional_length;
    }

    double Valve::pipeDiameter() const {
        return m_pipe_diameter;
    }

    double Valve::pipeRoughness() const {
        return m_pipe_roughness;
    }

    double Valve::pipeCrossArea() const {
        return m_pipe_cross_area;
    }

    double Valve::conMaxCrossArea() const {
        return m_con_max_cross_area;
    }

    void Valve::setPipeDiameter(const double dia) {
        m_pipe_diameter = dia;
    }

    void Valve::setPipeRoughness(const double rou) {
        m_pipe_roughness = rou;
    }

    void Valve::setPipeCrossArea(const double area) {
        m_pipe_cross_area = area;
    }

    void Valve::setConMaxCrossArea(const double area) {
        m_con_max_cross_area = area;
    }

    void Valve::setPipeAdditionalLength(const double length) {
        m_pipe_additional_length = length;
    }

    bool Valve::operator==(const Valve& data) const {
        return this->conFlowCoefficient() == data.conFlowCoefficient() &&
               this->conCrossArea() == data.conCrossArea() &&
               this->conMaxCrossArea() == data.conMaxCrossArea() &&
               this->pipeAdditionalLength() == data.pipeAdditionalLength() &&
               this->pipeDiameter() == data.pipeDiameter() &&
               this->pipeRoughness() == data.pipeRoughness() &&
               this->pipeCrossArea() == data.pipeCrossArea() &&
               this->status() == data.status();
    }
}
