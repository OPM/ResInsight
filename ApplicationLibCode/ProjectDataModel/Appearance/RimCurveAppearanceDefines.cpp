/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimCurveAppearanceDefines.h"
#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RimCurveAppearanceDefines::ParameterSorting>::setUp()
{
    addItem( RimCurveAppearanceDefines::ParameterSorting::ABSOLUTE_VALUE, "ABSOLUTE_VALUE", "Absolute Correlation" );
    addItem( RimCurveAppearanceDefines::ParameterSorting::ALPHABETICALLY, "ALPHABETICALLY", "Alphabetically" );
    setDefault( RimCurveAppearanceDefines::ParameterSorting::ABSOLUTE_VALUE );
}
template <>
void AppEnum<RimCurveAppearanceDefines::AppearanceMode>::setUp()
{
    addItem( RimCurveAppearanceDefines::AppearanceMode::DEFAULT, "DEFAULT", "Default" );
    addItem( RimCurveAppearanceDefines::AppearanceMode::CUSTOM, "CUSTOM", "Custom" );
    setDefault( RimCurveAppearanceDefines::AppearanceMode::DEFAULT );
}
} // namespace caf
