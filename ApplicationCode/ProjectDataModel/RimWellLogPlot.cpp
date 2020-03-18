/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogPlot.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogTrack.h"

#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogPlot.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObjectScriptability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cvfAssert.h"

#include <QKeyEvent>

#include <cmath>

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

CAF_PDM_SOURCE_INIT( RimWellLogPlot, "WellLogPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::RimWellLogPlot()
{
    CAF_PDM_InitScriptableObject( "Well Log Plot",
                                  ":/WellLogPlot16x16.png",
                                  "",
                                  "A Well Log Plot With a shared Depth Axis and Multiple Tracks" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimWellLogPlot& RimWellLogPlot::operator=( RimWellLogPlot&& rhs )
{
    RimPlotWindow::operator=( std::move( rhs ) );

    // Move all tracks
    std::vector<RimPlot*> plots = rhs.m_plots.childObjects();
    rhs.m_plots.clear();
    for ( RimPlot* plot : plots )
    {
        m_plots.push_back( plot );
    }

    // Deliberately don't set m_plotWindowTitle. This operator is used for copying parameters from children.
    // This only happens for some plots that used to own a plot but now inherits the plot.
    // These all had their own description at top level which we don't want to overwrite.

    m_showPlotWindowTitle = rhs.m_showPlotWindowTitle;

    auto dataSource = rhs.m_commonDataSource();
    rhs.m_commonDataSource.removeChildObject( dataSource );
    m_commonDataSource = dataSource;

    m_depthType               = rhs.m_depthType();
    m_depthUnit               = rhs.m_depthUnit();
    m_minVisibleDepth         = rhs.m_minVisibleDepth();
    m_maxVisibleDepth         = rhs.m_maxVisibleDepth();
    m_depthAxisGridVisibility = rhs.m_depthAxisGridVisibility();
    m_isAutoScaleDepthEnabled = rhs.m_isAutoScaleDepthEnabled();

    // Deliberately don't copy m_nameConfig. This operator is used for copying parameters from children.
    // This only happens for some plots that used to own a plot but now inherits the plot.
    // These all had their own description at top level which we don't want to overwrite.

    m_minAvailableDepth = rhs.m_minAvailableDepth;
    m_maxAvailableDepth = rhs.m_maxAvailableDepth;
    return *this;
}
