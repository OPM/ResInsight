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

#include <opm/input/eclipse/EclipseState/Aquifer/AquiferCT.hpp>

#include <opm/io/eclipse/rst/aquifer.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>

#include <opm/input/eclipse/EclipseState/Tables/Aqudims.hpp>
#include <opm/input/eclipse/EclipseState/Tables/AqutabTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/FlatTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableContainer.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <cstddef>
#include <utility>
#include <vector>

namespace {
    Opm::AquiferCT::AQUCT_data
    makeAquifer(const Opm::RestartIO::RstAquifer::CarterTracy& rst_aquifer)
    {
        auto aquifer = Opm::AquiferCT::AQUCT_data{};

        aquifer.aquiferID  = rst_aquifer.aquiferID;
        aquifer.inftableID = rst_aquifer.inftableID;
        aquifer.pvttableID = rst_aquifer.pvttableID;

        aquifer.porosity         = rst_aquifer.porosity;
        aquifer.datum_depth      = rst_aquifer.datum_depth;
        aquifer.total_compr      = rst_aquifer.total_compr;
        aquifer.inner_radius     = rst_aquifer.inner_radius;
        aquifer.permeability     = rst_aquifer.permeability;
        aquifer.thickness        = rst_aquifer.thickness;
        aquifer.angle_fraction   = rst_aquifer.angle_fraction;
        aquifer.initial_pressure = rst_aquifer.initial_pressure;

        return aquifer;
    }
}

namespace Opm {

namespace {

// The default Pd v/s Td tables (constant terminal rate case for an infinite
// aquifer) as described in Van Everdingen, A. & Hurst, W., December, 1949.The
// Application of the Laplace Transformation to Flow Problems in Reservoirs.
// Petroleum Transactions, AIME.

static const std::vector<double> default_pressure = { 0.112, 0.229, 0.315, 0.376, 0.424, 0.469, 0.503, 0.564,
                                                      0.616, 0.659, 0.702, 0.735, 0.772, 0.802, 0.927, 1.020,
                                                      1.101, 1.169, 1.275, 1.362, 1.436, 1.500, 1.556, 1.604,
                                                      1.651, 1.829, 1.960, 2.067, 2.147, 2.282, 2.388, 2.476,
                                                      2.550, 2.615, 2.672, 2.723, 2.921, 3.064, 3.173, 3.263,
                                                      3.406, 3.516, 3.608, 3.684, 3.750, 3.809, 3.86 };

static const std::vector<double> default_time =     { 0.010, 0.050, 0.100, 0.150, 0.200, 0.250, 0.300, 0.400,
                                                      0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.500, 2.000,
                                                      2.500, 3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000,
                                                      10.00, 15.00, 20.00, 25.00, 30.00, 40.00, 50.00, 60.00,
                                                      70.00, 80.00, 90.00, 100.0, 150.0, 200.0, 250.0, 300.0,
                                                      400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000 };

}

AquiferCT::AQUCT_data::AQUCT_data(const DeckRecord& record, const TableManager& tables)
    : aquiferID     (record.getItem<ParserKeywords::AQUCT::AQUIFER_ID>().get<int>(0))
    , inftableID    (record.getItem<ParserKeywords::AQUCT::TABLE_NUM_INFLUENCE_FN>().get<int>(0))
    , pvttableID    (record.getItem<ParserKeywords::AQUCT::TABLE_NUM_WATER_PRESS>().get<int>(0))
    , porosity      (record.getItem<ParserKeywords::AQUCT::PORO_AQ>().getSIDouble(0))
    , datum_depth   (record.getItem<ParserKeywords::AQUCT::DAT_DEPTH>().getSIDouble(0))
    , total_compr   (record.getItem<ParserKeywords::AQUCT::C_T>().getSIDouble(0))
    , inner_radius  (record.getItem<ParserKeywords::AQUCT::RAD>().getSIDouble(0))
    , permeability  (record.getItem<ParserKeywords::AQUCT::PERM_AQ>().getSIDouble(0))
    , thickness     (record.getItem<ParserKeywords::AQUCT::THICKNESS_AQ>().getSIDouble(0))
    , angle_fraction(record.getItem<ParserKeywords::AQUCT::INFLUENCE_ANGLE>().getSIDouble(0) / 360.0)
{
    if (record.getItem<ParserKeywords::AQUCT::P_INI>().hasValue(0))
        this->initial_pressure = record.getItem<ParserKeywords::AQUCT::P_INI>().getSIDouble(0);

    if (record.getItem<ParserKeywords::AQUCT::TEMP_AQUIFER>().hasValue(0))
        this->initial_temperature = record.getItem<ParserKeywords::AQUCT::TEMP_AQUIFER>().getSIDouble(0);

    this->finishInitialisation(tables);
}

bool AquiferCT::AQUCT_data::operator==(const AquiferCT::AQUCT_data& other) const
{
    return (this->aquiferID == other.aquiferID)
        && (this->inftableID == other.inftableID)
        && (this->pvttableID == other.pvttableID)
        && (this->porosity == other.porosity)
        && (this->datum_depth == other.datum_depth)
        && (this->total_compr == other.total_compr)
        && (this->inner_radius == other.inner_radius)
        && (this->permeability == other.permeability)
        && (this->thickness == other.thickness)
        && (this->angle_fraction == other.angle_fraction)
        && (this->initial_pressure == other.initial_pressure)
        && (this->initial_temperature == other.initial_temperature)
        && (this->dimensionless_time == other.dimensionless_time)
        && (this->dimensionless_pressure == other.dimensionless_pressure)
        && (this->timeConstant() == other.timeConstant())
        && (this->influxConstant() == other.influxConstant())
        && (this->waterDensity() == other.waterDensity())
        && (this->waterViscosity() == other.waterViscosity())
        ;
}

AquiferCT::AQUCT_data::AQUCT_data(const int aqID,
                                  const int infID,
                                  const int pvtID,
                                  const double phi_aq_,
                                  const double d0_,
                                  const double C_t_,
                                  const double r_o_,
                                  const double k_a_,
                                  const double h_,
                                  const double theta_,
                                  const double p0_,
                                  const double T0_)
    : aquiferID(aqID)
    , inftableID(infID)
    , pvttableID(pvtID)
    , porosity(phi_aq_)
    , datum_depth(d0_)
    , total_compr(C_t_)
    , inner_radius(r_o_)
    , permeability(k_a_)
    , thickness(h_)
    , angle_fraction(theta_)
    , initial_pressure(p0_)
    , initial_temperature(T0_)
{}

AquiferCT::AQUCT_data AquiferCT::AQUCT_data::serializeObject()
{
    auto ret = AQUCT_data {
        1, 2, 3, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0
    };

    ret.dimensionless_time = std::vector<double>{ 13.0 };
    ret.dimensionless_pressure = std::vector<double>{ 14.0 };
    ret.time_constant_ = 15.0;
    ret.influx_constant_ = 16.0;
    ret.water_density_ = 17.0;
    ret.water_viscosity_ = 18.0;

    return ret;
}

void AquiferCT::AQUCT_data::finishInitialisation(const TableManager& tables)
{
    // Get the correct influence table values
    if (this->inftableID == 1) {
        this->dimensionless_time = default_time;
        this->dimensionless_pressure = default_pressure;
    }
    else if (this->inftableID > 1) {
        const auto& aqutabTable = tables.getAqutabTables().getTable(this->inftableID - 2);
        this->dimensionless_time = aqutabTable.getColumn(0).vectorCopy();
        this->dimensionless_pressure = aqutabTable.getColumn(1).vectorCopy();
    }

    const auto x = this->porosity * this->total_compr * this->inner_radius * this->inner_radius;

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
        this->time_constant_ = this->water_viscosity_ * x / this->permeability;
    }

