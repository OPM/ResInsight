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
#include <iostream>
#include <string>
#include <vector>

#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleTypes.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>

#include "injection.hpp"
#include "../eval_uda.hpp"

namespace Opm {


    Well::WellInjectionProperties::WellInjectionProperties()
        : WellInjectionProperties(UnitSystem(UnitSystem::UnitType::UNIT_TYPE_METRIC), "")
    {
    }


    Well::WellInjectionProperties::WellInjectionProperties(const UnitSystem& units, const std::string& wname)
        : name(wname),
          surfaceInjectionRate(units.getDimension(UnitSystem::measure::identity)),
          reservoirInjectionRate(units.getDimension(UnitSystem::measure::rate)),
          BHPTarget(units.getDimension(UnitSystem::measure::pressure)),
          THPTarget(units.getDimension(UnitSystem::measure::pressure)),
          temperature(Metric::TemperatureOffset + ParserKeywords::STCOND::TEMPERATURE::defaultValue),
          BHPH(0),
          THPH(0),
          VFPTableNumber(0),
          predictionMode(true),
          injectionControls(0),
          injectorType(InjectorType::WATER),
          controlMode(InjectorCMode::CMODE_UNDEFINED)
    {
    }

    Well::WellInjectionProperties Well::WellInjectionProperties::serializeObject()
    {
        Well::WellInjectionProperties result;
        result.name = "test";
        result.surfaceInjectionRate = UDAValue(1.0);
        result.reservoirInjectionRate = UDAValue("test");
        result.BHPTarget = UDAValue(2.0);
        result.THPTarget = UDAValue(3.0);
        result.bhp_hist_limit = 4.0;
        result.thp_hist_limit = 5.0;
        result.temperature = 6.0;
        result.BHPH = 7.0;
        result.THPH = 8.0;
        result.VFPTableNumber = 9;
        result.predictionMode = true;
        result.injectionControls = 10;
        result.injectorType = InjectorType::OIL;
        result.controlMode = InjectorCMode::BHP;

        return result;
    }

    void Well::WellInjectionProperties::handleWCONINJE(const DeckRecord& record, bool availableForGroupControl, const std::string& well_name) {
        this->injectorType = InjectorTypeFromString( record.getItem("TYPE").getTrimmedString(0) );
        this->predictionMode = true;

        if (!record.getItem("RATE").defaultApplied(0)) {
            this->surfaceInjectionRate = record.getItem("RATE").get<UDAValue>(0);
            this->addInjectionControl(InjectorCMode::RATE);
        } else
            this->dropInjectionControl(InjectorCMode::RATE);


        if (!record.getItem("RESV").defaultApplied(0)) {
            this->reservoirInjectionRate = record.getItem("RESV").get<UDAValue>(0);
            this->addInjectionControl(InjectorCMode::RESV);
        } else
            this->dropInjectionControl(InjectorCMode::RESV);


        if (!record.getItem("THP").defaultApplied(0)) {
            this->THPTarget = record.getItem("THP").get<UDAValue>(0);
            this->addInjectionControl(InjectorCMode::THP);
        } else
            this->dropInjectionControl(InjectorCMode::THP);

        this->VFPTableNumber = record.getItem("VFP_TABLE").get< int >(0);

        /*
          There is a sensible default BHP limit defined, so the BHPLimit can be
          safely set unconditionally, and we make BHP limit as a constraint based
          on that default value. It is not easy to infer from the manual, while the
          current behavoir agrees with the behavior of Eclipse when BHPLimit is not
          specified while employed during group control.
        */
        this->BHPTarget = record.getItem("BHP").get<UDAValue>(0);
        this->addInjectionControl(InjectorCMode::BHP);

        if (availableForGroupControl)
            this->addInjectionControl(InjectorCMode::GRUP);
        else
            this->dropInjectionControl(InjectorCMode::GRUP);
        {
            const std::string& cmodeString = record.getItem("CMODE").getTrimmedString(0);
            InjectorCMode controlModeArg = InjectorCModeFromString( cmodeString );
            if (this->hasInjectionControl( controlModeArg))
                this->controlMode = controlModeArg;
            else {
                throw std::invalid_argument("Tried to set invalid control: " + cmodeString + " for well: " + well_name);
            }
        }
    }



