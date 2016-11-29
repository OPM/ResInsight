/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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


#include <cassert>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellEconProductionLimits.hpp>


namespace Opm {

    WellEconProductionLimits::WellEconProductionLimits()
        : m_min_oil_rate(0.0)
        , m_min_gas_rate(0.0)
        , m_max_water_cut(0.0)
        , m_max_gas_oil_ratio(0.0)
        , m_max_water_gas_ratio(0.0)
        , m_workover(WellEcon::NONE)
        , m_end_run(false)
        , m_followon_well("'")
        , m_quantity_limit(WellEcon::RATE)
        , m_secondary_max_water_cut(0.0)
        , m_workover_secondary(m_workover)
        , m_max_gas_liquid_ratio(0.0)
        , m_min_liquid_rate(0.0)
        , m_max_temperature(-1e8)
        , m_min_reservoir_fluid_rate(0.0)
    {
    }



    WellEconProductionLimits::WellEconProductionLimits(const DeckRecord& record)
        : m_min_oil_rate(record.getItem("MIN_OIL_PRODUCTION").getSIDouble(0))
        , m_min_gas_rate(record.getItem("MIN_GAS_PRODUCTION").getSIDouble(0))
        , m_max_water_cut(record.getItem("MAX_WATER_CUT").get<double>(0))
        , m_max_gas_oil_ratio(record.getItem("MAX_GAS_OIL_RATIO").get<double>(0))
        , m_max_water_gas_ratio(record.getItem("MAX_WATER_GAS_RATIO").get<double>(0))
        , m_workover(WellEcon::WorkoverEnumFromString(record.getItem("WORKOVER_RATIO_LIMIT").getTrimmedString(0)))
        , m_end_run(false)
        , m_followon_well(record.getItem("FOLLOW_ON_WELL").getTrimmedString(0))
        , m_quantity_limit(WellEcon::QuantityLimitEnumFromString(record.getItem("LIMITED_QUANTITY").getTrimmedString(0)))
        , m_secondary_max_water_cut(record.getItem("SECOND_MAX_WATER_CUT").get<double>(0))
        , m_max_gas_liquid_ratio(record.getItem("MAX_GAS_LIQUID_RATIO").get<double>(0))
        , m_min_liquid_rate(record.getItem("MIN_LIQUID_PRODCUTION_RATE").getSIDouble(0))
        , m_min_reservoir_fluid_rate(record.getItem("MIN_RES_FLUID_RATE").getSIDouble(0))
    {
        assert(m_workover != WellEcon::LAST);
        assert(m_workover != WellEcon::RED);

        if (record.getItem("MAX_TEMP").hasValue(0)) {
            m_max_temperature = record.getItem("MAX_TEMP").getSIDouble(0);
        } else {
            // defaulted with one really non-physical value.
            m_max_temperature = -1e8;
        }

        if (record.getItem("WORKOVER_SECOND_WATER_CUT_LIMIT").hasValue(0)) {
            const std::string string_workover = record.getItem("WORKOVER_SECOND_WATER_CUT_LIMIT").getTrimmedString(0);
            m_workover_secondary = WellEcon::WorkoverEnumFromString(string_workover);
        } else {
            m_workover_secondary = m_workover;
        }

        if (record.getItem("END_RUN_FLAG").hasValue(0)) {
            std::string string_endrun = record.getItem("END_RUN_FLAG").getTrimmedString(0);
            if (string_endrun == "YES") {
                m_end_run = true;
            } else if (string_endrun != "NO") {
                throw std::invalid_argument("Unknown input: " + string_endrun + " for END_RUN_FLAG in WECON");
            }
        }
    }


    bool WellEconProductionLimits::operator==(const WellEconProductionLimits& other) const {
        if ((minOilRate() == other.minOilRate()) &&
            (minGasRate() == other.minGasRate()) &&
            (maxWaterCut() == other.maxWaterCut()) &&
            (maxGasOilRatio() == other.maxGasOilRatio()) &&
            (maxWaterGasRatio() == other.maxWaterGasRatio()) &&
            (workover() == other.workover()) &&
            (followonWell() == other.followonWell()) &&
            (quantityLimit() == other.quantityLimit()) &&
            (maxSecondaryMaxWaterCut() == other.maxSecondaryMaxWaterCut()) &&
            (workoverSecondary() == other.workoverSecondary()) &&
            (maxGasLiquidRatio() == other.maxGasLiquidRatio()) &&
            (minLiquidRate() == other.minLiquidRate()) &&
            (maxTemperature() == other.maxTemperature()) &&
            (minReservoirFluidRate() == other.minReservoirFluidRate()))
        {

            return true;
        } else {
            return false;
        }
    }


    bool WellEconProductionLimits::operator!=(const WellEconProductionLimits& other) const {
        return (!(*this == other));
    }

} // namespace Opm
