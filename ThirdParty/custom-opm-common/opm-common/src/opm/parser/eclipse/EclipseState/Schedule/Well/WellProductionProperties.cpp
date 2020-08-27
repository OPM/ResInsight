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

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellProductionProperties.hpp>

#include "../eval_uda.hpp"

namespace Opm {

    Well::WellProductionProperties::WellProductionProperties() :
        WellProductionProperties(UnitSystem(UnitSystem::UnitType::UNIT_TYPE_METRIC), "")
    {}

    Well::WellProductionProperties::WellProductionProperties(const UnitSystem& units, const std::string& name_arg) :
        name(name_arg),
        OilRate(units.getDimension(UnitSystem::measure::liquid_surface_rate)),
        WaterRate(units.getDimension(UnitSystem::measure::liquid_surface_rate)),
        GasRate(units.getDimension(UnitSystem::measure::gas_surface_rate)),
        LiquidRate(units.getDimension(UnitSystem::measure::liquid_surface_rate)),
        ResVRate(units.getDimension(UnitSystem::measure::rate)),
        BHPTarget(units.getDimension(UnitSystem::measure::pressure)),
        THPTarget(units.getDimension(UnitSystem::measure::pressure)),
        BHPH(0.0),
        THPH(0.0),
        VFPTableNumber(0),
        ALQValue(0.0),
        predictionMode(true),
        controlMode(ProducerCMode::CMODE_UNDEFINED),
        whistctl_cmode(ProducerCMode::CMODE_UNDEFINED),
        m_productionControls(0)
    {}


    Well::WellProductionProperties Well::WellProductionProperties::serializeObject()
    {
        Well::WellProductionProperties result;
        result.name = "test";
        result.OilRate = UDAValue(1.0);
        result.WaterRate = UDAValue("test");
        result.GasRate = UDAValue(2.0);
        result.LiquidRate = UDAValue(3.0);
        result.ResVRate = UDAValue(4.0);
        result.BHPTarget = UDAValue(5.0);
        result.THPTarget = UDAValue(6.0);
        result.bhp_hist_limit = 7.0;
        result.thp_hist_limit = 8.0;
        result.BHPH = 9.0;
        result.THPH = 10.0;
        result.VFPTableNumber = 11;
        result.ALQValue = 12.0;
        result.predictionMode = true;
        result.controlMode = ProducerCMode::CRAT;
        result.whistctl_cmode = ProducerCMode::BHP;
        result.m_productionControls = 13;

        return result;
    }

    void Well::WellProductionProperties::init_rates( const DeckRecord& record ) {
        this->OilRate    = record.getItem("ORAT").get<UDAValue>(0);
        this->WaterRate  = record.getItem("WRAT").get<UDAValue>(0);
        this->GasRate    = record.getItem("GRAT").get<UDAValue>(0);
    }


    void Well::WellProductionProperties::init_history(const DeckRecord& record)
    {
        this->predictionMode = false;
        // update LiquidRate. The funnny constrction with explicitly making a new
        // UDAValue is to ensure that the UDAValue has the correct dimension.
        this->LiquidRate = UDAValue(this->WaterRate.get<double>() + this->OilRate.get<double>(), this->OilRate.get_dim());

        if ( record.getItem( "BHP" ).hasValue(0) )
            this->BHPH = record.getItem("BHP").get<UDAValue>(0).getSI();

        if ( record.getItem( "THP" ).hasValue(0) )
            this->THPH = record.getItem("THP").get<UDAValue>(0).getSI();

        const auto& cmodeItem = record.getItem("CMODE");
        if ( cmodeItem.defaultApplied(0) ) {
            const std::string msg = "control mode can not be defaulted for keyword WCONHIST";
            throw std::invalid_argument(msg);
        }

        ProducerCMode cmode;

        if (effectiveHistoryProductionControl(this->whistctl_cmode) )
            cmode = this->whistctl_cmode;
        else
            cmode = ProducerCModeFromString( cmodeItem.getTrimmedString( 0 ) );

        // clearing the existing targets/limits
        clearControls();

        if (effectiveHistoryProductionControl(cmode)) {
            this->addProductionControl( cmode );
            this->controlMode = cmode;
        } else {
            const std::string cmode_string = cmodeItem.getTrimmedString( 0 );
            const std::string msg = "unsupported control mode " + cmode_string + " for WCONHIST";
            throw std::invalid_argument(msg);
        }

        // always have a BHP control/limit, while the limit value needs to be determined
        // the control mode added above can be a BHP control or a type of RATE control
        if ( !this->hasProductionControl( ProducerCMode::BHP ) )
            this->addProductionControl( ProducerCMode::BHP );

        if (cmode == ProducerCMode::BHP)
            this->setBHPLimit(this->BHPH);

        const auto vfp_table = record.getItem("VFP_TABLE").get< int >(0);
        if (vfp_table != 0)
            this->VFPTableNumber = vfp_table;

        auto alq_value = record.getItem("LIFT").get<double>(0); //NOTE: Unit of ALQ is never touched
        if (alq_value != 0.)
            this->ALQValue = alq_value;
    }