    const auto tau = 6.283; // ECLIPSE 100's approximation of 2\pi.
    this->influx_constant_ = tau * this->thickness * this->angle_fraction * x;
}

AquiferCT::AquiferCT(const TableManager& tables, const Deck& deck)
{
    using AQUCT = ParserKeywords::AQUCT;
    if (!deck.hasKeyword<AQUCT>())
        return;

    const auto& aquctKeyword = deck.get<AQUCT>().back();
    OpmLog::info(OpmInputError::format("Initializing Carter Tracey aquifers from {keyword} in {file} line {line}", aquctKeyword.location()));
    for (auto& record : aquctKeyword)
        this->m_aquct.emplace_back(record, tables);
}

AquiferCT::AquiferCT(const std::vector<AquiferCT::AQUCT_data>& data) :
    m_aquct(data)
{}

void AquiferCT::loadFromRestart(const RestartIO::RstAquifer& rst,
                                const TableManager&          tables)
{
    this->m_aquct.clear();

    const auto& rst_aquifers = rst.carterTracy();
    if (! rst_aquifers.empty()) {
        this->m_aquct.reserve(rst_aquifers.size());
    }

    for (const auto& rst_aquifer : rst_aquifers) {
        this->m_aquct.push_back(makeAquifer(rst_aquifer));

        auto& new_aquifer = this->m_aquct.back();
        new_aquifer.finishInitialisation(tables);

        // Assign 'private:' members in AQUCT_data.
        new_aquifer.time_constant_   = rst_aquifer.time_constant;
        new_aquifer.influx_constant_ = rst_aquifer.influx_constant;
        new_aquifer.water_density_   = rst_aquifer.water_density;
        new_aquifer.water_viscosity_ = rst_aquifer.water_viscosity;
    }
}

AquiferCT AquiferCT::serializeObject()
{
    return { { AQUCT_data::serializeObject() } };
}

std::size_t AquiferCT::size() const {
    return this->m_aquct.size();
}

std::vector<AquiferCT::AQUCT_data>::const_iterator AquiferCT::begin() const {
    return this->m_aquct.begin();
}

std::vector<AquiferCT::AQUCT_data>::const_iterator AquiferCT::end() const {
    return this->m_aquct.end();
}

bool AquiferCT::operator==(const AquiferCT& other) const {
    return this->m_aquct == other.m_aquct;
}

const std::vector<AquiferCT::AQUCT_data>& AquiferCT::data() const {
    return this->m_aquct;
}

bool AquiferCT::hasAquifer(const int aquID) const {
    return std::any_of(this->m_aquct.begin(), this->m_aquct.end(),
                       [&aquID](const auto& aqu) { return aqu.aquiferID == aquID; });
}

}
