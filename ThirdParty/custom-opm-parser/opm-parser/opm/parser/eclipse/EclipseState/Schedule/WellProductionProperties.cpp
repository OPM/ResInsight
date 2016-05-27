/*
  Copyright 2016 Statoil ASA.

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
#include <string>
#include <vector>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellProductionProperties.hpp>


namespace Opm {

    WellProductionProperties::
    WellProductionProperties() : predictionMode( true )
    {}

    WellProductionProperties::
    WellProductionProperties( const DeckRecord& record ) :
        OilRate( record.getItem( "ORAT" ).getSIDouble( 0 ) ),
        WaterRate( record.getItem( "WRAT" ).getSIDouble( 0 ) ),
        GasRate( record.getItem( "GRAT" ).getSIDouble( 0 ) )
    {}


    WellProductionProperties WellProductionProperties::history(double BHPLimit, const DeckRecord& record)
    {
        // Modes supported in WCONHIST just from {O,W,G}RAT values
        //
        // Note: The default value of observed {O,W,G}RAT is zero
        // (numerically) whence the following control modes are
        // unconditionally supported.
        WellProductionProperties p(record);
        p.predictionMode = false;

        for (auto const& modeKey : {"ORAT", "WRAT", "GRAT", "LRAT", "RESV", "GRUP"}) {
            auto cmode = WellProducer::ControlModeFromString( modeKey );
            p.addProductionControl( cmode );
        }

        /*
          We do not update the BHPLIMIT based on the BHP value given
          in WCONHIST, that is purely a historical value; instead we
          copy the old value of the BHP limit from the previous
          timestep.

          To actually set the BHPLIMIT in historical mode you must
          use the WELTARG keyword.
        */
        p.BHPLimit = BHPLimit;

        const auto& cmodeItem = record.getItem("CMODE");
        if (!cmodeItem.defaultApplied(0)) {
            const WellProducer::ControlModeEnum cmode = WellProducer::ControlModeFromString( cmodeItem.get< std::string >(0) );

            if (p.hasProductionControl( cmode ))
                p.controlMode = cmode;
            else
                throw std::invalid_argument("Setting CMODE to unspecified control");
        }

        return p;
    }



    WellProductionProperties WellProductionProperties::prediction( const DeckRecord& record, bool addGroupProductionControl)
    {
        WellProductionProperties p(record);
        p.predictionMode = true;

        p.LiquidRate     = record.getItem("LRAT"     ).getSIDouble(0);
        p.ResVRate       = record.getItem("RESV"     ).getSIDouble(0);
        p.BHPLimit       = record.getItem("BHP"      ).getSIDouble(0);
        p.THPLimit       = record.getItem("THP"      ).getSIDouble(0);
        p.ALQValue       = record.getItem("ALQ"      ).get< double >(0); //NOTE: Unit of ALQ is never touched
        p.VFPTableNumber = record.getItem("VFP_TABLE").get< int >(0);

        for (auto const& modeKey : {"ORAT", "WRAT", "GRAT", "LRAT","RESV", "BHP" , "THP"}) {
             if (!record.getItem(modeKey).defaultApplied(0)) {
                 auto cmode = WellProducer::ControlModeFromString( modeKey );
                 p.addProductionControl( cmode );
            }
        }

        if (addGroupProductionControl) {
            p.addProductionControl(WellProducer::GRUP);
        }


        {
            const auto& cmodeItem = record.getItem("CMODE");
            if (cmodeItem.hasValue(0)) {
                const WellProducer::ControlModeEnum cmode = WellProducer::ControlModeFromString( cmodeItem.get< std::string >(0) );

                if (p.hasProductionControl( cmode ))
                    p.controlMode = cmode;
                else
                    throw std::invalid_argument("Setting CMODE to unspecified control");
            }
        }
        return p;
    }


    bool WellProductionProperties::operator==(const WellProductionProperties& other) const {
        if ((OilRate == other.OilRate) &&
            (WaterRate == other.WaterRate) &&
            (GasRate == other.GasRate) &&
            (LiquidRate == other.LiquidRate) &&
            (ResVRate == other.ResVRate) &&
            (BHPLimit == other.BHPLimit) &&
            (THPLimit == other.THPLimit) &&
            (VFPTableNumber == other.VFPTableNumber) &&
            (controlMode == other.controlMode) &&
            (m_productionControls == other.m_productionControls))
            return true;
        else
            return false;
    }


    bool WellProductionProperties::operator!=(const WellProductionProperties& other) const {
        return !(*this == other);
    }


} // namespace Opm
