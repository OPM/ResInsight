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

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
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

    setDeletable( true );
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
    RimDepthTrackPlot::operator=( std::move( rhs ) );
    return *this;
}
