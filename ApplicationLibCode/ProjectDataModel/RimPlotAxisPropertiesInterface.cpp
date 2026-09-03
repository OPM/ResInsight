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

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotAxisPropertiesInterface, "PlotAxisPropertiesInterface", "RimPlotAxisPropertiesInterface" );

namespace caf
{
template <>
void caf::AppEnum<RimPlotAxisPropertiesInterface::AxisTitlePositionType>::setUp()
{
    addItem( RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center" );
    addItem( RimPlotAxisPropertiesInterface::AXIS_TITLE_END, "AXIS_TITLE_END", "At End" );

    setDefault( RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER );
}

template <>
void RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum::setUp()
{
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_VERY_FEW, "VERY_FEW", "Very Few" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW, "Few", "Few" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT, "Default", "Default" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_MANY, "Many", "Many" );
    setDefault( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface::RimPlotAxisPropertiesInterface()
    : settingsChanged( this )
{
    CAF_PDM_InitObject( "Plot Axis Properties Interface" );

    CAF_PDM_InitField( &m_isRangeUserDefined, "IsRangeUserDefined", false, "Is Range Defined by User" );
    m_isRangeUserDefined.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
/// A user defined axis range is kept when the data source of a plot is changed, and is released when the user asks for
/// automatic range computation (Zoom All or "Set Range Automatically").
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isRangeUserDefined() const
{
    return m_isRangeUserDefined();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisPropertiesInterface::setRangeUserDefined( bool isUserDefined )
{
    m_isRangeUserDefined = isUserDefined;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isAxisInverted() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isLogarithmicScaleEnabled() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( LegendTickmarkCount count )
{
    int maxTickmarkCount = 8;

    switch ( count )
    {
        case LegendTickmarkCount::TICKMARK_VERY_FEW:
            maxTickmarkCount = 2;
            break;
        case LegendTickmarkCount::TICKMARK_FEW:
            maxTickmarkCount = 4;
            break;
        case LegendTickmarkCount::TICKMARK_DEFAULT:
            maxTickmarkCount = 8; // Taken from QwtPlot::initAxesData()
            break;
        case LegendTickmarkCount::TICKMARK_MANY:
            maxTickmarkCount = 10;
            break;
        default:
            break;
    }

    return maxTickmarkCount;
}
