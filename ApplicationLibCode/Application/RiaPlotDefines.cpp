/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiaPlotDefines.h"

#include "RiaGuiApplication.h"
#include "RiaPreferencesSystem.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::PlotAxis>::setUp()
{
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, "PLOT_AXIS_LEFT", "Left" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT, "PLOT_AXIS_RIGHT", "Right" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, "PLOT_AXIS_BOTTOM", "Bottom" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_TOP, "PLOT_AXIS_TOP", "Top" );

    setDefault( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
}

template <>
void caf::AppEnum<RiaDefines::MultiPlotAxisVisibility>::setUp()
{
    addItem( RiaDefines::MultiPlotAxisVisibility::ONE_VISIBLE, "ONE_VISIBLE", "One Axis Per Plot" );
    addItem( RiaDefines::MultiPlotAxisVisibility::ALL_VISIBLE, "ALL_VISIBLE", "All Axis Visible" );

    setDefault( RiaDefines::MultiPlotAxisVisibility::ONE_VISIBLE );
}

template <>
void caf::AppEnum<RiaDefines::ObjectNamingMethod>::setUp()
{
    addItem( RiaDefines::ObjectNamingMethod::CUSTOM, "CUSTOM", "Custom" );
    addItem( RiaDefines::ObjectNamingMethod::AUTO, "AUTO", "Auto" );
    addItem( RiaDefines::ObjectNamingMethod::TEMPLATE, "TEMPLATE", "Template" );

    setDefault( RiaDefines::ObjectNamingMethod::AUTO );
}

template <>
void caf::AppEnum<Qt::PenStyle>::setUp()
{
    addItem( Qt::PenStyle::NoPen, "NO_PEN", "No Pen" );
    addItem( Qt::PenStyle::SolidLine, "SOLID_LINE", "Solid Line" );
    addItem( Qt::PenStyle::DashLine, "DASH_LINE", "Dash Line" );
    addItem( Qt::PenStyle::DotLine, "DOT_LINE", "Dot Line" );
    addItem( Qt::PenStyle::DashDotLine, "DASH_DOT_LINE", "Dash Dot Line" );
    addItem( Qt::PenStyle::DashDotDotLine, "DASH_DOT_DOT_LINE", "Dash Dot Dot Line" );

    setDefault( Qt::PenStyle::SolidLine );
}

template <>
void caf::AppEnum<RiaDefines::Orientation>::setUp()
{
    addItem( RiaDefines::Orientation::HORIZONTAL, "HORIZONTAL", "Horizontal" );
    addItem( RiaDefines::Orientation::VERTICAL, "VERTICAL", "Vertical" );
    setDefault( RiaDefines::Orientation::VERTICAL );
}

template <>
void caf::AppEnum<RiaDefines::TextAlignment>::setUp()
{
    addItem( RiaDefines::TextAlignment::LEFT, "LEFT", "Left" );
    addItem( RiaDefines::TextAlignment::CENTER, "CENTER", "Center" );
    addItem( RiaDefines::TextAlignment::RIGHT, "RIGHT", "Right" );
    setDefault( RiaDefines::TextAlignment::RIGHT );
}

template <>
void caf::AppEnum<RiaDefines::WindowTileMode>::setUp()
{
    addItem( RiaDefines::WindowTileMode::DEFAULT, "DEFAULT", "Default" );
    addItem( RiaDefines::WindowTileMode::VERTICAL, "VERTICAL", "Vertical" );
    addItem( RiaDefines::WindowTileMode::HORIZONTAL, "HORIZONTAL", "Horizontal" );
    addItem( RiaDefines::WindowTileMode::UNDEFINED, "UNDEFINED", "Undefined" );

    setDefault( RiaDefines::WindowTileMode::UNDEFINED );
}

template <>
void caf::AppEnum<RiaDefines::ReadOutType>::setUp()
{
    addItem( RiaDefines::ReadOutType::NONE, "NONE", "None" );
    addItem( RiaDefines::ReadOutType::SNAP_TO_POINT, "SNAP_TO_POINT", "Snap to Closest Curve Point" );
    addItem( RiaDefines::ReadOutType::TIME_TRACKING, "TIME_TRACKING", "Time Tracking" );
    addItem( RiaDefines::ReadOutType::TIME_VALUE_TRACKING, "TIME_VALUE_TRACKING", "Time and Value Tracking" );

    setDefault( RiaDefines::ReadOutType::SNAP_TO_POINT );
}

}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableCase()
{
    return "$CASE";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableWell()
{
    return "$WELL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableRefWell()
{
    return "$REF_WELL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableWellBranch()
{
    return "$WELL_BRANCH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableResultName()
{
    return "$RESULT_NAME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableResultType()
{
    return "$RESULT_TYPE";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableTime()
{
    return "$TIME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableTimestep()
{
    return "$TIME_STEP";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableAirGap()
{
    return "$AIR_GAP";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::namingVariableWaterDepth()
{
    return "$WATER_DEPTH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::selectionTextNone()
{
    return "None";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultValuePlot()
{
    return -10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultLogValuePlot()
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::maximumDefaultValuePlot()
{
    return 100.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isHorizontal( RiaDefines::PlotAxis axis )
{
    return !isVertical( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isVertical( RiaDefines::PlotAxis axis )
{
    return ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || axis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RiaDefines::opposite( PlotAxis axis )
{
    switch ( axis )
    {
        case RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM:
            return RiaDefines::PlotAxis::PLOT_AXIS_TOP;
        case RiaDefines::PlotAxis::PLOT_AXIS_TOP:
            return RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
        case RiaDefines::PlotAxis::PLOT_AXIS_LEFT:
            return RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
        case RiaDefines::PlotAxis::PLOT_AXIS_RIGHT:
            return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    }

    // Should never come here
    CVF_ASSERT( false );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::scalingFactor( QPaintDevice* paintDevice )
{
    auto scalingFactor = RiaPreferencesSystem::current()->exportPdfScalingFactor();

    if ( scalingFactor > 0.0 ) return scalingFactor;

    if ( !paintDevice ) return 1.0;

    int    resolution = paintDevice->logicalDpiX();
    double scaling    = resolution / static_cast<double>( RiaGuiApplication::applicationResolution() );

    return scaling;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::curveNameGroupName()
{
    return "Curve Name";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::appearanceGroupName()
{
    return "Appearance";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::additionalDataSourcesGroupName()
{
    return "Additional Data Sources";
}
