/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimVfpDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::setUp()
{
    addItem( RimVfpDefines::InterpolatedVariableType::BHP, "BHP", "Bottom Hole Pressure" );
    addItem( RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF, "BHP_THP_DIFF", "BHP-THP" );
    setDefault( RimVfpDefines::InterpolatedVariableType::BHP );
}

template <>
void caf::AppEnum<RimVfpDefines::TableType>::setUp()
{
    addItem( RimVfpDefines::TableType::INJECTION, "INJECTION", "Injection" );
    addItem( RimVfpDefines::TableType::PRODUCTION, "PRODUCTION", "Production" );
    setDefault( RimVfpDefines::TableType::INJECTION );
}

template <>
void caf::AppEnum<RimVfpDefines::ProductionVariableType>::setUp()
{
    addItem( RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE, "LIQUID_FLOW_RATE", "Liquid Flow Rate" );
    addItem( RimVfpDefines::ProductionVariableType::THP, "THP", "THP" );
    addItem( RimVfpDefines::ProductionVariableType::WATER_CUT, "WATER_CUT", "Water Cut" );
    addItem( RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO, "GAS_LIQUID_RATIO", "Gas Liquid Ratio" );
    addItem( RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, "ALQ", "Artificial Lift Quantity" );
    setDefault( RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE );
}

template <>
void caf::AppEnum<RimVfpDefines::FlowingPhaseType>::setUp()
{
    addItem( RimVfpDefines::FlowingPhaseType::OIL, "OIL", "Oil" );
    addItem( RimVfpDefines::FlowingPhaseType::GAS, "GAS", "Gas" );
    addItem( RimVfpDefines::FlowingPhaseType::WATER, "WATER", "Water" );
    addItem( RimVfpDefines::FlowingPhaseType::LIQUID, "LIQUID", "Liquid (Oil and Water)" );
    addItem( RimVfpDefines::FlowingPhaseType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpDefines::FlowingPhaseType::INVALID );
}

template <>
void caf::AppEnum<RimVfpDefines::FlowingWaterFractionType>::setUp()
{
    addItem( RimVfpDefines::FlowingWaterFractionType::WOR, "WOR", "Water-Oil Ratio" );
    addItem( RimVfpDefines::FlowingWaterFractionType::WCT, "WCT", "Water Cut" );
    addItem( RimVfpDefines::FlowingWaterFractionType::WGR, "WGR", "Water-Gas Ratio" );
    addItem( RimVfpDefines::FlowingWaterFractionType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpDefines::FlowingWaterFractionType::INVALID );
}

template <>
void caf::AppEnum<RimVfpDefines::FlowingGasFractionType>::setUp()
{
    addItem( RimVfpDefines::FlowingGasFractionType::GOR, "GOR", "Gas-Oil Ratio" );
    addItem( RimVfpDefines::FlowingGasFractionType::GLR, "GLR", "Gas-Liquid Ratio" );
    addItem( RimVfpDefines::FlowingGasFractionType::OGR, "OGR", "Oil-Gas Ratio" );
    addItem( RimVfpDefines::FlowingGasFractionType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpDefines::FlowingGasFractionType::INVALID );
}
} // namespace caf
