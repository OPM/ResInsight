/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimFaultDistanceCollection.h"

#include "RimFaultDistance.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include <QRegularExpression>

CAF_PDM_SOURCE_INIT( RimFaultDistanceCollection, "RimFaultDistanceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultDistanceCollection::RimFaultDistanceCollection()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Fault Distance",
                                                    ":/draw_style_faults_24x24.png",
                                                    "",
                                                    "",
                                                    "FaultDistanceCollection",
                                                    "Collection of named, subset-based FAULTDIST results" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_items, "FaultDistances", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultDistance* RimFaultDistanceCollection::addResult()
{
    auto* newResult = new RimFaultDistance();
    newResult->setResultName( nextDefaultName() );
    addItem( newResult );
    return newResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultDistanceCollection::nextDefaultName() const
{
    QRegularExpression pattern( "^FAULTDIST(\\d+)$" );
    int                maxIndex = 0;
    for ( RimFaultDistance* result : items() )
    {
        if ( !result ) continue;
        const QRegularExpressionMatch match = pattern.match( result->resultName() );
        if ( match.hasMatch() )
        {
            maxIndex = std::max( maxIndex, match.captured( 1 ).toInt() );
        }
    }
    return QString( "FAULTDIST%1" ).arg( maxIndex + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( RimFaultDistance* result : items() )
    {
        uiTreeOrdering.add( result );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewFaultDistanceResultFeature";
}