    void Well::WellInjectionProperties::handleWELTARG(WELTARGCMode cmode, const UDAValue& new_arg, double SiFactorP) {
        if (cmode == Well::WELTARGCMode::BHP){
            if (this->predictionMode) {
                this->BHPTarget.update_value( new_arg );
            } else
                this->bhp_hist_limit = new_arg.get<double>() * SiFactorP;
        }
        else if (cmode == WELTARGCMode::ORAT){
            if (this->injectorType == InjectorType::OIL)
                this->surfaceInjectionRate.update_value( new_arg );
            else
                std::invalid_argument("Well type must be OIL to set the oil rate");
        }
        else if (cmode == WELTARGCMode::WRAT){
            if (this->injectorType == InjectorType::WATER)
                this->surfaceInjectionRate.update_value( new_arg );
            else
                std::invalid_argument("Well type must be WATER to set the water rate");
        }
        else if (cmode == WELTARGCMode::GRAT){
            if(this->injectorType == InjectorType::GAS)
                this->surfaceInjectionRate.update_value( new_arg );
            else
                std::invalid_argument("Well type must be GAS to set the gas rate");
        }
        else if (cmode == WELTARGCMode::THP)
            this->THPTarget.update_value( new_arg );
        else if (cmode == WELTARGCMode::VFP)
            this->VFPTableNumber = static_cast<int>(new_arg.get<double>());
        else if (cmode == WELTARGCMode::RESV)
            this->reservoirInjectionRate.update_value( new_arg );
        else if (cmode != WELTARGCMode::GUID)
            throw std::invalid_argument("Invalid keyword (MODE) supplied");
    }


    void Well::WellInjectionProperties::handleWCONINJH(const DeckRecord& record, bool is_producer, const std::string& well_name) {
        // convert injection rates to SI
        const auto& typeItem = record.getItem("TYPE");
        if (typeItem.defaultApplied(0)) {
            const std::string msg = "Injection type can not be defaulted for keyword WCONINJH";
            throw std::invalid_argument(msg);
        }
        this->injectorType = InjectorTypeFromString( typeItem.getTrimmedString(0));

        if (!record.getItem("RATE").defaultApplied(0)) {
            double injectionRate = record.getItem("RATE").get<double>(0);
            this->surfaceInjectionRate = injectionRate;
        }
        if ( record.getItem( "BHP" ).hasValue(0) )
            this->BHPH = record.getItem("BHP").getSIDouble(0);
        if ( record.getItem( "THP" ).hasValue(0) )
            this->THPH = record.getItem("THP").getSIDouble(0);

        const std::string& cmodeString = record.getItem("CMODE").getTrimmedString(0);
        const InjectorCMode newControlMode = InjectorCModeFromString( cmodeString );

        if ( !(newControlMode == InjectorCMode::RATE || newControlMode == InjectorCMode::BHP) ) {
            const std::string msg = "Only RATE and BHP control are allowed for WCONINJH for well " + well_name;
            throw std::invalid_argument(msg);
        }

        // when well is under BHP control, we use its historical BHP value as BHP limit
        if (newControlMode == InjectorCMode::BHP) {
            this->bhp_hist_limit = this->BHPH;
        } else {
            const bool switching_from_producer = is_producer;
            const bool switching_from_prediction = this->predictionMode;
            const bool switching_from_BHP_control = (this->controlMode == InjectorCMode::BHP);
            if (switching_from_prediction ||
                switching_from_BHP_control ||
                switching_from_producer) {
                this->resetDefaultHistoricalBHPLimit();
            }
            // otherwise, we keep its previous BHP limit
        }

        this->addInjectionControl(InjectorCMode::BHP);
        this->addInjectionControl(newControlMode);
        this->controlMode = newControlMode;
        this->predictionMode = false;

        const int VFPTableNumberArg = record.getItem("VFP_TABLE").get< int >(0);
        if (VFPTableNumberArg > 0) {
            this->VFPTableNumber = VFPTableNumberArg;
        }
    }

