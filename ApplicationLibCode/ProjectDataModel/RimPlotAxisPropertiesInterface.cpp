/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RimPlotAxisPropertiesInterface.h"

#include "cafAppEnum.h"

// clang-format off
namespace caf
{
template<>
void caf::AppEnum<RimPlotAxisPropertiesInterface::AxisTitlePositionType>::setUp()
{
    addItem(RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center");
    addItem(RimPlotAxisPropertiesInterface::AXIS_TITLE_END, "AXIS_TITLE_END", "At End");

    setDefault(RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER);
}

template <>
void RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum::setUp()
{
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW, "Few", "Few" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT, "Default", "Default" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_MANY, "Many", "Many" );
    setDefault( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT );
}
} // namespace caf
