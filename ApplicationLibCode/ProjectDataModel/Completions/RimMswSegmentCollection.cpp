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

#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimMswSegment.h"
#include "RimProject.h"
#include "RimWellPath.h"
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
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );

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

    // Create segments with computed start/end MD
    for ( size_t i = 0; i < welsegsData.size(); ++i )
    {
        const auto& row = welsegsData[i];

        double startMD = row.length;
        double endMD   = ( i + 1 < welsegsData.size() ) ? welsegsData[i + 1].length : wellTotalDepth;

        // Skip if start >= end (invalid interval)
        if ( startMD >= endMD ) continue;

        double diameter = row.diameter.value_or( 0.1 ); // Default diameter if not specified

        auto* segment = new RimMswSegment();
        segment->setSegmentData( row.segment1, row.branch, startMD, endMD, diameter );
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
RimEclipseCase* RimMswSegmentCollection::eclipseCase() const
{
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::updateSegments()
{
    // Clear existing segments before creating new ones
    clearSegments();

    auto scheduleRedraw = []()
    {
        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    };

    auto* wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath )
    {
        RiaLogging::error( "Unable to update MSW segments: no well path found." );
        scheduleRedraw();
        return;
    }

    if ( !m_eclipseCase() )
    {
        RiaLogging::error( "Unable to update MSW segments: no Eclipse case selected." );
        scheduleRedraw();
        return;
    }

    constexpr int timeStep        = 0;
    auto          tableDataResult = RicWellPathExportMswTableData::extractSingleWellMswData( m_eclipseCase(), wellPath, timeStep );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_isChecked );
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.addNewButton( "Update Segments", [this]() { updateSegments(); } );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMswSegmentCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        auto cases = RimEclipseCaseTools::eclipseCases();
        for ( auto* c : cases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isChecked )
    {
        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}
