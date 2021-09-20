/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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
#include "RimcWellLogPlotCollection.h"

#include "RiaApplication.h"

#include "RimcWellLogPlot.h"
#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogPlotCollection, RimcWellLogPlotCollection_newWellLogPlot, "NewWellLogPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLogPlotCollection_newWellLogPlot::RimcWellLogPlotCollection_newWellLogPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Well Log Plot", "", "", "Create a new well log plot" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "", "", "", "Property Type" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "", "", "", "Property Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "", "", "", "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcWellLogPlotCollection_newWellLogPlot::execute()
{
    RimWellLogPlot*           newWellLogPlot        = nullptr;
    RimWellLogPlotCollection* wellLogPlotCollection = self<RimWellLogPlotCollection>();

    if ( m_case && m_wellPath && wellLogPlotCollection )
    {
        newWellLogPlot = createWellLogPlot( wellLogPlotCollection, m_wellPath, m_case );
    }

    return newWellLogPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RimcWellLogPlotCollection_newWellLogPlot::createWellLogPlot( RimWellLogPlotCollection* wellLogPlotCollection,
                                                                             RimWellPath*              wellPath,
                                                                             RimEclipseCase*           eclipseCase )
{
    RimWellLogPlot* newWellLogPlot = new RimWellLogPlot;
    newWellLogPlot->setAsPlotMdiWindow();

    wellLogPlotCollection->addWellLogPlot( newWellLogPlot );

    newWellLogPlot->commonDataSource()->setCaseToApply( eclipseCase );
    newWellLogPlot->commonDataSource()->setWellPathToApply( wellPath );
    newWellLogPlot->loadDataAndUpdate();
    newWellLogPlot->updateConnectedEditors();
    return newWellLogPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcWellLogPlotCollection_newWellLogPlot::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcWellLogPlotCollection_newWellLogPlot::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimWellLogPlot );
}
