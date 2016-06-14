/*
  Copyright 2013 Statoil ASA.

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

#ifndef WELLPRODUCTIONPROPERTIES_HPP_HEADER_INCLUDED
#define WELLPRODUCTIONPROPERTIES_HPP_HEADER_INCLUDED

#include <memory>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

namespace Opm {

    class DeckRecord;

    class WellProductionProperties {
    public:
        double  OilRate     = 0.0;
        double  WaterRate   = 0.0;
        double  GasRate     = 0.0;
        double  LiquidRate  = 0.0;
        double  ResVRate    = 0.0;
        double  BHPLimit    = 0.0;
        double  THPLimit    = 0.0;
        int     VFPTableNumber = 0;
        double  ALQValue    = 0.0;
        bool    predictionMode = false;

        WellProducer::ControlModeEnum controlMode = WellProducer::CMODE_UNDEFINED;

        bool operator==(const WellProductionProperties& other) const;
        bool operator!=(const WellProductionProperties& other) const;
        WellProductionProperties();

        static WellProductionProperties history(double BHPLimit, const DeckRecord& record );
        static WellProductionProperties prediction( const DeckRecord& record, bool addGroupProductionControl );

        bool hasProductionControl(WellProducer::ControlModeEnum controlModeArg) const {
            return (m_productionControls & controlModeArg) != 0;
        }

        void dropProductionControl(WellProducer::ControlModeEnum controlModeArg) {
            if (hasProductionControl(controlModeArg))
                m_productionControls -= controlModeArg;
        }

        void addProductionControl(WellProducer::ControlModeEnum controlModeArg) {
            if (! hasProductionControl(controlModeArg))
                m_productionControls += controlModeArg;
        }

    private:
        int m_productionControls = 0;

        WellProductionProperties(const DeckRecord& record);
    };
} // namespace Opm

#endif  // WELLPRODUCTIONPROPERTIES_HPP_HEADER_INCLUDED
