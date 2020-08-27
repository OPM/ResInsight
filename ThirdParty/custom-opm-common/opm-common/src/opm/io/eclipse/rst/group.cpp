/*
  Copyright 2020 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/common/utility/String.hpp>

#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/group.hpp>

#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>


namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm {
namespace RestartIO {

using M  = ::Opm::UnitSystem::measure;

RstGroup::RstGroup(const ::Opm::UnitSystem& unit_system,
                   const std::string* zwel,
                   const int *,
                   const float * sgrp,
                   const double * xgrp) :
    name(trim_copy(zwel[0])),
    oil_rate_limit(                unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::OilRateLimit])),
    water_rate_limit(              unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::WatRateLimit])),
    gas_rate_limit(                unit_system.to_si(M::gas_surface_rate,      sgrp[VI::SGroup::GasRateLimit])),
    liquid_rate_limit(             unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::LiqRateLimit])),
    water_surface_limit(           unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::waterSurfRateLimit])),
    water_reservoir_limit(         unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::waterResRateLimit])),
    water_reinject_limit(          unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::waterReinjectionLimit])),
    water_voidage_limit(           unit_system.to_si(M::liquid_surface_rate,   sgrp[VI::SGroup::waterVoidageLimit])),
    gas_surface_limit(             unit_system.to_si(M::gas_surface_rate,      sgrp[VI::SGroup::gasSurfRateLimit])),
    gas_reservoir_limit(           unit_system.to_si(M::geometric_volume_rate, sgrp[VI::SGroup::gasResRateLimit])),
    gas_reinject_limit(            unit_system.to_si(M::gas_surface_rate,      sgrp[VI::SGroup::gasReinjectionLimit])),
    gas_voidage_limit(             unit_system.to_si(M::geometric_volume_rate, sgrp[VI::SGroup::gasVoidageLimit])),
    oil_production_rate(           unit_system.to_si(M::liquid_surface_rate,   xgrp[VI::XGroup::OilPrRate])),
    water_production_rate(         unit_system.to_si(M::liquid_surface_rate,   xgrp[VI::XGroup::WatPrRate])),
    gas_production_rate(           unit_system.to_si(M::gas_surface_rate,      xgrp[VI::XGroup::GasPrRate])),
    liquid_production_rate(        unit_system.to_si(M::liquid_surface_rate,   xgrp[VI::XGroup::LiqPrRate])),
    water_injection_rate(          unit_system.to_si(M::liquid_surface_rate,   xgrp[VI::XGroup::WatInjRate])),
    gas_injection_rate(            unit_system.to_si(M::gas_surface_rate,      xgrp[VI::XGroup::GasInjRate])),
    wct(                           unit_system.to_si(M::water_cut,             xgrp[VI::XGroup::WatCut])),
    gor(                           unit_system.to_si(M::gas_oil_ratio,         xgrp[VI::XGroup::GORatio])),
    oil_production_total(          unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::OilPrTotal])),
    water_production_total(        unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::WatPrTotal])),
    gas_production_total(          unit_system.to_si(M::gas_surface_volume,    xgrp[VI::XGroup::GasPrTotal])),
    voidage_production_total(      unit_system.to_si(M::geometric_volume,      xgrp[VI::XGroup::VoidPrTotal])),
    water_injection_total(         unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::WatInjTotal])),
    gas_injection_total(           unit_system.to_si(M::gas_surface_volume,    xgrp[VI::XGroup::GasInjTotal])),
    voidage_injection_total(       unit_system.to_si(M::volume,                xgrp[VI::XGroup::VoidInjTotal])),
    oil_production_potential(      unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::OilPrPot])),
    water_production_potential(    unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::WatPrPot])),
    history_total_oil_production(  unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::HistOilPrTotal])),
    history_total_water_production(unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::HistWatPrTotal])),
    history_total_water_injection( unit_system.to_si(M::liquid_surface_volume, xgrp[VI::XGroup::HistWatInjTotal])),
    history_total_gas_production(  unit_system.to_si(M::gas_surface_volume,    xgrp[VI::XGroup::HistGasPrTotal])),
    history_total_gas_injection(   unit_system.to_si(M::gas_surface_volume,    xgrp[VI::XGroup::HistGasInjTotal]))
{
}


}
}
