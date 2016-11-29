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

#ifndef WELLECONPRODUCTIONLIMITS_HPP_HEADER_INCLUDED
#define WELLECONPRODUCTIONLIMITS_HPP_HEADER_INCLUDED

#include <string>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

namespace Opm {

    class DeckRecord;

    class WellEconProductionLimits{
    public:
        WellEconProductionLimits(const DeckRecord& record);
        WellEconProductionLimits();

        // TODO: not handling things related to m_secondary_max_water_cut
        // for the moment.

        // limit switch on?
        bool onAnyEffectiveLimit() const {
            return (onAnyRatioLimit() ||
                    onAnyRateLimit());
        };

        bool onAnyRatioLimit() const {
            return (onMaxWaterCut()       ||
                    onMaxGasOilRatio()    ||
                    onMaxWaterGasRatio()  ||
                    onMaxGasLiquidRatio());
        };


        bool onAnyRateLimit() const {
            return (onMinOilRate()            ||
                    onMinGasRate()            ||
                    onMinLiquidRate()         ||
                    onMinReservoirFluidRate());
        };

        bool onMinOilRate() const {
            return (m_min_oil_rate > 0.0);
        };

        bool onMinGasRate() const {
            return (m_min_gas_rate > 0.0);
        };

        bool onMaxWaterCut() const {
            return (m_max_water_cut > 0.0);
        };

        bool onMaxGasOilRatio() const {
            return (m_max_gas_oil_ratio > 0.0);
        };

        bool onMaxWaterGasRatio() const {
            return (m_max_water_gas_ratio > 0.0);
        };

        bool onSecondaryMaxWaterCut() const {
            return (m_secondary_max_water_cut > 0.0);
        };

        bool onMaxGasLiquidRatio() const {
            return (m_max_gas_oil_ratio > 0.0);
        };

        // assuming Celsius temperature is used internally
        bool onMaxTemperature() const {
            return (m_max_temperature > -273.15);
        };

        bool onMinLiquidRate() const {
            return (m_min_liquid_rate > 0.0);
        };

        bool onMinReservoirFluidRate() const {
            return (m_min_reservoir_fluid_rate > 0.0);
        };

        // not sure what will happen if the followon well is a well does not exist.
        bool validFollowonWell() const {
            return (m_followon_well != "'");
        };

        bool requireWorkover() const {
            return (m_workover != WellEcon::NONE);
        };

        bool requireSecondaryWorkover() const {
            return (m_workover_secondary != WellEcon::NONE);
        }

        bool endRun() const {
            return m_end_run;
        }

        double minOilRate() const { return m_min_oil_rate; };

        double minGasRate() const { return m_min_gas_rate; };

        double maxWaterCut() const { return m_max_water_cut; };

        double maxGasOilRatio() const { return m_max_gas_oil_ratio; };

        double maxWaterGasRatio() const { return m_max_water_gas_ratio; };

        WellEcon::WorkoverEnum workover() const { return m_workover; };

        const std::string& followonWell() const { return m_followon_well; };

        WellEcon::QuantityLimitEnum quantityLimit() const {return m_quantity_limit; };

        double maxSecondaryMaxWaterCut() const { return m_secondary_max_water_cut; };

        WellEcon::WorkoverEnum workoverSecondary() const { return m_workover_secondary; };

        double maxGasLiquidRatio() const { return m_max_gas_liquid_ratio; };

        double minLiquidRate() const { return m_min_liquid_rate; };

        double maxTemperature() const { return m_max_temperature; };

        double minReservoirFluidRate() const { return m_min_reservoir_fluid_rate; };

        bool operator==(const WellEconProductionLimits& other) const;

        bool operator!=(const WellEconProductionLimits& other) const;

    private:
        double m_min_oil_rate;
        double m_min_gas_rate;
        double m_max_water_cut;
        double m_max_gas_oil_ratio;
        double m_max_water_gas_ratio;
        WellEcon::WorkoverEnum m_workover;
        bool m_end_run;
        std::string m_followon_well;
        WellEcon::QuantityLimitEnum m_quantity_limit;
        double m_secondary_max_water_cut;
        WellEcon::WorkoverEnum m_workover_secondary;
        double m_max_gas_liquid_ratio;
        double m_min_liquid_rate;
        double m_max_temperature;
        double m_min_reservoir_fluid_rate;
    };
} // namespace Opm

#endif  // WELLECONPRODUCTIONLIMITS_HPP_HEADER_INCLUDED
