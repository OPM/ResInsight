/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModelPlotCollection.h"

#include "RimFractureModelPlot.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimFractureModelPlotCollection, "FractureModelPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlotCollection::RimFractureModelPlotCollection()
{
    CAF_PDM_InitScriptableObject( "Fracture Model Plots", ":/WellLogPlots16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureModelPlots, "FractureModelPlots", "", "", "", "" );
    m_fractureModelPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlotCollection::~RimFractureModelPlotCollection()
{
    m_fractureModelPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlotCollection::reloadAllPlots()
{
    for ( const auto& w : m_fractureModelPlots() )
    {
        w->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlotCollection::addFractureModelPlot( RimFractureModelPlot* newPlot )
{
    m_fractureModelPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureModelPlot*> RimFractureModelPlotCollection::fractureModelPlots() const
{
    return m_fractureModelPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlotCollection::deleteAllPlots()
{
    m_fractureModelPlots.deleteAllChildObjects();
}
