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

    CAF_PDM_InitFieldNoDefault( &m_locations, "Locations", "Locations", "", "", "" );
    m_locations = new RimMultipleLocations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::setSourceLateral( RimModeledWellPath* lateral )
{
    m_sourceLateral = lateral;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::setDefaultValues( double start, double end )
{
    m_locations->setRange( start, end );
    m_locations->updateRangesAndLocations();
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

    //     if ( fieldNeedingOptions == &m_sourceCase )
    //     {
    //         RimTools::caseOptionItems( &options );
    //     }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLateralsUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                                 QString                    uiConfigName,
                                                                 caf::PdmUiEditorAttribute* attribute )
{
    /*
        if ( field == &m_fractureCreationSummary )
        {
            auto attr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
            if ( attr )
            {
                QFont font( "Courier", 8 );

                attr->font     = font;
                attr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            }
        }
        else if ( field == &m_options )
        {
            auto attr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
            if ( attr )
            {
                attr->minimumHeight = 130;
                attr->columnWidths  = { 90, 90, 400, 70 };
            }
        }
    */
}
