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
#include "RiaResultNames.h"

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
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Well Log Extraction Curve", "", "", "Create a well log extraction curve" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "", "", "", "Property Type" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "", "", "", "Property Name" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "", "", "", "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellLogTrack_addExtractionCurve::execute()
{
    RimWellLogTrack* wellLogTrack = self<RimWellLogTrack>();

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
RimWellLogExtractionCurve* RimcWellLogTrack_addExtractionCurve::addExtractionCurve( RimWellLogTrack*          wellLogTrack,
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

    if ( resultCategoryType == RiaDefines::ResultCatType::FORMATION_NAMES )
    {
        // The result name for formations is fragile. Always use the application result name for this
        curve->setEclipseResultVariable( RiaResultNames::activeFormationNamesResultName() );
    }
    else
    {
        curve->setEclipseResultVariable( propertyName );
    }

    curve->setEclipseResultCategory( resultCategoryType );

    wellLogTrack->addCurve( curve );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();

    wellLogTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    wellLogTrack->setShowRegionLabels( true );
    wellLogTrack->setAutoScalePropertyValuesEnabled( true );
    wellLogTrack->updateConnectedEditors();
    wellLogTrack->setShowWindow( true );

    RimWellLogPlot* wellLogPlot = dynamic_cast<RimWellLogPlot*>( wellLogTrack->parentField() );
    if ( wellLogPlot ) wellLogPlot->loadDataAndUpdate();

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellLogTrack_addExtractionCurve::classKeywordReturnedType() const
{
    return RimWellLogExtractionCurve::classKeywordStatic();
}
