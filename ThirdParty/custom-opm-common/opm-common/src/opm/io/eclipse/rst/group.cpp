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

double sgrp_value(float raw_value) {
    const auto infty = 1.0e+20f;
    if (std::abs(raw_value) == infty)
        return 0;
    else
        return raw_value;
}

RstGroup::RstGroup(const ::Opm::UnitSystem& unit_system,
                   const RstHeader& header,
                   const std::string* zwel,
                   const int * igrp,
                   const float * sgrp,
                   const double * xgrp) :
    name(trim_copy(zwel[0])),
    parent_group(igrp[header.nwgmax + VI::IGroup::ParentGroup] ),
    // prod_active_cmode(igrp[header.nwgmax + VI::IGroup::ProdActiveCMode]),
    prod_cmode(igrp[header.nwgmax + VI::IGroup::GConProdCMode]),
    winj_cmode(igrp[header.nwgmax + VI::IGroup::WInjCMode]),
    ginj_cmode(igrp[header.nwgmax + VI::IGroup::GInjCMode]),
    guide_rate_def(igrp[header.nwgmax + VI::IGroup::GuideRateDef]),
    // The values oil_rate_limit -> gas_voidage_limit will be used in UDA
    // values. The UDA values are responsible for unit conversion and raw values
    // are internalized here.
    oil_rate_limit(                sgrp_value(sgrp[VI::SGroup::OilRateLimit])),
    water_rate_limit(              sgrp_value(sgrp[VI::SGroup::WatRateLimit])),
    gas_rate_limit(                sgrp_value(sgrp[VI::SGroup::GasRateLimit])),
    liquid_rate_limit(             sgrp_value(sgrp[VI::SGroup::LiqRateLimit])),
    water_surface_limit(           sgrp_value(sgrp[VI::SGroup::waterSurfRateLimit])),
    water_reservoir_limit(         sgrp_value(sgrp[VI::SGroup::waterResRateLimit])),
    water_reinject_limit(          sgrp_value(sgrp[VI::SGroup::waterReinjectionLimit])),
    water_voidage_limit(           sgrp_value(sgrp[VI::SGroup::waterVoidageLimit])),
    gas_surface_limit(             sgrp_value(sgrp[VI::SGroup::gasSurfRateLimit])),
    gas_reservoir_limit(           sgrp_value(sgrp[VI::SGroup::gasResRateLimit])),
    gas_reinject_limit(            sgrp_value(sgrp[VI::SGroup::gasReinjectionLimit])),
    gas_voidage_limit(             sgrp_value(sgrp[VI::SGroup::gasVoidageLimit])),
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
