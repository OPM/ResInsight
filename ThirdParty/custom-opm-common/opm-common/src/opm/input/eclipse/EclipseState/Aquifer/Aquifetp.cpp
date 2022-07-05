/*
  Copyright (C) 2017 TNO

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

#include <opm/input/eclipse/EclipseState/Aquifer/Aquifetp.hpp>

#include <opm/io/eclipse/rst/aquifer.hpp>

#include <opm/input/eclipse/EclipseState/Tables/FlatTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>

#include <vector>

namespace {
    Opm::Aquifetp::AQUFETP_data
    makeAquifer(const Opm::RestartIO::RstAquifer::Fetkovich& rst_aquifer)
    {
        auto aquifer = Opm::Aquifetp::AQUFETP_data{};

        aquifer.aquiferID  = rst_aquifer.aquiferID;
        aquifer.pvttableID = rst_aquifer.pvttableID;

        aquifer.prod_index        = rst_aquifer.prod_index;
        aquifer.total_compr       = rst_aquifer.total_compr;
        aquifer.initial_watvolume = rst_aquifer.initial_watvolume;
        aquifer.datum_depth       = rst_aquifer.datum_depth;
        aquifer.initial_pressure  = rst_aquifer.initial_pressure;

        return aquifer;
    }
}

namespace Opm {

using AQUFETP = ParserKeywords::AQUFETP;

Aquifetp::AQUFETP_data::AQUFETP_data(const DeckRecord& record, const TableManager& tables)
    : aquiferID        (record.getItem<AQUFETP::AQUIFER_ID>().get<int>(0))
    , pvttableID       (record.getItem<AQUFETP::TABLE_NUM_WATER_PRESS>().get<int>(0))
    , prod_index       (record.getItem<AQUFETP::PI>().getSIDouble(0))
    , total_compr      (record.getItem<AQUFETP::C_T>().getSIDouble(0))
    , initial_watvolume(record.getItem<AQUFETP::V0>().getSIDouble(0))
    , datum_depth      (record.getItem<AQUFETP::DAT_DEPTH>().getSIDouble(0))
{
    if (record.getItem<AQUFETP::P0>().hasValue(0))
        this->initial_pressure = record.getItem<AQUFETP::P0>().getSIDouble(0);

    this->finishInitialisation(tables);
}

bool Aquifetp::AQUFETP_data::operator==(const Aquifetp::AQUFETP_data& other) const
{
    return (this->aquiferID == other.aquiferID)
        && (this->pvttableID == other.pvttableID)
        && (this->prod_index == other.prod_index)
        && (this->total_compr == other.total_compr)
        && (this->initial_watvolume == other.initial_watvolume)
        && (this->datum_depth == other.datum_depth)
        && (this->initial_pressure == other.initial_pressure)
        && (this->timeConstant() == other.timeConstant())
        && (this->waterDensity() == other.waterDensity())
        && (this->waterViscosity() == other.waterViscosity())
        ;
}

Aquifetp::AQUFETP_data::AQUFETP_data(const int aquiferID_,
                                     const int pvttableID_,
                                     const double J_,
                                     const double C_t_,
                                     const double V0_,
                                     const double d0_,
                                     const double p0_)
    : aquiferID        (aquiferID_)
    , pvttableID       (pvttableID_)
    , prod_index       (J_)
    , total_compr      (C_t_)
    , initial_watvolume(V0_)
    , datum_depth      (d0_)
    , initial_pressure (p0_)
{}

Aquifetp::AQUFETP_data Aquifetp::AQUFETP_data::serializeObject()
{
    auto ret = AQUFETP_data {
        1, 2, 3.0, 4.0, 5.0, 6.0, 7.0
    };

    ret.time_constant_ = 8.0;
    ret.water_density_ = 9.0;
    ret.water_viscosity_ = 10.0;

    return ret;
}

void Aquifetp::AQUFETP_data::finishInitialisation(const TableManager& tables)
{
    this->time_constant_ = this->total_compr * this->initial_watvolume / this->prod_index;

    const auto& pvtwTables = tables.getPvtwTable();
    const auto& densityTables = tables.getDensityTable();
    if (this->initial_pressure.has_value() &&
        !pvtwTables.empty() &&
        !densityTables.empty())
    {
        const auto& pvtw = pvtwTables[this->pvttableID - 1];
        const auto& density = densityTables[this->pvttableID - 1];

        const auto press_diff = this->initial_pressure.value() - pvtw.reference_pressure;
        const auto BW = pvtw.volume_factor * (1.0 - pvtw.compressibility*press_diff);

        this->water_viscosity_ = pvtw.viscosity * (1.0 + pvtw.viscosibility*press_diff);
        this->water_density_ = density.water / BW;
    }
}

Aquifetp::Aquifetp(const TableManager& tables, const Deck& deck)
{
    if (!deck.hasKeyword<AQUFETP>())
        return;

    const auto& aqufetpKeyword = deck.get<AQUFETP>().back();
    OpmLog::info(OpmInputError::format("Initializing Fetkovich aquifers from {keyword} in {file} line {line}", aqufetpKeyword.location()));
    for (auto& record : aqufetpKeyword)
        this->m_aqufetp.emplace_back(record, tables);
}


Aquifetp::Aquifetp(const std::vector<Aquifetp::AQUFETP_data>& data) :
    m_aqufetp(data)
{}

void Aquifetp::loadFromRestart(const RestartIO::RstAquifer& rst,
                               const TableManager&          tables)
{
    this->m_aqufetp.clear();

    const auto& rst_aquifers = rst.fetkovich();
    if (! rst_aquifers.empty()) {
        this->m_aqufetp.reserve(rst_aquifers.size());
    }

    for (const auto& rst_aquifer : rst_aquifers) {
        this->m_aqufetp.push_back(makeAquifer(rst_aquifer));

        auto& new_aquifer = this->m_aqufetp.back();
        new_aquifer.finishInitialisation(tables);

        // Assign 'private:' members in AQUFETP_data.
        new_aquifer.time_constant_ = rst_aquifer.time_constant;
    }
}

Aquifetp Aquifetp::serializeObject()
{
    Aquifetp result;
    result.m_aqufetp = { AQUFETP_data::serializeObject() };

    return result;
}


const std::vector<Aquifetp::AQUFETP_data>& Aquifetp::data() const
{
    return m_aqufetp;
}


bool Aquifetp::operator==(const Aquifetp& other) const {
    return this->m_aqufetp == other.m_aqufetp;
}

std::size_t Aquifetp::size() const {
    return this->m_aqufetp.size();
}

std::vector<Aquifetp::AQUFETP_data>::const_iterator Aquifetp::begin() const {
    return this->m_aqufetp.begin();
}


std::vector<Aquifetp::AQUFETP_data>::const_iterator Aquifetp::end() const {
    return this->m_aqufetp.end();
}

bool Aquifetp::hasAquifer(const int aquID) const {
    return std::any_of(this->m_aqufetp.begin(), this->m_aqufetp.end(),
                       [&aquID](const auto& aqu) { return aqu.aquiferID == aquID; });

}

}
