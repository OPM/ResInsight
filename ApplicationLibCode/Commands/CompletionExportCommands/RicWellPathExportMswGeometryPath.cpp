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

#include "RicMswTableDataTools.h"
#include "RicWellPathExportMswBuildSegments.h"
#include "RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"

#include "RigEclipseCaseData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathTieIn.h"
#include "RimWellPathValve.h"

#include <algorithm>

namespace RicWellPathExportMswGeometryPath
{

using CompletionType = RicWellPathExportMswTableData::CompletionType;

//--------------------------------------------------------------------------------------------------
/// Recursively build all WELSEGS/COMPSEGS/valve segments for one lateral (child well path)
/// and any of its own child laterals.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranchExportData> buildLateralSegments( RimEclipseCase*                 eclipseCase,
                                                          const RimWellPath*              wellPath,
                                                          const RigMainGrid*              mainGrid,
                                                          int                             outletSegNum,
                                                          CompletionType                  completionType,
                                                          const std::optional<QDateTime>& exportDate,
                                                          int&                            segmentNumber,
                                                          int&                            branchNumber,
                                                          RiaDefines::EclipseUnitSystem   unitSystem )
{
    std::vector<RigMswBranchExportData> result;
    auto                                mswParameters = wellPath->mswCompletionParameters();
    if ( !mswParameters ) return result;

    const std::string infoType          = mswParameters->lengthAndDepth().text().toStdString();
    const double      tieInMD           = wellPath->wellPathTieIn()->tieInMeasuredDepth();
    const double      tieInTVD          = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( tieInMD ).z();
    const std::string wellNameForExport = wellPath->completionSettings()->wellNameForExport().toStdString();

    const int lateralBranchNum = ++branchNumber;
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
            const double valveEndMD    = tieInMD + offset;
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

    std::vector<RicWellPathExportMswBuildSegments::CellSegmentEntry> childCellSegMap;

    auto lateralBranch       = RicWellPathExportMswBuildSegments::buildMainBoreSegmentsFromGeometry( wellPath,
                                                                                               filteredIntersections,
                                                                                               mainGrid,
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
                                                                                               &childCellSegMap );
    lateralBranch.tieInValve = std::move( tieInValve );
    result.push_back( std::move( lateralBranch ) );

    auto valveBranches = RicWellPathExportMswBuildSegments::buildValveSegmentsFromGeometry( wellPath,
                                                                                            filteredIntersections,
                                                                                            mainGrid,
                                                                                            perforationIntervals,
                                                                                            childCellSegMap,
                                                                                            infoType,
                                                                                            wellNameForExport,
                                                                                            segmentNumber,
                                                                                            branchNumber,
                                                                                            mswParameters->maxSegmentLength(),
                                                                                            {},
                                                                                            exportDate,
                                                                                            unitSystem );
    result.insert( result.end(), std::make_move_iterator( valveBranches.begin() ), std::make_move_iterator( valveBranches.end() ) );

    if ( ( completionType & CompletionType::FRACTURES ) == CompletionType::FRACTURES )
    {
        auto fracBranches = RicWellPathExportMswBuildSegments::buildFractureSegmentsFromGeometry( eclipseCase,
                                                                                                  wellPath,
                                                                                                  mainGrid,
                                                                                                  childCellSegMap,
                                                                                                  infoType,
                                                                                                  segmentNumber,
                                                                                                  branchNumber );
        result.insert( result.end(), std::make_move_iterator( fracBranches.begin() ), std::make_move_iterator( fracBranches.end() ) );
    }

    if ( ( completionType & CompletionType::FISHBONES ) == CompletionType::FISHBONES )
    {
        auto fishBranches = RicWellPathExportMswBuildSegments::buildFishbonesSegmentsFromGeometry( eclipseCase,
                                                                                                   wellPath,
                                                                                                   mainGrid,
                                                                                                   filteredIntersections,
                                                                                                   childCellSegMap,
                                                                                                   infoType,
                                                                                                   wellNameForExport,
                                                                                                   segmentNumber,
                                                                                                   branchNumber,
                                                                                                   unitSystem );
        result.insert( result.end(), std::make_move_iterator( fishBranches.begin() ), std::make_move_iterator( fishBranches.end() ) );
    }

    // Recurse into grandchildren
    for ( auto* grandchild : RicWellPathExportMswTableData::wellPathsWithTieIn( wellPath ) )
    {
        const int grandchildOutlet =
            RicWellPathExportMswBuildSegments::findOutletSegmentForMD( childCellSegMap, grandchild->wellPathTieIn()->tieInMeasuredDepth() );
        auto grandchildBranches =
            buildLateralSegments( eclipseCase, grandchild, mainGrid, grandchildOutlet, completionType, exportDate, segmentNumber, branchNumber, unitSystem );
        result.insert( result.end(), std::make_move_iterator( grandchildBranches.begin() ), std::make_move_iterator( grandchildBranches.end() ) );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Build the flat MSW export data directly from well-path geometry and Rim completion objects,
/// without building the RicMswBranch / RicMswItem tree.
///
/// Currently implemented: main-bore WELSEGS segments + perforation COMPSEGS entries.
/// TODO: valve completions (ICD/AICD/SICD/ICV), fishbones laterals, fractures, tie-in wells.
//--------------------------------------------------------------------------------------------------
RigMswFlatExportData buildMswFromGeometry( RimEclipseCase*                               eclipseCase,
                                           const RimWellPath*                            wellPath,
                                           double                                        maxSegmentLength,
                                           const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                           CompletionType                                completionType,
                                           const std::optional<QDateTime>&               exportDate )
{
    auto mswParameters = wellPath->mswCompletionParameters();
    CVF_ASSERT( mswParameters );

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

    int                                                              segmentNumber = 2; // Segment 1 is the implicit well heel.
    int                                                              branchNumber  = 1; // Incremented for each new branch.
    std::vector<RicWellPathExportMswBuildSegments::CellSegmentEntry> cellSegMap;

    auto mainBoreBranch = RicWellPathExportMswBuildSegments::buildMainBoreSegmentsFromGeometry( wellPath,
                                                                                                filteredIntersections,
                                                                                                mainGrid,
                                                                                                perforationIntervals,
                                                                                                valvedIntervals,
                                                                                                infoType,
                                                                                                initialMD,
                                                                                                initialTVD,
                                                                                                branchNumber,
                                                                                                segmentNumber,
                                                                                                1, // outlet = heel (segment 1)
                                                                                                maxSegmentLength,
                                                                                                customSegmentIntervals,
                                                                                                exportDate,
                                                                                                unitSystem,
                                                                                                &cellSegMap );

    const std::string wellNameForExport = wellPath->completionSettings()->wellNameForExport().toStdString();

    auto valveBranches = RicWellPathExportMswBuildSegments::buildValveSegmentsFromGeometry( wellPath,
                                                                                            filteredIntersections,
                                                                                            mainGrid,
                                                                                            perforationIntervals,
                                                                                            cellSegMap,
                                                                                            infoType,
                                                                                            wellNameForExport,
                                                                                            segmentNumber,
                                                                                            branchNumber,
                                                                                            maxSegmentLength,
                                                                                            customSegmentIntervals,
                                                                                            exportDate,
                                                                                            unitSystem );

    const bool                          includeFractures = ( completionType & CompletionType::FRACTURES ) == CompletionType::FRACTURES;
    std::vector<RigMswBranchExportData> fractureBranches;
    if ( includeFractures )
    {
        fractureBranches = RicWellPathExportMswBuildSegments::buildFractureSegmentsFromGeometry( eclipseCase,
                                                                                                 wellPath,
                                                                                                 mainGrid,
                                                                                                 cellSegMap,
                                                                                                 infoType,
                                                                                                 segmentNumber,
                                                                                                 branchNumber );
    }

    const bool                          includeFishbones = ( completionType & CompletionType::FISHBONES ) == CompletionType::FISHBONES;
    std::vector<RigMswBranchExportData> fishbonesBranches;
    if ( includeFishbones )
    {
        fishbonesBranches = RicWellPathExportMswBuildSegments::buildFishbonesSegmentsFromGeometry( eclipseCase,
                                                                                                   wellPath,
                                                                                                   mainGrid,
                                                                                                   filteredIntersections,
                                                                                                   cellSegMap,
                                                                                                   infoType,
                                                                                                   wellNameForExport,
                                                                                                   segmentNumber,
                                                                                                   branchNumber,
                                                                                                   unitSystem );
    }

    // Tie-in child laterals (recursive)
    std::vector<RigMswBranchExportData> lateralBranches;
    for ( auto* childWellPath : RicWellPathExportMswTableData::wellPathsWithTieIn( wellPath ) )
    {
        const int childOutlet =
            RicWellPathExportMswBuildSegments::findOutletSegmentForMD( cellSegMap, childWellPath->wellPathTieIn()->tieInMeasuredDepth() );
        auto childBranches =
            buildLateralSegments( eclipseCase, childWellPath, mainGrid, childOutlet, completionType, exportDate, segmentNumber, branchNumber, unitSystem );
        lateralBranches.insert( lateralBranches.end(),
                                std::make_move_iterator( childBranches.begin() ),
                                std::make_move_iterator( childBranches.end() ) );
    }

    RigMswFlatExportData result;
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
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Populate RigMswTableData from a pre-built flat segment list.
/// This replaces the recursive tree-based collection functions (collectWelsegsData etc.) with
/// simple iteration over RigMswSegment objects.
//--------------------------------------------------------------------------------------------------
RigMswTableData collectDataFromFlatList( const RigMswFlatExportData& exportData, RiaDefines::EclipseUnitSystem unitSystem )
{
    RigMswTableData tableData( exportData.header.well, unitSystem );
    tableData.setWelsegsHeader( exportData.header );

    auto emitSegment = [&]( const RigMswBranchExportData& branch, const RigMswSegment& seg )
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
            tableData.addCompsegsRow( compRow );
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
    return tableData;
}

} // namespace RicWellPathExportMswGeometryPath
