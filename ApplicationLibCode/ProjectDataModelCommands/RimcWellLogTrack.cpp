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
#include "RimcWellLogTrack.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaGuiApplication.h"

#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogTrack, RimcWellLogTrack_addExtractionCurve, "AddExtractionCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLogTrack_addExtractionCurve::RimcWellLogTrack_addExtractionCurve( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Well Log Extraction Curve", "", "", "Create a well log extraction curve" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "", "", "", "Property Type" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "", "", "", "Property Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "", "", "", "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcWellLogTrack_addExtractionCurve::execute()
{
    RimWellLogTrack* wellLogTrack = self<RimWellLogTrack>();

    // Make sure the plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    if ( m_case && m_wellPath && wellLogTrack )
    {
        RiaDefines::ResultCatType resultCategoryType = caf::AppEnum<RiaDefines::ResultCatType>::fromText( m_propertyType );

        return addExtractionCurve( wellLogTrack, m_case, m_wellPath, m_propertyName, resultCategoryType, m_timeStep );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve*
    RimcWellLogTrack_addExtractionCurve::addExtractionCurve( RimWellLogTrack*          wellLogTrack,
                                                             RimEclipseCase*           eclipseCase,
                                                             RimWellPath*              wellPath,
                                                             const QString&            propertyName,
                                                             RiaDefines::ResultCatType resultCategoryType,
                                                             int                       timeStep )
{
    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve;
    curve->setWellPath( wellPath );
    curve->setCase( eclipseCase );
    curve->setCurrentTimeStep( timeStep );
    curve->setEclipseResultVariable( propertyName );

    curve->setEclipseResultCategory( resultCategoryType );

    wellLogTrack->addCurve( curve );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();

    wellLogTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    wellLogTrack->setShowRegionLabels( true );
    wellLogTrack->setAutoScaleXEnabled( true );
    wellLogTrack->updateConnectedEditors();
    wellLogTrack->setShowWindow( true );

    RiaApplication::instance()->project()->updateConnectedEditors();

    RimWellLogPlot* wellLogPlot = dynamic_cast<RimWellLogPlot*>( wellLogTrack->parentField() );
    if ( wellLogPlot ) wellLogPlot->loadDataAndUpdate();

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcWellLogTrack_addExtractionCurve::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcWellLogTrack_addExtractionCurve::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimWellLogExtractionCurve );
}