    void Well::WellProductionProperties::handleWCONPROD( const std::string& /* well */, const DeckRecord& record)
    {
        this->predictionMode = true;

        this->BHPTarget      = record.getItem("BHP").get<UDAValue>(0);
        this->THPTarget      = record.getItem("THP").get<UDAValue>(0);
        this->ALQValue       = record.getItem("ALQ").get< double >(0); //NOTE: Unit of ALQ is never touched
        this->VFPTableNumber = record.getItem("VFP_TABLE").get< int >(0);
        this->LiquidRate     = record.getItem("LRAT").get<UDAValue>(0);
        this->ResVRate       = record.getItem("RESV").get<UDAValue>(0);

        using mode = std::pair< const std::string, ProducerCMode >;
        static const mode modes[] = {
            { "ORAT", ProducerCMode::ORAT }, { "WRAT", ProducerCMode::WRAT }, { "GRAT", ProducerCMode::GRAT },
            { "LRAT", ProducerCMode::LRAT }, { "RESV", ProducerCMode::RESV }, { "THP", ProducerCMode::THP }
        };


        this->init_rates(record);

        for( const auto& cmode : modes ) {
            if( !record.getItem( cmode.first ).defaultApplied( 0 ) ) {

                // a zero value THP limit will not be handled as a THP limit
                if (cmode.first == "THP" && this->THPTarget.zero())
                    continue;

                this->addProductionControl( cmode.second );
            }
        }

        // There is always a BHP constraint, when not specified, will use the default value
        this->addProductionControl( ProducerCMode::BHP );
        {
            const auto& cmodeItem = record.getItem("CMODE");
            if (cmodeItem.hasValue(0)) {
                auto cmode = Well::ProducerCModeFromString( cmodeItem.getTrimmedString(0) );

                if (this->hasProductionControl( cmode ))
                    this->controlMode = cmode;
                else
                    throw std::invalid_argument("Trying to set CMODE to: " + cmodeItem.getTrimmedString(0) + " - no value has been specified for this control");
            }
        }
    }

    /*
      This is now purely "history" constructor - i.e. the record should
      originate from the WCONHIST keyword. Predictions are handled with the
      default constructor and the handleWCONPROD() method.
    */
    void Well::WellProductionProperties::handleWCONHIST(const DeckRecord& record)
    {
        this->init_rates(record);
        this->LiquidRate = 0;
        this->ResVRate = 0;

        // when the well is switching to history matching producer from prediction mode
        // or switching from injector to producer
        // or switching from BHP control to RATE control (under history matching mode)
        // we use the defaulted BHP limit, otherwise, we use the previous BHP limit
        if (this->predictionMode)
            this->resetDefaultBHPLimit();

        if (this->controlMode == ProducerCMode::BHP)
            this->resetDefaultBHPLimit();

        this->init_history(record);
    }



    void Well::WellProductionProperties::handleWELTARG(Well::WELTARGCMode cmode, const UDAValue& new_arg, double SiFactorP) {
        if (cmode == WELTARGCMode::ORAT){
            this->OilRate.update_value( new_arg );
            this->addProductionControl( ProducerCMode::ORAT );
        }
        else if (cmode == WELTARGCMode::WRAT){
            this->WaterRate.update_value( new_arg );
            this->addProductionControl( ProducerCMode::WRAT );
        }
        else if (cmode == WELTARGCMode::GRAT){
            this->GasRate.update_value( new_arg );
            this->addProductionControl( ProducerCMode::GRAT );
        }
        else if (cmode == WELTARGCMode::LRAT){
            this->LiquidRate.update_value( new_arg );
            this->addProductionControl( ProducerCMode::LRAT );
        }
        else if (cmode == WELTARGCMode::RESV){
            this->ResVRate.update_value( new_arg );
            this->addProductionControl( ProducerCMode::RESV );
        }
        else if (cmode == WELTARGCMode::BHP){
            if (this->predictionMode)
                this->BHPTarget.update_value( new_arg );
            else
                this->bhp_hist_limit = new_arg.get<double>() * SiFactorP;
            this->addProductionControl( ProducerCMode::BHP );
        }
        else if (cmode == WELTARGCMode::THP){
            this->THPTarget.update_value( new_arg );
            this->addProductionControl( ProducerCMode::THP );
        }
        else if (cmode == WELTARGCMode::VFP)
            this->VFPTableNumber = static_cast<int>(new_arg.get<double>());
        else if (cmode != WELTARGCMode::GUID)
            throw std::invalid_argument("Invalid keyword (MODE) supplied");
    }



