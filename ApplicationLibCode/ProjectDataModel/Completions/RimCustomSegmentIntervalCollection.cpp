/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimCustomSegmentIntervalCollection.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimCustomSegmentIntervalCollection, "CustomSegmentIntervalCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentIntervalCollection::RimCustomSegmentIntervalCollection()
{
    CAF_PDM_InitObject( "Custom Segment Intervals", ":/WellPathComponent16x16.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Intervals", "Intervals" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentIntervalCollection::~RimCustomSegmentIntervalCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimCustomSegmentIntervalCollection::validate( const QString& configName ) const
{
    // First validate all fields using base implementation
    auto errors = PdmObject::validate( configName );

    QStringList validationIssues;

    // Validate each child interval
    for ( const auto* interval : intervals() )
    {
        if ( interval )
        {
            // Get validation errors from the interval itself
            auto intervalErrors = interval->validate( configName );
            for ( const auto& [field, errorMsg] : intervalErrors )
            {
                validationIssues.append( QString( "Interval (%1): %2." ).arg( interval->generateDisplayLabel() ).arg( errorMsg ) );
            }
        }
    }

    // Check for overlaps between intervals
    for ( size_t i = 0; i < intervals().size(); ++i )
    {
        for ( size_t j = i + 1; j < intervals().size(); ++j )
        {
            RimCustomSegmentInterval* intervalI = intervals()[i];
            RimCustomSegmentInterval* intervalJ = intervals()[j];
            if ( intervalI && intervalJ && intervalI->overlaps( intervalJ ) )
            {
                validationIssues.append( QString( "Interval (%1) overlaps with another interval (%2)." )
                                             .arg( intervalI->generateDisplayLabel() )
                                             .arg( intervalJ->generateDisplayLabel() ) );
            }
        }
    }

    // If there are any validation issues, combine them into a single error message
    if ( !validationIssues.isEmpty() )
    {
        errors["Intervals"] = validationIssues.join( "\n" );
    }

    return errors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                                  QMenu*                     menu,
                                                                  QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewCustomSegmentIntervalFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteCustomSegmentIntervalFeature";

    menuBuilder.appendToMenu( menu );
}
