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

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/Schedule/Well/WellEconProductionLimits.hpp>


namespace Opm {

    WellEconProductionLimits::WellEconProductionLimits()
        : m_min_oil_rate(0.0)
        , m_min_gas_rate(0.0)
        , m_max_water_cut(0.0)
        , m_max_gas_oil_ratio(0.0)
        , m_max_water_gas_ratio(0.0)
        , m_workover(EconWorkover::NONE)
        , m_end_run(false)
        , m_followon_well("'")
        , m_quantity_limit(QuantityLimit::RATE)
        , m_secondary_max_water_cut(0.0)
        , m_workover_secondary(m_workover)
        , m_max_gas_liquid_ratio(0.0)
        , m_min_liquid_rate(0.0)
        , m_max_temperature(-1e8)
        , m_min_reservoir_fluid_rate(0.0)
    {
    }


    WellEconProductionLimits::WellEconProductionLimits(const DeckRecord& record)
        : m_min_oil_rate(record.getItem("MIN_OIL_PRODUCTION").get<UDAValue>(0).getSI())
        , m_min_gas_rate(record.getItem("MIN_GAS_PRODUCTION").get<UDAValue>(0).getSI())
        , m_max_water_cut(record.getItem("MAX_WATER_CUT").get<UDAValue>(0).getSI())
        , m_max_gas_oil_ratio(record.getItem("MAX_GAS_OIL_RATIO").get<UDAValue>(0).getSI())
        , m_max_water_gas_ratio(record.getItem("MAX_WATER_GAS_RATIO").get<UDAValue>(0).getSI())
        , m_workover(EconWorkoverFromString(record.getItem("WORKOVER_RATIO_LIMIT").getTrimmedString(0)))
        , m_end_run(false)
        , m_followon_well(record.getItem("FOLLOW_ON_WELL").getTrimmedString(0))
        , m_quantity_limit(QuantityLimitFromString(record.getItem("LIMITED_QUANTITY").getTrimmedString(0)))
        , m_secondary_max_water_cut(record.getItem("SECOND_MAX_WATER_CUT").get<double>(0))
        , m_max_gas_liquid_ratio(record.getItem("MAX_GAS_LIQUID_RATIO").get<double>(0))
        , m_min_liquid_rate(record.getItem("MIN_LIQUID_PRODCUTION_RATE").getSIDouble(0))
        , m_min_reservoir_fluid_rate(record.getItem("MIN_RES_FLUID_RATE").getSIDouble(0))
    {
        assert(m_workover != EconWorkover::LAST);
        assert(m_workover != EconWorkover::RED);

        if (record.getItem("MAX_TEMP").hasValue(0)) {
            m_max_temperature = record.getItem("MAX_TEMP").getSIDouble(0);
        } else {
            // defaulted with one really non-physical value.
            m_max_temperature = -1e8;
        }

        if (record.getItem("WORKOVER_SECOND_WATER_CUT_LIMIT").hasValue(0)) {
            const std::string string_workover = record.getItem("WORKOVER_SECOND_WATER_CUT_LIMIT").getTrimmedString(0);
            m_workover_secondary = EconWorkoverFromString(string_workover);
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

    WellEconProductionLimits WellEconProductionLimits::serializeObject()
    {
        WellEconProductionLimits result;
        result.m_min_oil_rate = 1.0;
        result.m_min_gas_rate = 2.0;
        result.m_max_water_cut = 3.0;
        result.m_max_gas_oil_ratio = 4.0;
        result.m_max_water_gas_ratio = 5.0;
        result.m_workover = EconWorkover::CONP;
        result.m_end_run = true;
        result.m_followon_well = "test";
        result.m_quantity_limit = QuantityLimit::POTN;
        result.m_secondary_max_water_cut = 6.0;
        result.m_workover_secondary = EconWorkover::WELL;
        result.m_max_gas_liquid_ratio = 7.0;
        result.m_min_liquid_rate = 8.0;
        result.m_max_temperature = 9.0;
        result.m_min_reservoir_fluid_rate = 10.0;

        return result;
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

// limit switch on?
bool WellEconProductionLimits::onAnyEffectiveLimit() const {
    return (onAnyRatioLimit() || onAnyRateLimit());
}

bool WellEconProductionLimits::onAnyRatioLimit() const {
    return (onMaxWaterCut() || onMaxGasOilRatio() || onMaxWaterGasRatio() ||
            onMaxGasLiquidRatio());
}

bool WellEconProductionLimits::onAnyRateLimit() const {
    return (onMinOilRate() || onMinGasRate() || onMinLiquidRate() ||
            onMinReservoirFluidRate());
}

bool WellEconProductionLimits::onMinOilRate() const { return (m_min_oil_rate > 0.0); }

bool WellEconProductionLimits::onMinGasRate() const { return (m_min_gas_rate > 0.0); }

bool WellEconProductionLimits::onMaxWaterCut() const { return (m_max_water_cut > 0.0); }

bool WellEconProductionLimits::onMaxGasOilRatio() const { return (m_max_gas_oil_ratio > 0.0); }

bool WellEconProductionLimits::onMaxWaterGasRatio() const { return (m_max_water_gas_ratio > 0.0); }

bool WellEconProductionLimits::onSecondaryMaxWaterCut() const {
    return (m_secondary_max_water_cut > 0.0);
}

bool WellEconProductionLimits::onMaxGasLiquidRatio() const { return (m_max_gas_liquid_ratio > 0.0); }

// assuming Celsius temperature is used internally
bool WellEconProductionLimits::onMaxTemperature() const { return (m_max_temperature > -273.15); }

bool WellEconProductionLimits::onMinLiquidRate() const { return (m_min_liquid_rate > 0.0); }

bool WellEconProductionLimits::onMinReservoirFluidRate() const {
    return (m_min_reservoir_fluid_rate > 0.0);
}

// not sure what will happen if the followon well is a well does not exist.
bool WellEconProductionLimits::validFollowonWell() const { return (m_followon_well != "'"); }

bool WellEconProductionLimits::requireWorkover() const {
    return (m_workover != EconWorkover::NONE);
}

bool WellEconProductionLimits::requireSecondaryWorkover() const {
    return (m_workover_secondary != EconWorkover::NONE);
}

bool WellEconProductionLimits::endRun() const { return m_end_run; }

double WellEconProductionLimits::minOilRate() const { return m_min_oil_rate; }

double WellEconProductionLimits::minGasRate() const { return m_min_gas_rate; }

double WellEconProductionLimits::maxWaterCut() const { return m_max_water_cut; }

double WellEconProductionLimits::maxGasOilRatio() const { return m_max_gas_oil_ratio; }

double WellEconProductionLimits::maxWaterGasRatio() const { return m_max_water_gas_ratio; }

double WellEconProductionLimits::maxGasLiquidRatio() const { return m_max_gas_liquid_ratio; }

double WellEconProductionLimits::minLiquidRate() const { return m_min_liquid_rate; }

double WellEconProductionLimits::maxTemperature() const { return m_max_temperature; }

double WellEconProductionLimits::minReservoirFluidRate() const { return m_min_reservoir_fluid_rate; }

WellEconProductionLimits::EconWorkover WellEconProductionLimits::workover() const { return m_workover; }

const std::string& WellEconProductionLimits::followonWell() const { return m_followon_well; }

WellEconProductionLimits::QuantityLimit WellEconProductionLimits::quantityLimit() const {
    return m_quantity_limit;
}

double WellEconProductionLimits::maxSecondaryMaxWaterCut() const {
    return m_secondary_max_water_cut;
}

WellEconProductionLimits::EconWorkover WellEconProductionLimits::workoverSecondary() const {
    return m_workover_secondary;
}

std::string WellEconProductionLimits::EconWorkover2String(EconWorkover enumValue) {
    switch(enumValue) {
    case EconWorkover::NONE:
        return "NONE";
    case EconWorkover::CON:
      return "CON";
    case EconWorkover::CONP:
      return "+CON";
    case EconWorkover::WELL:
      return "WELL";
    case EconWorkover::PLUG:
      return "PLUG";
    case EconWorkover::LAST:
      return "LAST";
    case EconWorkover::RED:
      return "RED";
    default:
      throw std::invalid_argument("unhandled WorkoverEnum value");
    }
}


WellEconProductionLimits::EconWorkover WellEconProductionLimits::EconWorkoverFromString(const std::string& stringValue) {
    if (stringValue == "NONE")
        return EconWorkover::NONE;
    else if (stringValue == "CON")
        return EconWorkover::CON;
    else if (stringValue == "+CON")
        return EconWorkover::CONP;
    else if (stringValue == "WELL")
        return EconWorkover::WELL;
    else if (stringValue == "PLUG")
        return EconWorkover::PLUG;
    else if (stringValue == "LAST")
        return EconWorkover::LAST;
    else if (stringValue == "RED")
        return EconWorkover::RED;
    else
        throw std::invalid_argument("Unknown enum stringValue: " + stringValue +
                                    " for WorkoverEnum");
}

const std::string WellEconProductionLimits::QuantityLimit2String(QuantityLimit enumValue) {
    switch(enumValue) {
    case QuantityLimit::RATE:
        return "RATE";
    case QuantityLimit::POTN:
        return "POTN";
    default:
        throw std::invalid_argument("unhandled QuantityLimitvalue");
    }
}

WellEconProductionLimits::QuantityLimit WellEconProductionLimits::QuantityLimitFromString(const std::string& string ) {
    if (string == "RATE") {
        return QuantityLimit::RATE;
    } else if (string == "POTN") {
        return QuantityLimit::POTN;
    } else {
        throw std::invalid_argument("Unknown enum string: " + string + " for QuantityLimitEnum");
    }
}


} // namespace Opm
