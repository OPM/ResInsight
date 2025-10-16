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

#pragma once

class QPaintDevice;
class QString;

// Defines relate to plotting
namespace RiaDefines
{
enum class PlotAxis
{
    PLOT_AXIS_LEFT,
    PLOT_AXIS_RIGHT,
    PLOT_AXIS_BOTTOM,
    PLOT_AXIS_TOP
};

enum class RegionAnnotationType
{
    NO_ANNOTATIONS        = 0,
    FORMATION_ANNOTATIONS = 1,
    // Used to have Wbs-parameter coding as 2
    RESULT_PROPERTY_ANNOTATIONS = 3
};
enum RegionDisplay
{
    DARK_LINES              = 0x01,
    COLORED_LINES           = 0x02,
    COLOR_SHADING           = 0x04,
    COLOR_SHADING_AND_LINES = 0x05,
    LIGHT_LINES             = 0x08,
};
enum class TrackSpan
{
    FULL_WIDTH,
    LEFT_COLUMN,
    CENTRE_COLUMN,
    RIGHT_COLUMN
};
enum class Orientation
{
    HORIZONTAL = 0,
    VERTICAL
};

enum class MultiPlotAxisVisibility
{
    ONE_VISIBLE,
    ALL_VISIBLE
};

enum class ObjectNamingMethod
{
    CUSTOM,
    AUTO,
    TEMPLATE
};

enum class WindowTileMode
{
    DEFAULT,
    VERTICAL,
    HORIZONTAL,
    UNDEFINED,
};

enum class TextAlignment
{
    LEFT,
    RIGHT,
    CENTER
};

enum class ReadOutType
{
    NONE,
    SNAP_TO_POINT,
    TIME_TRACKING,
    TIME_VALUE_TRACKING
};

// Defines relate to curve and plot template names
QString namingVariableCase();
QString namingVariableWell();
QString namingVariableRefWell();
QString namingVariableWellBranch();
QString namingVariableResultName();
QString namingVariableResultType();
QString namingVariableTime();
QString namingVariableTimestep();
QString namingVariableAirGap();
QString namingVariableWaterDepth();

QString selectionTextNone();

double minimumDefaultValuePlot();
double minimumDefaultLogValuePlot();
double maximumDefaultValuePlot();

bool     isHorizontal( PlotAxis axis );
bool     isVertical( PlotAxis axis );
PlotAxis opposite( PlotAxis axis );

double scalingFactor( QPaintDevice* paintDevice );

// Project editor group names
QString curveNameGroupName();
QString appearanceGroupName();
QString additionalDataSourcesGroupName();

}; // namespace RiaDefines
