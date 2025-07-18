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
#include "RimcWellLogPlot.h"

#include "RiaApplication.h"

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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogPlot, RimcWellLogPlot_newWellLogTrack, "NewWellLogTrack" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLogPlot_newWellLogTrack::RimcWellLogPlot_newWellLogTrack( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Well Log Track", "", "", "Create a new well log track" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_title, "Title", "", "", "", "Title" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellLogPlot_newWellLogTrack::execute()
{
    RimWellLogPlot* wellLogPlot = self<RimWellLogPlot>();

    if ( !wellLogPlot ) return std::unexpected( "No well log plot found" );

    return createWellLogTrack( wellLogPlot, m_case(), m_wellPath(), m_title() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RimcWellLogPlot_newWellLogTrack::createWellLogTrack( RimWellLogPlot* wellLogPlot,
                                                                      RimEclipseCase* eclipseCase,
                                                                      RimWellPath*    wellPath,
                                                                      const QString&  title )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, title, wellLogPlot );
    if ( eclipseCase ) plotTrack->setFormationCase( eclipseCase );
    if ( wellPath ) plotTrack->setFormationWellPath( wellPath );

    plotTrack->setColSpan( RimPlot::TWO );
    plotTrack->setLegendsVisible( true );
    plotTrack->setPlotTitleVisible( true );
    plotTrack->setShowWindow( true );
    plotTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setAutoScalePropertyValuesEnabled( true );
    plotTrack->updateConnectedEditors();

    wellLogPlot->setShowWindow( true );
    wellLogPlot->updateConnectedEditors();
    wellLogPlot->loadDataAndUpdate();

    return plotTrack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellLogPlot_newWellLogTrack::classKeywordReturnedType() const
{
    return RimWellLogTrack::classKeywordStatic();
}
