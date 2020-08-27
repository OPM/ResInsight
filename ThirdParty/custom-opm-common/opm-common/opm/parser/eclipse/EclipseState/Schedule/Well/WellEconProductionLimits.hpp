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

namespace Opm {

    class DeckRecord;

    class WellEconProductionLimits {
    public:

        enum class QuantityLimit {
            RATE = 0,
            POTN = 1
        };

        static const std::string QuantityLimit2String(QuantityLimit enumValue);
        static QuantityLimit QuantityLimitFromString(const std::string& stringValue);


        enum class EconWorkover {
            NONE = 0,
            CON  = 1, // CON
            CONP = 2, // +CON
            WELL = 3,
            PLUG = 4,
            // the following two only related to workover action
            // on exceeding secondary water cut limit
            LAST = 5,
            RED  = 6
        };
        static std::string EconWorkover2String(EconWorkover enumValue);
        static EconWorkover EconWorkoverFromString(const std::string& stringValue);


        explicit WellEconProductionLimits(const DeckRecord& record);
        WellEconProductionLimits();

        static WellEconProductionLimits serializeObject();

        // TODO: not handling things related to m_secondary_max_water_cut
        // for the moment.

        // limit switch on?
        bool onAnyEffectiveLimit() const;
        bool onAnyRatioLimit() const;
        bool onAnyRateLimit() const;
        bool onMinOilRate() const;
        bool onMinGasRate() const;
        bool onMaxWaterCut() const;
        bool onMaxGasOilRatio() const;
        bool onMaxWaterGasRatio() const;
        bool onSecondaryMaxWaterCut() const;
        bool onMaxGasLiquidRatio() const;
        // assuming Celsius temperature is used internally;
        bool onMaxTemperature() const;
        bool onMinLiquidRate() const;
        bool onMinReservoirFluidRate() const;
        // not sure what will happen if the followon well is a well does not exist.;
        bool validFollowonWell() const;
        bool requireWorkover() const;
        bool requireSecondaryWorkover() const;
        bool endRun() const;
        double minOilRate() const;
        double minGasRate() const;
        double maxWaterCut() const;
        double maxGasOilRatio() const;
        double maxWaterGasRatio() const;
        EconWorkover workover() const;
        const std::string& followonWell() const;
        QuantityLimit quantityLimit() const;
        double maxSecondaryMaxWaterCut() const;
        EconWorkover workoverSecondary() const;
        double maxGasLiquidRatio() const;
        double minLiquidRate() const;
        double maxTemperature() const;
        double minReservoirFluidRate() const;
        bool operator==(const WellEconProductionLimits& other) const;
        bool operator!=(const WellEconProductionLimits& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_min_oil_rate);
            serializer(m_min_gas_rate);
            serializer(m_max_water_cut);
            serializer(m_max_gas_oil_ratio);
            serializer(m_max_water_gas_ratio);
            serializer(m_workover);
            serializer(m_end_run);
            serializer(m_followon_well);
            serializer(m_quantity_limit);
            serializer(m_secondary_max_water_cut);
            serializer(m_workover_secondary);
            serializer(m_max_gas_liquid_ratio);
            serializer(m_min_liquid_rate);
            serializer(m_max_temperature);
            serializer(m_min_reservoir_fluid_rate);
        }

    private:
        double m_min_oil_rate;
        double m_min_gas_rate;
        double m_max_water_cut;
        double m_max_gas_oil_ratio;
        double m_max_water_gas_ratio;
        EconWorkover m_workover;
        bool m_end_run;
        std::string m_followon_well;
        QuantityLimit m_quantity_limit;
        double m_secondary_max_water_cut;
        EconWorkover m_workover_secondary;
        double m_max_gas_liquid_ratio;
        double m_min_liquid_rate;
        double m_max_temperature;
        double m_min_reservoir_fluid_rate;
    };
} // namespace Opm

#endif  // WELLECONPRODUCTIONLIMITS_HPP_HEADER_INCLUDED
