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

#pragma once

#include "RiaEclipseUnitTools.h"

#include <QString>

namespace RiaDefines
{
enum class CurveProperty
{
    UNDEFINED,
    FACIES,
    LAYERS,
    POROSITY,
    PERMEABILITY_X,
    PERMEABILITY_Z,
    INITIAL_PRESSURE,
    PRESSURE,
    STRESS,
    INITIAL_STRESS,
    STRESS_GRADIENT,
    YOUNGS_MODULUS,
    POISSONS_RATIO,
    K_IC,
    PROPPANT_EMBEDMENT,
    BIOT_COEFFICIENT,
    K0,
    FLUID_LOSS_COEFFICIENT,
    SPURT_LOSS,
    TEMPERATURE,
    RELATIVE_PERMEABILITY_FACTOR,
    PORO_ELASTIC_CONSTANT,
    THERMAL_EXPANSION_COEFFICIENT,
    IMMOBILE_FLUID_SATURATION,
    NET_TO_GROSS,
    POROSITY_UNSCALED,
};

double defaultPorosity();
double defaultPermeability();

}; // namespace RiaDefines
