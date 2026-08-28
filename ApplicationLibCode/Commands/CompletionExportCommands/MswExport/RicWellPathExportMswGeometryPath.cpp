/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicWellPathExportMswGeometryPath.h"

#include "RicMswBranchBuilder.h"
#include "RicMswTableDataTools.h"
#include "RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimValveCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathTieIn.h"
#include "RimWellPathValve.h"

#include "cafAssert.h"

#include <algorithm>
#include <numeric>
#include <set>
#include <tuple>

namespace RicWellPathExportMswGeometryPath
{

using CompletionType = RicWellPathExportMswTableData::CompletionType;

//--------------------------------------------------------------------------------------------------
/// Number of well path laterals in the tie-in tree, including the given well path itself.
//--------------------------------------------------------------------------------------------------
static int lateralCount( const RimWellPath* wellPath )
{
    int count = 1;
    for ( const auto* childWellPath : RicWellPathExportMswTableData::wellPathsWithTieIn( wellPath ) )
    {
        count += lateralCount( childWellPath );
    }
    return count;
}

//--------------------------------------------------------------------------------------------------
/// Recursively build all WELSEGS/COMPSEGS/valve segments for one lateral (child well path)
/// and any of its own child laterals.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildLateralBranches( RimEclipseCase*                              eclipseCase,
                                                const RimWellPath*                           wellPath,
                                                const RigMainGrid*                           mainGrid,
                                                int                                          outletSegNum,
                                                CompletionType                               completionType,
                                                const std::optional<QDateTime>&              exportDate,
                                                int&                                         segmentNumber,
                                                int&                                         lateralBranchNumber,
                                                int&                                         completionBranchNumber,
                                                RiaDefines::EclipseUnitSystem                unitSystem,
                                                RicMswBranchBuilder::FishbonesExportContext& fishbonesContext )
{
    std::vector<RigMswBranch> result;
    auto                      mswParameters = wellPath->mswCompletionParameters();
    if ( !mswParameters ) return result;

    const std::string infoType          = mswParameters->lengthAndDepth().text().toStdString();
    const double      tieInMD           = wellPath->wellPathTieIn()->tieInMeasuredDepth();
    const double      tieInTVD          = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( tieInMD ).z();
    const std::string wellNameForExport = wellPath->completionSettings()->wellNameForExport().toStdString();

    const int lateralBranchNum = ++lateralBranchNumber;
    int       childOutletSeg   = outletSegNum;

    // Optional ICV valve at the tie-in point — stored on the branch, not as a separate branch
    std::optional<RigMswSegment> tieInValve;
    const RimWellPathValve*      outletValve = wellPath->wellPathTieIn()->outletValve();
    if ( outletValve && outletValve->isChecked() )
    {
        const bool isActive = !exportDate.has_value() || outletValve->isActiveOnDate( *exportDate );
        if ( isActive )
        {
            const double valveMD       = wellPath->wellPathTieIn()->branchValveMeasuredDepth();
            const double offset        = ( valveMD == tieInMD ) ? RicMswTableDataTools::valveSegmentLength : 0.0;
            const double valveEndMD    = valveMD + offset;
            const double valveStartTVD = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveMD );
            const double valveEndTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveEndMD );

            double length = 0.0;
            double depth  = 0.0;
            if ( infoType == "INC" )
            {
                length = valveEndMD - valveMD;
                depth  = valveEndTVD - valveStartTVD;
            }
            else
            {
                length = valveEndMD;
                depth  = valveEndTVD;
            }

            const int valveSegNum = segmentNumber++;

            RigMswSegment valveSeg;
            valveSeg.segmentNumber       = valveSegNum;
            valveSeg.outletSegmentNumber = outletSegNum;
            valveSeg.length              = length;
            valveSeg.depth               = depth;
            valveSeg.diameter            = mswParameters->linerDiameter( unitSystem );
            valveSeg.roughness           = mswParameters->roughnessFactor( unitSystem );
            valveSeg.sourceWellName      = wellPath->name().toStdString();
            valveSeg.description = QString( "%1 valve for %2" ).arg( outletValve->componentLabel() ).arg( wellPath->name() ).toStdString();

            WsegvalvRow wv;
            wv.well               = wellNameForExport;
            wv.segmentNumber      = valveSegNum;
            wv.cv                 = outletValve->flowCoefficient();
            wv.area               = outletValve->area( unitSystem );
            wv.status             = outletValve->isOpen() ? "OPEN" : "SHUT";
            wv.description        = valveSeg.description;
            valveSeg.wsegvalvData = wv;

            tieInValve     = std::move( valveSeg );
            childOutletSeg = valveSegNum;
        }
    }

    // Child main-bore segments
    auto cellIntersections = RicWellPathExportMswTableData::generateCellSegments( eclipseCase, wellPath );
    auto filteredIntersections =
        RicWellPathExportMswTableData::filterIntersections( cellIntersections, tieInMD, wellPath->wellPathGeometry(), eclipseCase );

    const bool includePerforations = ( completionType & CompletionType::PERFORATIONS ) == CompletionType::PERFORATIONS;
    const std::vector<const RimPerforationInterval*> perforationIntervals =
        includePerforations ? wellPath->perforationIntervalCollection()->activePerforations() : std::vector<const RimPerforationInterval*>();

    std::set<const RimPerforationInterval*> valvedIntervals;
    for ( const auto* perf : perforationIntervals )
    {
        if ( exportDate.has_value() && !perf->isActiveOnDate( exportDate.value() ) ) continue;
        for ( const auto* valve : perf->descendantsIncludingThisOfType<RimWellPathValve>() )
        {
            if ( !valve->isChecked() ) continue;
            if ( exportDate.has_value() && !valve->isActiveOnDate( exportDate.value() ) ) continue;
            const auto t = valve->componentType();
            if ( t == RiaDefines::WellPathComponentType::ICD || t == RiaDefines::WellPathComponentType::ICV ||
                 t == RiaDefines::WellPathComponentType::AICD || t == RiaDefines::WellPathComponentType::SICD )
            {
                valvedIntervals.insert( perf );
                break;
            }
        }
    }

    std::vector<RicMswBranchBuilder::CellSegmentEntry> childCellSegMap;

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto lateralBranch = RicMswBranchBuilder::buildMainBoreBranch( wellPath,
                                                                   filteredIntersections,
                                                                   mainGrid,
                                                                   eclipseCase,
                                                                   perforationIntervals,
                                                                   valvedIntervals,
                                                                   infoType,
                                                                   tieInMD,
                                                                   tieInTVD,
                                                                   lateralBranchNum,
                                                                   segmentNumber,
                                                                   childOutletSeg,
                                                                   mswParameters->maxSegmentLength(),
                                                                   {},
                                                                   exportDate,
                                                                   unitSystem,
                                                                   &childCellSegMap,
                                                                   activeCellInfo );
    // Identify the segments of this lateral by name in the exported comment line
    if ( !lateralBranch.segments.empty() )
    {
        lateralBranch.segments.front().description = QString( "Segments on lateral %1" ).arg( wellPath->name() ).toStdString();
    }
    lateralBranch.tieInValve = std::move( tieInValve );

    // Standalone valves (from valveCollection, not inside perforation intervals)
    for ( const auto* valve : wellPath->valveCollection()->activeValves() )
    {
        if ( exportDate.has_value() && !valve->isActiveOnDate( *exportDate ) ) continue;

        const int segNum = RicMswBranchBuilder::findOutletSegmentForMD( childCellSegMap, valve->startMD() );
        for ( auto& seg : lateralBranch.segments )
        {
            if ( seg.segmentNumber == segNum )
            {
                WsegvalvRow wv;
                wv.well          = wellNameForExport;
                wv.segmentNumber = segNum;
                wv.cv            = valve->flowCoefficient();
                wv.area          = valve->area( unitSystem );
                wv.status        = valve->isOpen() ? "OPEN" : "SHUT";
                wv.description   = valve->name().toStdString();
                seg.wsegvalvData = wv;
                break;
            }
        }
    }

    result.push_back( std::move( lateralBranch ) );

    auto valveBranches = RicMswBranchBuilder::buildValveBranches( wellPath,
                                                                  filteredIntersections,
                                                                  mainGrid,
                                                                  eclipseCase,
                                                                  perforationIntervals,
                                                                  childCellSegMap,
                                                                  infoType,
                                                                  wellNameForExport,
                                                                  segmentNumber,
                                                                  completionBranchNumber,
                                                                  mswParameters->maxSegmentLength(),
                                                                  {},
                                                                  exportDate,
                                                                  unitSystem );
    result.insert( result.end(), std::make_move_iterator( valveBranches.begin() ), std::make_move_iterator( valveBranches.end() ) );

    if ( ( completionType & CompletionType::FRACTURES ) == CompletionType::FRACTURES )
    {
        auto fracBranches = RicMswBranchBuilder::buildFractureBranches( eclipseCase,
                                                                        wellPath,
                                                                        mainGrid,
                                                                        childCellSegMap,
                                                                        infoType,
                                                                        segmentNumber,
                                                                        completionBranchNumber );
        result.insert( result.end(), std::make_move_iterator( fracBranches.begin() ), std::make_move_iterator( fracBranches.end() ) );
    }

    if ( ( completionType & CompletionType::FISHBONES ) == CompletionType::FISHBONES )
    {
        auto fishBranches = RicMswBranchBuilder::buildFishbonesBranches( eclipseCase,
                                                                         wellPath,
                                                                         mainGrid,
                                                                         filteredIntersections,
                                                                         childCellSegMap,
                                                                         infoType,
                                                                         wellNameForExport,
                                                                         segmentNumber,
                                                                         completionBranchNumber,
                                                                         mswParameters->maxSegmentLength(),
                                                                         {},
                                                                         unitSystem,
                                                                         fishbonesContext );
        result.insert( result.end(), std::make_move_iterator( fishBranches.begin() ), std::make_move_iterator( fishBranches.end() ) );
    }

    // Recurse into grandchildren
    for ( auto* grandchild : RicWellPathExportMswTableData::wellPathsWithTieIn( wellPath ) )
    {
        const int grandchildOutlet =
            RicMswBranchBuilder::findOutletSegmentForMD( childCellSegMap, grandchild->wellPathTieIn()->branchValveMeasuredDepth() );
        auto grandchildBranches = buildLateralBranches( eclipseCase,
                                                        grandchild,
                                                        mainGrid,
                                                        grandchildOutlet,
                                                        completionType,
                                                        exportDate,
                                                        segmentNumber,
                                                        lateralBranchNumber,
                                                        completionBranchNumber,
                                                        unitSystem,
                                                        fishbonesContext );
        result.insert( result.end(), std::make_move_iterator( grandchildBranches.begin() ), std::make_move_iterator( grandchildBranches.end() ) );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Build MSW export data directly from well-path geometry and Rim completion objects.
///
/// Handles main-bore segments, perforation and valve completions (ICD/AICD/SICD/ICV),
/// fractures, fishbones laterals, and tie-in child laterals.
//--------------------------------------------------------------------------------------------------
RigMswWellExportData buildMswWellExportData( RimEclipseCase*                               eclipseCase,
                                             const RimWellPath*                            wellPath,
                                             double                                        maxSegmentLength,
                                             const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                             CompletionType                                completionType,
                                             const std::optional<QDateTime>&               exportDate )
{
    auto mswParameters = wellPath->mswCompletionParameters();
    CAF_ASSERT( mswParameters );

    const RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();
    const RigMainGrid*                  mainGrid   = eclipseCase->mainGrid();
    const std::string                   infoType   = mswParameters->lengthAndDepth().text().toStdString();

    auto cellIntersections = RicWellPathExportMswTableData::generateCellSegments( eclipseCase, wellPath );
    double initialMD = RicWellPathExportMswTableData::computeIntitialMeasuredDepth( eclipseCase, wellPath, mswParameters, cellIntersections );
    double initialTVD = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( initialMD ).z();

    auto filteredIntersections =
        RicWellPathExportMswTableData::filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    WelsegsHeader header;
    header.well               = wellPath->completionSettings()->wellNameForExport().toStdString();
    header.topLength          = initialMD;
    header.topDepth           = initialTVD;
    header.infoType           = infoType;
    header.pressureComponents = mswParameters->pressureDrop().text().toStdString();

    const bool includePerforations = ( completionType & CompletionType::PERFORATIONS ) == CompletionType::PERFORATIONS;
    const std::vector<const RimPerforationInterval*> perforationIntervals =
        includePerforations ? wellPath->perforationIntervalCollection()->activePerforations() : std::vector<const RimPerforationInterval*>();

    // Determine which perforation intervals have at least one active, checked valve.
    // Main bore COMPSEGS are skipped for these; the valve builder will emit them instead.
    std::set<const RimPerforationInterval*> valvedIntervals;
    for ( const auto* perf : perforationIntervals )
    {
        if ( exportDate.has_value() && !perf->isActiveOnDate( exportDate.value() ) ) continue;
        for ( const auto* valve : perf->descendantsIncludingThisOfType<RimWellPathValve>() )
        {
            if ( !valve->isChecked() ) continue;
            if ( exportDate.has_value() && !valve->isActiveOnDate( exportDate.value() ) ) continue;
            const auto t = valve->componentType();
            if ( t == RiaDefines::WellPathComponentType::ICD || t == RiaDefines::WellPathComponentType::ICV ||
                 t == RiaDefines::WellPathComponentType::AICD || t == RiaDefines::WellPathComponentType::SICD )
            {
                valvedIntervals.insert( perf );
                break;
            }
        }
    }

    int segmentNumber = 2; // Segment 1 is the implicit well heel.

    // The well path laterals are given their branch number first, so that the main bore and the
    // laterals occupy the lowest branch numbers. The completion branches (valves, fractures and
    // fishbones) are numbered after all laterals, but are still listed immediately after the
    // lateral they are connected to.
    int lateralBranchNumber    = 1; // Main bore is branch 1, incremented for each lateral.
    int completionBranchNumber = lateralCount( wellPath ); // Incremented for each completion branch.

    std::vector<RicMswBranchBuilder::CellSegmentEntry> cellSegMap;

    // Collected across the main bore and all tie-in laterals, applied once the branches are assembled.
    RicMswBranchBuilder::FishbonesExportContext fishbonesContext;

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto mainBoreBranch = RicMswBranchBuilder::buildMainBoreBranch( wellPath,
                                                                    filteredIntersections,
                                                                    mainGrid,
                                                                    eclipseCase,
                                                                    perforationIntervals,
                                                                    valvedIntervals,
                                                                    infoType,
                                                                    initialMD,
                                                                    initialTVD,
                                                                    lateralBranchNumber,
                                                                    segmentNumber,
                                                                    1, // outlet = heel (segment 1)
                                                                    maxSegmentLength,
                                                                    customSegmentIntervals,
                                                                    exportDate,
                                                                    unitSystem,
                                                                    &cellSegMap,
                                                                    activeCellInfo );

    const std::string wellNameForExport = wellPath->completionSettings()->wellNameForExport().toStdString();

    // Standalone valves (from valveCollection, not inside perforation intervals)
    for ( const auto* valve : wellPath->valveCollection()->activeValves() )
    {
        if ( exportDate.has_value() && !valve->isActiveOnDate( *exportDate ) ) continue;

        const int segNum = RicMswBranchBuilder::findOutletSegmentForMD( cellSegMap, valve->startMD() );
        for ( auto& seg : mainBoreBranch.segments )
        {
            if ( seg.segmentNumber == segNum )
            {
                WsegvalvRow wv;
                wv.well          = wellNameForExport;
                wv.segmentNumber = segNum;
                wv.cv            = valve->flowCoefficient();
                wv.area          = valve->area( unitSystem );
                wv.status        = valve->isOpen() ? "OPEN" : "SHUT";
                wv.description   = valve->name().toStdString();
                seg.wsegvalvData = wv;
                break;
            }
        }
    }

    auto valveBranches = RicMswBranchBuilder::buildValveBranches( wellPath,
                                                                  filteredIntersections,
                                                                  mainGrid,
                                                                  eclipseCase,
                                                                  perforationIntervals,
                                                                  cellSegMap,
                                                                  infoType,
                                                                  wellNameForExport,
                                                                  segmentNumber,
                                                                  completionBranchNumber,
                                                                  maxSegmentLength,
                                                                  customSegmentIntervals,
                                                                  exportDate,
                                                                  unitSystem );

    const bool                includeFractures = ( completionType & CompletionType::FRACTURES ) == CompletionType::FRACTURES;
    std::vector<RigMswBranch> fractureBranches;
    if ( includeFractures )
    {
        fractureBranches =
            RicMswBranchBuilder::buildFractureBranches( eclipseCase, wellPath, mainGrid, cellSegMap, infoType, segmentNumber, completionBranchNumber );
    }

    const bool                includeFishbones = ( completionType & CompletionType::FISHBONES ) == CompletionType::FISHBONES;
    std::vector<RigMswBranch> fishbonesBranches;
    if ( includeFishbones )
    {
        fishbonesBranches = RicMswBranchBuilder::buildFishbonesBranches( eclipseCase,
                                                                         wellPath,
                                                                         mainGrid,
                                                                         filteredIntersections,
                                                                         cellSegMap,
                                                                         infoType,
                                                                         wellNameForExport,
                                                                         segmentNumber,
                                                                         completionBranchNumber,
                                                                         maxSegmentLength,
                                                                         customSegmentIntervals,
                                                                         unitSystem,
                                                                         fishbonesContext );
    }

    // Tie-in child laterals (recursive)
    std::vector<RigMswBranch> lateralBranches;
    for ( auto* childWellPath : RicWellPathExportMswTableData::wellPathsWithTieIn( wellPath ) )
    {
        const int childOutlet =
            RicMswBranchBuilder::findOutletSegmentForMD( cellSegMap, childWellPath->wellPathTieIn()->branchValveMeasuredDepth() );
        auto childBranches = buildLateralBranches( eclipseCase,
                                                   childWellPath,
                                                   mainGrid,
                                                   childOutlet,
                                                   completionType,
                                                   exportDate,
                                                   segmentNumber,
                                                   lateralBranchNumber,
                                                   completionBranchNumber,
                                                   unitSystem,
                                                   fishbonesContext );
        lateralBranches.insert( lateralBranches.end(),
                                std::make_move_iterator( childBranches.begin() ),
                                std::make_move_iterator( childBranches.end() ) );
    }

    RigMswWellExportData result;
    result.header = header;
    result.branches.push_back( std::move( mainBoreBranch ) );
    result.branches.insert( result.branches.end(),
                            std::make_move_iterator( valveBranches.begin() ),
                            std::make_move_iterator( valveBranches.end() ) );
    result.branches.insert( result.branches.end(),
                            std::make_move_iterator( fractureBranches.begin() ),
                            std::make_move_iterator( fractureBranches.end() ) );
    result.branches.insert( result.branches.end(),
                            std::make_move_iterator( fishbonesBranches.begin() ),
                            std::make_move_iterator( fishbonesBranches.end() ) );
    result.branches.insert( result.branches.end(),
                            std::make_move_iterator( lateralBranches.begin() ),
                            std::make_move_iterator( lateralBranches.end() ) );

    RicMswBranchBuilder::applyEffectiveDiameters( fishbonesContext, result );
    RicMswBranchBuilder::applyIcdAreaPerCell( fishbonesContext, result );

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Populate RigMswTableData from a pre-built RigMswWellExportData by simple iteration over
/// RigMswSegment objects.
//--------------------------------------------------------------------------------------------------
RigMswTableData collectTableData( const RigMswWellExportData& exportData, RiaDefines::EclipseUnitSystem unitSystem )
{
    RigMswTableData tableData( exportData.header.well, unitSystem );
    tableData.setWelsegsHeader( exportData.header );

    // Collected while the segments are visited, and added to the table data ordered by branch number.
    // The branch source is kept alongside the row to resolve cells claimed by several branches.
    struct CollectedCompsegsRow
    {
        CompsegsRow        row;
        RigMswBranchSource source;
    };
    std::vector<CollectedCompsegsRow> compsegsRows;

    auto emitSegment = [&]( const RigMswBranch& branch, const RigMswSegment& seg )
    {
        // WELSEGS row
        WelsegsRow welsegsRow;
        welsegsRow.segment1       = seg.segmentNumber;
        welsegsRow.segment2       = seg.segmentNumber;
        welsegsRow.branch         = branch.branchNumber;
        welsegsRow.joinSegment    = seg.outletSegmentNumber;
        welsegsRow.length         = seg.length;
        welsegsRow.depth          = seg.depth;
        welsegsRow.diameter       = seg.diameter;
        welsegsRow.roughness      = seg.roughness;
        welsegsRow.description    = seg.description;
        welsegsRow.sourceWellName = seg.sourceWellName;
        tableData.addWelsegsRow( welsegsRow );

        // COMPSEGS rows
        for ( const auto& inter : seg.intersections )
        {
            CompsegsRow compRow;
            compRow.i             = inter.i;
            compRow.j             = inter.j;
            compRow.k             = inter.k;
            compRow.branch        = branch.branchNumber;
            compRow.distanceStart = inter.distanceStart;
            compRow.distanceEnd   = inter.distanceEnd;
            compRow.gridName      = inter.gridName;
            compsegsRows.push_back( { compRow, branch.source } );
        }

        // WSEGVALV row
        if ( seg.wsegvalvData ) tableData.addWsegvalvRow( *seg.wsegvalvData );

        // WSEGAICD row
        if ( seg.wsegaicdData ) tableData.addWsegaicdRow( *seg.wsegaicdData );

        // WSEGSICD row
        if ( seg.wsegsicdData ) tableData.addWsegsicdRow( *seg.wsegsicdData );
    };

    for ( const auto& branch : exportData.branches )
    {
        if ( branch.tieInValve ) emitSegment( branch, *branch.tieInValve );
        for ( const auto& seg : branch.segments )
            emitSegment( branch, seg );
        tableData.addMswBranch( branch );
    }

    // A grid cell has a single COMPDAT connection and must be connected to exactly one segment. When
    // several branches intersect the same cell, the branch carrying most flow claims it: perforations
    // first, then fishbones, then fractures. Visit the rows in that order and drop the later claims.
    std::vector<size_t> claimOrder( compsegsRows.size() );
    std::iota( claimOrder.begin(), claimOrder.end(), size_t( 0 ) );
    std::stable_sort( claimOrder.begin(),
                      claimOrder.end(),
                      [&]( size_t lhs, size_t rhs ) { return compsegsRows[lhs].source < compsegsRows[rhs].source; } );

    std::set<std::tuple<std::string, size_t, size_t, size_t>> connectedCells;
    std::vector<bool>                                         isCellOwner( compsegsRows.size(), false );
    for ( size_t index : claimOrder )
    {
        const auto& row    = compsegsRows[index].row;
        isCellOwner[index] = connectedCells.emplace( row.gridName, row.i, row.j, row.k ).second;
    }

    // COMPSEGS is ordered by branch number. WELSEGS keeps the order the branches are listed in, where
    // the completion branches follow the lateral they are connected to and thus are not sorted by
    // branch number. The sort is stable, so the rows of a branch keep their order along the well path.
    std::vector<CompsegsRow> ownedRows;
    for ( size_t index = 0; index < compsegsRows.size(); ++index )
    {
        if ( isCellOwner[index] ) ownedRows.push_back( compsegsRows[index].row );
    }

    std::stable_sort( ownedRows.begin(),
                      ownedRows.end(),
                      []( const CompsegsRow& lhs, const CompsegsRow& rhs ) { return lhs.branch < rhs.branch; } );

    for ( const auto& compsegsRow : ownedRows )
    {
        tableData.addCompsegsRow( compsegsRow );
    }

    return tableData;
}

} // namespace RicWellPathExportMswGeometryPath