    bool Well::WellInjectionProperties::operator==(const Well::WellInjectionProperties& other) const {
        if ((surfaceInjectionRate == other.surfaceInjectionRate) &&
            (reservoirInjectionRate == other.reservoirInjectionRate) &&
            (temperature == other.temperature) &&
            (BHPTarget == other.BHPTarget) &&
            (THPTarget == other.THPTarget) &&
            (BHPH == other.BHPH) &&
            (THPH == other.THPH) &&
            (bhp_hist_limit == other.bhp_hist_limit) &&
            (thp_hist_limit == other.thp_hist_limit) &&
            (VFPTableNumber == other.VFPTableNumber) &&
            (predictionMode == other.predictionMode) &&
            (injectionControls == other.injectionControls) &&
            (injectorType == other.injectorType) &&
            (controlMode == other.controlMode))
            return true;
        else
            return false;
    }

    bool Well::WellInjectionProperties::operator!=(const Well::WellInjectionProperties& other) const {
        return !(*this == other);
    }

    void Well::WellInjectionProperties::resetDefaultHistoricalBHPLimit() {
        // this default BHP value is from simulation result,
        // without finding any related document
        this->bhp_hist_limit = 6891.2 * unit::barsa;
    }

    void Well::WellInjectionProperties::resetBHPLimit() {
        this->bhp_hist_limit = 0;
    }


    std::ostream& operator<<( std::ostream& stream,
                              const Well::WellInjectionProperties& wp ) {
        return stream
            << "Well::WellInjectionProperties { "
            << "surfacerate: "      << wp.surfaceInjectionRate << ", "
            << "reservoir rate "    << wp.reservoirInjectionRate << ", "
            << "temperature: "      << wp.temperature << ", "
            << "BHP target: "       << wp.BHPTarget << ", "
            << "THP target: "       << wp.THPTarget << ", "
            << "BHPH: "             << wp.BHPH << ", "
            << "THPH: "             << wp.THPH << ", "
            << "VFP table: "        << wp.VFPTableNumber << ", "
            << "prediction mode: "  << wp.predictionMode << ", "
            << "injection ctrl: "   << wp.injectionControls << ", "
            << "injector type: "    << InjectorType2String(wp.injectorType) << ", "
            << "control mode: "     << Well::InjectorCMode2String(wp.controlMode) << " }";
    }


     Well::InjectionControls Well::WellInjectionProperties::controls(const UnitSystem& unit_sys, const SummaryState& st, double udq_def) const {
        InjectionControls controls(this->injectionControls);

        controls.surface_rate = UDA::eval_well_uda_rate(this->surfaceInjectionRate, this->name, st, udq_def, this->injectorType, unit_sys);
        controls.reservoir_rate = UDA::eval_well_uda(this->reservoirInjectionRate, this->name, st, udq_def);
        if (this->predictionMode) {
            controls.bhp_limit = UDA::eval_well_uda(this->BHPTarget, this->name, st, udq_def);
            controls.thp_limit = UDA::eval_well_uda(this->THPTarget, this->name, st, udq_def);
        } else {
            controls.bhp_limit = this->bhp_hist_limit;
            controls.thp_limit = this->thp_hist_limit;
        }
        controls.temperature = this->temperature;
        controls.injector_type = this->injectorType;
        controls.cmode = this->controlMode;
        controls.vfp_table_number = this->VFPTableNumber;
        controls.injector_type = this->injectorType;

        return controls;
    }

    bool Well::WellInjectionProperties::updateUDQActive(const UDQConfig& udq_config, UDQActive& active) const {
        int update_count = 0;

        update_count += active.update(udq_config, this->surfaceInjectionRate, this->name, UDAControl::WCONINJE_RATE);
        update_count += active.update(udq_config, this->reservoirInjectionRate, this->name, UDAControl::WCONINJE_RESV);
        update_count += active.update(udq_config, this->BHPTarget, this->name, UDAControl::WCONINJE_BHP);
        update_count += active.update(udq_config, this->THPTarget, this->name, UDAControl::WCONINJE_THP);

        return (update_count > 0);
    }

}
