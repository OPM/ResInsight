/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaStimPlanModelDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RiaDefines::CurveProperty>::setUp()
{
    addItem( RiaDefines::CurveProperty::UNDEFINED, "UNDEFINED", "Undefined" );
    addItem( RiaDefines::CurveProperty::FACIES, "FACIES", "Facies" );
    addItem( RiaDefines::CurveProperty::LAYERS, "LAYERS", "Layers" );
    addItem( RiaDefines::CurveProperty::POROSITY, "POROSITY", "Porosity" );
    addItem( RiaDefines::CurveProperty::PERMEABILITY_X, "PERMEABILITY_X", "Permeability Horizontal" );
    addItem( RiaDefines::CurveProperty::PERMEABILITY_Z, "PERMEABILITY_Z", "Permeability Vertical" );
    addItem( RiaDefines::CurveProperty::INITIAL_PRESSURE, "INITIAL_PRESSURE", "Initial Pressure" );
    addItem( RiaDefines::CurveProperty::PRESSURE, "PRESSURE", "Pressure" );
    addItem( RiaDefines::CurveProperty::STRESS, "STRESS", "Stress" );
    addItem( RiaDefines::CurveProperty::INITIAL_STRESS, "INITIAL_STRESS", "Initial Stress" );
    addItem( RiaDefines::CurveProperty::STRESS_GRADIENT, "STRESS_GRADIENT", "Stress Gradient" );
    addItem( RiaDefines::CurveProperty::YOUNGS_MODULUS, "YOUNGS_MODULUS", "Young's Modulus" );
    addItem( RiaDefines::CurveProperty::POISSONS_RATIO, "POISSONS_RATIO", "Poisson's Ratio" );
    addItem( RiaDefines::CurveProperty::K_IC, "K_IC", "K-Ic" );
    addItem( RiaDefines::CurveProperty::PROPPANT_EMBEDMENT, "PROPPANT_EMBEDMENT", "Proppant Embedment" );
    addItem( RiaDefines::CurveProperty::BIOT_COEFFICIENT, "BIOT_COEFFICIENT", "Biot Coefficient" );
    addItem( RiaDefines::CurveProperty::K0, "K0", "k0" );
    addItem( RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT, "FLUID_LOSS_COEFFICIENT", "Fluid Loss Coefficient" );
    addItem( RiaDefines::CurveProperty::SPURT_LOSS, "SPURT_LOSS", "Spurt Loss" );
    addItem( RiaDefines::CurveProperty::TEMPERATURE, "TEMPERATURE", "Temperature" );
    addItem( RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
             "RELATIVE_PERMEABILITY_FACTOR",
             "Relative Permeability Factor" );
    addItem( RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT, "PORO_ELASTIC_CONSTANT", "Poro-Elastic Constant" );
    addItem( RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT,
             "THERMAL_EXPANSION_COEFFICIENT",
             "Thermal Expansion Coefficient" );
    addItem( RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION, "IMMOBILE_FLUID_SATURATION", "Immobile Fluid Saturation" );
    addItem( RiaDefines::CurveProperty::NET_TO_GROSS, "NET_TO_GROSS", "Net-To-Gross" );
    addItem( RiaDefines::CurveProperty::POROSITY_UNSCALED, "POROSITY_UNSCALED", "Porosity (Unscaled)" );

    setDefault( RiaDefines::CurveProperty::UNDEFINED );
}
}; // namespace caf

namespace RiaDefines
{
double defaultPorosity()
{
    return 0.01;
}

double defaultPermeability()
{
    return 1.0e-5;
}

double zeroReplacementForLogarithmicPlot()
{
    return 1.0e-5;
}

}; // namespace RiaDefines