    bool Well::WellProductionProperties::operator==(const Well::WellProductionProperties& other) const {
        return OilRate              == other.OilRate
            && WaterRate            == other.WaterRate
            && GasRate              == other.GasRate
            && LiquidRate           == other.LiquidRate
            && ResVRate             == other.ResVRate
            && BHPTarget            == other.BHPTarget
            && THPTarget            == other.THPTarget
            && bhp_hist_limit       == other.bhp_hist_limit
            && thp_hist_limit       == other.thp_hist_limit
            && BHPH                 == other.BHPH
            && THPH                 == other.THPH
            && VFPTableNumber       == other.VFPTableNumber
            && controlMode          == other.controlMode
            && m_productionControls == other.m_productionControls
            && whistctl_cmode       == other.whistctl_cmode
            && this->predictionMode == other.predictionMode;
    }


    bool Well::WellProductionProperties::operator!=(const Well::WellProductionProperties& other) const {
        return !(*this == other);
    }


    std::ostream& operator<<( std::ostream& stream, const Well::WellProductionProperties& wp ) {
        return stream
            << "Well::WellProductionProperties { "
            << "oil rate: "     << wp.OilRate           << ", "
            << "water rate: "   << wp.WaterRate         << ", "
            << "gas rate: "     << wp.GasRate           << ", "
            << "liquid rate: "  << wp.LiquidRate        << ", "
            << "ResV rate: "    << wp.ResVRate          << ", "
            << "BHP target: "   << wp.BHPTarget         << ", "
            << "THP target: "   << wp.THPTarget         << ", "
            << "BHPH: "         << wp.BHPH              << ", "
            << "THPH: "         << wp.THPH              << ", "
            << "VFP table: "    << wp.VFPTableNumber    << ", "
            << "ALQ: "          << wp.ALQValue          << ", "
            << "WHISTCTL: "     << Well::ProducerCMode2String(wp.whistctl_cmode)    << ", "
            << "prediction: "   << wp.predictionMode    << " }";
    }

    bool Well::WellProductionProperties::effectiveHistoryProductionControl(const Well::ProducerCMode cmode) {
        // Note, not handling CRAT for now
        return ( (cmode == ProducerCMode::LRAT || cmode == ProducerCMode::RESV || cmode == ProducerCMode::ORAT ||
                  cmode == ProducerCMode::WRAT || cmode == ProducerCMode::GRAT || cmode == ProducerCMode::BHP) );
    }

    void Well::WellProductionProperties::resetDefaultBHPLimit() {
        double si_value = 1. * unit::atm;
        this->setBHPLimit(si_value);
    }

    void Well::WellProductionProperties::clearControls() {
        m_productionControls = 0;
    }

    void Well::WellProductionProperties::setBHPLimit(const double si_limit) {
        this->bhp_hist_limit = si_limit;
    }


    Well::ProductionControls Well::WellProductionProperties::controls(const SummaryState& st, double udq_undef) const {
        Well::ProductionControls controls(this->m_productionControls);

        controls.oil_rate = UDA::eval_well_uda(this->OilRate, this->name, st, udq_undef);
        controls.water_rate = UDA::eval_well_uda(this->WaterRate, this->name, st, udq_undef);
        controls.gas_rate = UDA::eval_well_uda(this->GasRate, this->name, st, udq_undef);
        controls.liquid_rate = UDA::eval_well_uda(this->LiquidRate, this->name, st, udq_undef);
        controls.resv_rate = UDA::eval_well_uda(this->ResVRate, this->name, st, udq_undef);

        if (this->predictionMode) {
            controls.bhp_limit = UDA::eval_well_uda(this->BHPTarget, this->name, st, udq_undef);
            controls.thp_limit = UDA::eval_well_uda(this->THPTarget, this->name, st, udq_undef);
        } else {
            controls.bhp_limit = this->bhp_hist_limit;
            controls.thp_limit = this->thp_hist_limit;
        }

        controls.bhp_history = this->BHPH;
        controls.thp_history = this->THPH;
        controls.vfp_table_number = this->VFPTableNumber;
        controls.alq_value = this->ALQValue;
        controls.cmode = this->controlMode;

        return controls;
    }

    bool Well::WellProductionProperties::updateUDQActive(const UDQConfig& udq_config, UDQActive& active) const {
        int update_count = 0;

        update_count += active.update(udq_config, this->OilRate, this->name, UDAControl::WCONPROD_ORAT);
        update_count += active.update(udq_config, this->WaterRate, this->name, UDAControl::WCONPROD_WRAT);
        update_count += active.update(udq_config, this->GasRate, this->name, UDAControl::WCONPROD_GRAT);
        update_count += active.update(udq_config, this->LiquidRate, this->name, UDAControl::WCONPROD_LRAT);
        update_count += active.update(udq_config, this->ResVRate, this->name, UDAControl::WCONPROD_RESV);
        update_count += active.update(udq_config, this->BHPTarget, this->name, UDAControl::WCONPROD_BHP);
        update_count += active.update(udq_config, this->THPTarget, this->name, UDAControl::WCONPROD_THP);

        return (update_count > 0);
    }

} // namespace Opm
