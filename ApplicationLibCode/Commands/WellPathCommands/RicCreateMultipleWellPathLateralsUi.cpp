/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RicCreateMultipleWellPathLateralsUi.h"

#include "RifTextDataTableFormatter.h"

#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathTieIn.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafSelectionManagerTools.h"

#include "cvfBoundingBox.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RicCreateMultipleWellPathLateralsUi, "RicCreateMultipleWellPathLateralsUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateMultipleWellPathLateralsUi::RicCreateMultipleWellPathLateralsUi()
{
    CAF_PDM_InitFieldNoDefault( &m_sourceLateral, "SourceLaterals", "Source Well Path Lateral", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_topLevelWellPath, "TopLevelWellPath", "Top Level Well Path", "", "", "" );
    m_topLevelWellPath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_locations, "Locations", "Locations", "", "", "" );
    m_locations = new RimMultipleLocations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::setTopLevelWellPath( RimWellPath* wellPath )
{
    m_topLevelWellPath = wellPath;

    auto laterals = RimTools::wellPathCollection()->connectedWellPathLaterals( m_topLevelWellPath );

    if ( !laterals.empty() ) m_sourceLateral = dynamic_cast<RimModeledWellPath*>( laterals.front() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::setDefaultValues( double start, double end )
{
    m_locations->setRange( start, end );
    m_locations->computeRangesAndLocations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath* RicCreateMultipleWellPathLateralsUi::sourceLateral() const
{
    return m_sourceLateral;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultipleLocations* RicCreateMultipleWellPathLateralsUi::locationConfig() const
{
    return m_locations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_sourceLateral );

    {
        auto group = uiOrdering.addNewGroup( "Locations" );
        m_locations->uiOrdering( uiConfigName, *group );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicCreateMultipleWellPathLateralsUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_sourceLateral )
    {
        if ( m_topLevelWellPath )
        {
            auto laterals = RimTools::wellPathCollection()->connectedWellPathLaterals( m_topLevelWellPath );

            for ( auto lateral : laterals )
            {
                caf::IconProvider iconProvider = lateral->uiIconProvider();

                options.push_back( caf::PdmOptionItemInfo( lateral->name(), lateral, false, iconProvider ) );
            }
        }
    }

    return options;
}
