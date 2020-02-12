/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimOilField.h"

#include "RimAnnotationCollection.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechModels.h"
#include "RimMeasurement.h"
#include "RimObservedDataCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSurfaceCollection.h"
#include "RimWellPathCollection.h"

CAF_PDM_SOURCE_INIT( RimOilField, "ResInsightOilField" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilField::RimOilField( void )
{
    CAF_PDM_InitObject( "Oil Field", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &analysisModels, "AnalysisModels", "Grid Models", ":/GridModels.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &geoMechModels, "GeoMechModels", "Geo Mech Models", ":/GridModels.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &wellPathCollection, "WellPathCollection", "Well Paths", ":/WellCollection.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &completionTemplateCollection, "CompletionTemplateCollection", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &summaryCaseMainCollection,
                                "SummaryCaseCollection",
                                "Summary Cases",
                                ":/GridModels.png",
                                "",
                                "" );
    CAF_PDM_InitFieldNoDefault( &formationNamesCollection, "FormationNamesCollection", "Formations", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &observedDataCollection,
                                "ObservedDataCollection",
                                "Observed Data",
                                ":/Cases16x16.png",
                                "",
                                "" );

    CAF_PDM_InitFieldNoDefault( &annotationCollection, "AnnotationCollection", "Annotations", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureTemplateCollection_OBSOLETE,
                                "FractureDefinitionCollection",
                                "Defenition of Fractures",
                                "",
                                "",
                                "" );

    CAF_PDM_InitFieldNoDefault( &measurement, "Measurement", "Measurement", "", "", "" );
    measurement = new RimMeasurement();
    measurement.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &surfaceCollection, "SurfaceCollection", "Surfaces", "", "", "" );
    surfaceCollection = new RimSurfaceCollection();

    completionTemplateCollection = new RimCompletionTemplateCollection;
    analysisModels               = new RimEclipseCaseCollection();
    wellPathCollection           = new RimWellPathCollection();
    summaryCaseMainCollection    = new RimSummaryCaseMainCollection();
    observedDataCollection       = new RimObservedDataCollection();
    formationNamesCollection     = new RimFormationNamesCollection();
    annotationCollection         = new RimAnnotationCollection();

    m_fractureTemplateCollection_OBSOLETE = new RimFractureTemplateCollection;
    m_fractureTemplateCollection_OBSOLETE.xmlCapability()->setIOWritable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilField::~RimOilField( void )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection* RimOilField::fractureDefinitionCollection()
{
    return completionTemplateCollection()->fractureTemplateCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimFractureTemplateCollection* RimOilField::fractureDefinitionCollection() const
{
    return completionTemplateCollection()->fractureTemplateCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplateCollection* RimOilField::valveTemplateCollection()
{
    return completionTemplateCollection()->valveTemplateCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimValveTemplateCollection* RimOilField::valveTemplateCollection() const
{
    return completionTemplateCollection()->valveTemplateCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilField::initAfterRead()
{
    RimFractureTemplateCollection* fractureTemplateCollection = m_fractureTemplateCollection_OBSOLETE.value();
    if ( !fractureTemplateCollection->fractureTemplates().empty() )
    {
        m_fractureTemplateCollection_OBSOLETE.removeChildObject( fractureTemplateCollection );
        completionTemplateCollection->setFractureTemplateCollection( fractureTemplateCollection );
    }
}
