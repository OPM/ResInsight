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

#include <unordered_set>

#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/RockConfig.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>

namespace Opm {

namespace {



std::string num_prop(const std::string& prop_name) {
    static const std::unordered_set<std::string> prop_names = {"PVTNUM", "SATNUM", "ROCKNUM"};
    if (prop_names.count(prop_name) == 1)
        return prop_name;

    throw std::invalid_argument("The rocknum propertype: " + prop_name + " is not valid");
}


RockConfig::Hysteresis hysteresis(const std::string& s) {
    if (s == "REVERS")
        return RockConfig::Hysteresis::REVERS;

    if (s == "IRREVERS")
        return RockConfig::Hysteresis::IRREVERS;

    if (s == "HYSTER")
        return RockConfig::Hysteresis::HYSTER;

    if (s == "BOBERG")
        return RockConfig::Hysteresis::BOBERG;

    if (s == "REVLIMIT")
        return RockConfig::Hysteresis::REVLIMIT;

    if (s == "PALM-MAN")
        return RockConfig::Hysteresis::PALM_MAN;

    if (s == "NONE")
        return RockConfig::Hysteresis::NONE;

    throw std::invalid_argument("Not recognized hysteresis option: " + s);
}

}

RockConfig::RockConfig() :
    num_property(ParserKeywords::ROCKOPTS::TABLE_TYPE::defaultValue),
    num_tables(ParserKeywords::ROCKCOMP::NTROCC::defaultValue)
{
}

bool RockConfig::RockComp::operator==(const RockConfig::RockComp& other) const {
    return this->pref == other.pref &&
           this->compressibility == other.compressibility;
}

RockConfig::RockComp::RockComp(double pref_arg, double comp_arg) :
    pref(pref_arg),
    compressibility(comp_arg)
{}

RockConfig RockConfig::serializeObject()
{
    RockConfig result;
    result.m_active = true;
    result.m_comp = {{100, 0.25}, {200, 0.30}};
    result.num_property = "ROCKNUM";
    result.num_tables = 10;
    result.m_water_compaction = false;
    result.hyst_mode = Hysteresis::HYSTER;

    return result;
}

bool RockConfig::water_compaction() const {
    return this->m_water_compaction;
}

const std::vector<RockConfig::RockComp>& RockConfig::comp() const {
    return this->m_comp;
}

std::size_t RockConfig::num_rock_tables() const {
    return this->num_tables;
}

const std::string& RockConfig::rocknum_property() const {
    return this->num_property;
}

RockConfig::Hysteresis RockConfig::hysteresis_mode() const {
    return this->hyst_mode;
}

bool RockConfig::active() const {
    return this->m_active;
}

RockConfig::RockConfig(const Deck& deck, const FieldPropsManager& fp)
{
    using rock = ParserKeywords::ROCK;
    using rockopts = ParserKeywords::ROCKOPTS;
    using rockcomp = ParserKeywords::ROCKCOMP;
    if (deck.hasKeyword<rock>()) {
        const auto& rock_kw = deck.get<rock>().back();
        for (const auto& record : rock_kw)
            this->m_comp.emplace_back( record.getItem<rock::PREF>().getSIDouble(0),
                                       record.getItem<rock::COMPRESSIBILITY>().getSIDouble(0));
    }

    if (deck.hasKeyword<rockopts>()) {
        const auto& record = deck.get<rockopts>().back().getRecord(0);
        this->num_property = num_prop( record.getItem<rockopts::TABLE_TYPE>().getTrimmedString(0) );
    }

    if (deck.hasKeyword<rockcomp>()) {
        const auto& record = deck.get<rockcomp>().back().getRecord(0);
        if (fp.has_int("ROCKNUM"))
            this->num_property = "ROCKNUM";

        this->num_tables = record.getItem<rockcomp::NTROCC>().get<int>(0);
        this->hyst_mode = hysteresis(record.getItem<rockcomp::HYSTERESIS>().getTrimmedString(0));
        this->m_water_compaction = DeckItem::to_bool(record.getItem<rockcomp::WATER_COMPACTION>().getTrimmedString(0));

        this->m_active = true;
        if (this->hyst_mode == Hysteresis::NONE && !this->m_water_compaction)
            this->m_active = false;
    }
}


bool RockConfig::operator==(const RockConfig& other) const {
    return this->num_property == other.num_property &&
           this->m_comp == other.m_comp &&
           this->num_tables == other.num_tables &&
           this->m_active == other.m_active &&
           this->m_water_compaction == other.m_water_compaction &&
           this->hyst_mode == other.hyst_mode;
}

} //namespace Opm
