/*
  Copyright 2020 Equinor ASA.

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

#include <opm/parser/eclipse/EclipseState/Schedule/Group/GPMaint.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/G.hpp>

namespace Opm {

GPMaint::GPMaint(const DeckRecord& record)
{
    using GP = ParserKeywords::GPMAINT;
    this->m_flow_target = FlowTargetFromString( record.getItem<GP::FLOW_TARGET>().get<std::string>(0) );
    this->m_region_number = record.getItem<GP::REGION>().get<int>(0);
    this->m_region_name = record.getItem<GP::FIP_FAMILY>().get<std::string>(0);
    this->m_pressure_target = record.getItem<GP::PRESSURE_TARGET>().getSIDouble(0);
    this->m_prop_constant = record.getItem<GP::PROP_CONSTANT>().getSIDouble(0);
    this->m_time_constant = record.getItem<GP::TIME_CONSTANT>().getSIDouble(0);
}


GPMaint::FlowTarget GPMaint::flow_target() const {
    return this->m_flow_target;
}

double GPMaint::pressure_target() const {
    return this->m_pressure_target;
}

double GPMaint::prop_constant() const {
    return this->m_prop_constant;
}

double GPMaint::time_constant() const {
    return this->m_time_constant;
}

GPMaint GPMaint::serializeObject() {
    GPMaint gpm;
    gpm.m_flow_target = FlowTarget::SURF_GINJ;
    gpm.m_region_name = "FIPNUM";
    gpm.m_region_number = 26;
    return gpm;
}

std::optional<std::pair<std::string, int>> GPMaint::region() const {
    if (this->m_region_number == 0)
        return {};

    return std::make_pair(this->m_region_name, this->m_region_number);
}

GPMaint::FlowTarget GPMaint::FlowTargetFromString(const std::string& string_value) {
    if (string_value == "PROD")
        return GPMaint::FlowTarget::RESV_PROD;

    if (string_value == "OINJ")
        return GPMaint::FlowTarget::RESV_OINJ;

    if (string_value == "WINJ")
        return GPMaint::FlowTarget::RESV_WINJ;

    if (string_value == "GINJ")
        return GPMaint::FlowTarget::RESV_GINJ;

    if (string_value == "OINS")
        return GPMaint::FlowTarget::SURF_OINJ;

    if (string_value == "WINS")
        return GPMaint::FlowTarget::SURF_WINJ;

    if (string_value == "GINS")
        return GPMaint::FlowTarget::SURF_GINJ;

    throw std::invalid_argument("The string: " + string_value + " could not be converted to a valid FLOW target");
}


bool GPMaint::operator==(const GPMaint& other) const {
    return this->m_flow_target == other.m_flow_target &&
           this->m_region_name == other.m_region_name &&
           this->m_region_number == other.m_region_number;
}

}
