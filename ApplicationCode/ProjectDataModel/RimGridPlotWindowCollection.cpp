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
#include "RimGridPlotWindowCollection.h"

#include "RiaApplication.h"
#include "RimGridPlotWindow.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimGridPlotWindowCollection, "RimGridPlotWindowCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridPlotWindowCollection::RimGridPlotWindowCollection()
{
    CAF_PDM_InitObject( "Combination Plots", ":/WellLogPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_gridPlotWindows, "GridPlotWindows", "Combination Plots", "", "", "" );
    m_gridPlotWindows.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridPlotWindowCollection::~RimGridPlotWindowCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridPlotWindowCollection::deleteAllChildObjects()
{
    m_gridPlotWindows.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridPlotWindow*> RimGridPlotWindowCollection::gridPlotWindows() const
{
    return m_gridPlotWindows.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridPlotWindow* RimGridPlotWindowCollection::createGridPlotWindow()
{
    RimGridPlotWindow* plot = new RimGridPlotWindow();
    plot->setAsPlotMdiWindow();

    addGridPlotWindow( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridPlotWindowCollection::addGridPlotWindow( RimGridPlotWindow* plot )
{
    m_gridPlotWindows().push_back( plot );
}
