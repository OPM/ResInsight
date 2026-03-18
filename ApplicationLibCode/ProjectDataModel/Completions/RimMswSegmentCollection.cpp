/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimMswSegmentCollection.h"

#include "RiaLogging.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimMswSegment.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathCompletions.h"

#include <algorithm>
#include <map>

CAF_PDM_SOURCE_INIT( RimMswSegmentCollection, "RimMswSegmentCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswSegmentCollection::RimMswSegmentCollection()
{
    CAF_PDM_InitObject( "MSW Segments", ":/WellCollection.png" );

    CAF_PDM_InitFieldNoDefault( &m_segments, "Segments", "Segments" );

    // m_isChecked field is defined in the base class RimCheckableNamedObject. Update its UI properties here.
    m_isChecked.uiCapability()->setUiHidden( false );
    m_isChecked.uiCapability()->setUiName( "Show MSW Segments" );

    setName( "MSW Segments" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswSegmentCollection::hasSegments() const
{
    return !m_segments.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimMswSegment*> RimMswSegmentCollection::segments() const
{
    std::vector<const RimMswSegment*> result;
    for ( const auto& seg : m_segments )
    {
        result.push_back( seg );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::appendSegment( RimMswSegment* segment )
{
    m_segments.push_back( segment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::clearSegments()
{
    m_segments.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::populateFromWelsegsData( std::vector<WelsegsRow> welsegsData, double wellTotalDepth )
{
    clearSegments();

    if ( welsegsData.empty() ) return;

    // Sort segments by measured depth (length)
    std::sort( welsegsData.begin(), welsegsData.end(), []( const WelsegsRow& a, const WelsegsRow& b ) { return a.length < b.length; } );

    // Assign segments data. Start/end measured depth is used for visualization in 2D plots by RimWellPathComponentInterface
    for ( size_t i = 0; i < welsegsData.size(); ++i )
    {
        const auto& row = welsegsData[i];

        double nextSegmentLength = ( i + 1 < welsegsData.size() ) ? welsegsData[i + 1].length : wellTotalDepth;

        double diameter = row.diameter.value_or( 0.1 ); // Default diameter if not specified

        auto* segment = new RimMswSegment();
        segment->setSegmentData( row.segment1, row.branch, row.joinSegment, row.length, row.depth, nextSegmentLength, diameter );
        appendSegment( segment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegmentCollection::referenceDiameter() const
{
    if ( m_segments.empty() ) return 0.1; // Default diameter

    // Use the first segment's diameter as reference
    double firstDiameter = m_segments[0]->diameter();
    if ( firstDiameter > 0.0 ) return firstDiameter;

    return 0.1; // Default if no diameter specified
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::updateSegments( RimWellPath* topLevelWell, RimEclipseCase* eclipseCase )
{
    if ( !topLevelWell || !eclipseCase )
    {
        return;
    }

    auto scheduleRedraw = []()
    {
        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    };

    auto exportDate                             = RicWellPathExportCompletionDataFeatureImpl::exportDateForTimeStep( eclipseCase, 0 );
    bool exportCompletionsAfterMainBoreSegments = true;

    auto tableDataResult = RicWellPathExportMswTableData::extractSingleWellMswData( eclipseCase,
                                                                                    topLevelWell,
                                                                                    exportCompletionsAfterMainBoreSegments,
                                                                                    RicWellPathExportMswTableData::CompletionType::ALL,
                                                                                    exportDate );

    if ( !tableDataResult.has_value() )
    {
        RiaLogging::error( QString::fromStdString( tableDataResult.error() ) );
        scheduleRedraw();
        return;
    }

    const RigMswTableData& tableData = tableDataResult.value();

    // Group segments by source well name
    std::map<std::string, std::vector<WelsegsRow>> segmentsByWell;
    for ( const auto& row : tableData.welsegsData() )
    {
        segmentsByWell[row.sourceWellName].push_back( row );
    }

    // Update MSW segments for each well path
    for ( const auto& [sourceWellName, segments] : segmentsByWell )
    {
        RimWellPath* targetWellPath = RimProject::current()->wellPathByName( QString::fromStdString( sourceWellName ) );
        if ( !targetWellPath )
        {
            RiaLogging::warning(
                QString( "Unable to find well path '%1' for MSW segment update." ).arg( QString::fromStdString( sourceWellName ) ) );
            continue;
        }

        if ( !targetWellPath->completions() || !targetWellPath->completions()->mswSegmentCollection() )
        {
            continue;
        }

        double wellTotalDepth = 0.0;
        if ( auto* geom = targetWellPath->wellPathGeometry() )
        {
            auto mds = geom->uniqueMeasuredDepths();
            if ( !mds.empty() ) wellTotalDepth = mds.back();
        }

        auto* segCollection = targetWellPath->completions()->mswSegmentCollection();
        segCollection->populateFromWelsegsData( segments, wellTotalDepth );
        segCollection->updateConnectedEditors();
    }

    scheduleRedraw();
}
