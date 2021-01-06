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
#include "RimMultiPlotCollection.h"

#include "RimMultiPlot.h"
#include "RimProject.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimMultiPlotCollection, "RimMultiPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotCollection::RimMultiPlotCollection()
{
    CAF_PDM_InitObject( "Multi Plots", ":/MultiPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_multiPlots, "MultiPlots", "Plots Reports", "", "", "" );
    m_multiPlots.uiCapability()->setUiHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_multiPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotCollection::~RimMultiPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotCollection::deleteAllChildObjects()
{
    m_multiPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimMultiPlot*> RimMultiPlotCollection::multiPlots() const
{
    return m_multiPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot* RimMultiPlotCollection::createMultiPlot()
{
    RimMultiPlot* plot = new RimMultiPlot();
    plot->setAsPlotMdiWindow();

    addMultiPlot( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotCollection::addMultiPlot( RimMultiPlot* plot )
{
    m_multiPlots().push_back( plot );
}
