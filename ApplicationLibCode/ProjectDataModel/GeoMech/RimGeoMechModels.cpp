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

#include "RimGeoMechModels.h"

#include "RiaLogging.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"

CAF_PDM_SOURCE_INIT( RimGeoMechModels, "ResInsightGeoMechModels" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechModels::RimGeoMechModels( void )
{
    CAF_PDM_InitObject( "Geomechanical Models", ":/GeoMechCases48x48.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "Cases", "" );
    m_cases.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechModels::~RimGeoMechModels( void )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechCase*> RimGeoMechModels::cases() const
{
    return m_cases.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechModels::addCase( RimGeoMechCase* thecase )
{
    m_cases.push_back( thecase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechModels::removeCase( RimGeoMechCase* thecase )
{
    m_cases.removeChildObject( thecase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechModels::copyCase( RimGeoMechCase* thecase, const QString& newInputFileName )
{
    for ( auto gmcase : m_cases() )
    {
        if ( gmcase->gridFileName() == newInputFileName )
        {
            RiaLogging::warning( "File has already been opened. Cannot open the file twice! - " + newInputFileName );
            return nullptr;
        }
    }

    RimGeoMechCase* copy = thecase->createCopy( newInputFileName );
    if ( !copy )
    {
        RiaLogging::warning( "Could not create a copy of the geomech case" + thecase->caseUserDescription() +
                             " using the new input file " + newInputFileName );
        return nullptr;
    }

    m_cases.push_back( copy );

    copy->resolveReferencesRecursively();

    copy->updateConnectedEditors();
    this->updateConnectedEditors();

    for ( auto riv : copy->views() )
    {
        RimGridView* rgv = dynamic_cast<RimGridView*>( riv );
        if ( rgv )
        {
            rgv->loadDataAndUpdate();
            rgv->scheduleCreateDisplayModelAndRedraw();
            rgv->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();

            for ( auto coll : rgv->separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
            {
                coll->update2dIntersectionViews();
            }

            for ( auto coll : rgv->separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
            {
                coll->update2dIntersectionViews();
            }
        }
    }

    return copy;
}
