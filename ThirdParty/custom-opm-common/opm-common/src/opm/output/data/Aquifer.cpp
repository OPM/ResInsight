/*
  Copyright 2021 Equinor ASA.

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

#include <opm/output/data/Aquifer.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <variant>

bool Opm::data::CarterTracyData::operator==(const CarterTracyData& other) const
{
    return (this->timeConstant == other.timeConstant)
        && (this->influxConstant == other.influxConstant)
        && (this->waterDensity == other.waterDensity)
        && (this->waterViscosity == other.waterViscosity)
        && (this->dimensionless_time == other.dimensionless_time)
        && (this->dimensionless_pressure == other.dimensionless_pressure);
}

bool Opm::data::FetkovichData::operator==(const FetkovichData& other) const
{
    return (this->initVolume == other.initVolume)
        && (this->prodIndex == other.prodIndex)
        && (this->timeConstant == other.timeConstant);
}

bool Opm::data::NumericAquiferData::operator==(const NumericAquiferData& other) const
{
    return this->initPressure == other.initPressure;
}

void Opm::data::TypeSpecificData::create(const std::size_t option)
{
    switch (option) {
    case std::size_t{0}:
    // Default constructor.  Nothing to do here
    break;

    case std::size_t{1}:
    this->options_.emplace<std::variant_alternative_t<1, Types>>();
    break;

    case std::size_t{2}:
    this->options_.emplace<std::variant_alternative_t<2, Types>>();
    break;

    case std::size_t{3}:
    this->options_.emplace<std::variant_alternative_t<3, Types>>();
    break;
    }
}

double Opm::data::AquiferData::get(const std::string& key) const
{
    auto func = summaryValueDispatchTable_.find(key);
    if (func == summaryValueDispatchTable_.end()) {
        return 0.0;
    }

    return (this->*func->second)();
}

bool Opm::data::AquiferData::operator==(const AquiferData& other) const
{
    return (this->aquiferID == other.aquiferID)
        && (this->pressure == other.pressure)
        && (this->fluxRate == other.fluxRate)
        && (this->volume == other.volume)
        && (this->initPressure == other.initPressure)
        && (this->datumDepth == other.datumDepth)
        && (this->typeData == other.typeData);
}

double Opm::data::AquiferData::aquiferPressure() const
{
    return this->pressure;
}

double Opm::data::AquiferData::aquiferFlowRate() const
{
    return this->fluxRate;
}

double Opm::data::AquiferData::aquiferTotalProduction() const
{
    return this->volume;
}

double Opm::data::AquiferData::carterTracyDimensionlessTime() const
{
    return this->typeData.is<AquiferType::CarterTracy>()
        ? this->typeData.get<AquiferType::CarterTracy>()->dimensionless_time
        : 0.0;
}

double Opm::data::AquiferData::carterTracyDimensionlessPressure() const
{
    return this->typeData.is<AquiferType::CarterTracy>()
        ? this->typeData.get<AquiferType::CarterTracy>()->dimensionless_pressure
        : 0.0;
}

Opm::data::AquiferData::SummaryValueDispatchTable
Opm::data::AquiferData::summaryValueDispatchTable_ =
{
    {"AAQP" , &AquiferData::aquiferPressure},
    {"ANQP" , &AquiferData::aquiferPressure},
    {"AAQR" , &AquiferData::aquiferFlowRate},
    {"ANQR" , &AquiferData::aquiferFlowRate},
    {"AAQT" , &AquiferData::aquiferTotalProduction},
    {"ANQT" , &AquiferData::aquiferTotalProduction},
    {"AAQTD", &AquiferData::carterTracyDimensionlessTime},
    {"AAQPD", &AquiferData::carterTracyDimensionlessPressure},
};
