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

#include "RicMswBranchBuilder.h"

#include "RicExportFractureCompletionsImpl.h"
#include "RicMswTableDataTools.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFractureTemplate.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathValve.h"

#include "RiaLogging.h"

#include <algorithm>
#include <limits>

namespace RicMswBranchBuilder
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigMswCellIntersection>
    toMswCellIntersection( const WellPathCellIntersectionInfo& cellInfo, const RigMainGrid* mainGrid, double distanceStart, double distanceEnd )
{
    if ( cellInfo.globCellIndex >= mainGrid->totalCellCount() ) return std::nullopt;

    size_t             localIdx  = 0;
    const RigGridBase* localGrid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( cellInfo.globCellIndex, &localIdx );

    size_t i, j, k;
    localGrid->ijkFromCellIndex( localIdx, &i, &j, &k );
    if ( mainGrid->isDualPorosity() ) k += mainGrid->cellCountK();

    RigMswCellIntersection ci;
    ci.i             = i + 1; // 1-based
    ci.j             = j + 1;
    ci.k             = k + 1;
    ci.distanceStart = distanceStart;
    ci.distanceEnd   = distanceEnd;
    if ( localGrid != mainGrid ) ci.gridName = localGrid->gridName();
    return ci;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double segmentMidpointMD( const std::vector<CellSegmentEntry>& cellSegMap, int segmentNumber )
{
    for ( const auto& entry : cellSegMap )
    {
        if ( entry.lastSubSegmentNumber == segmentNumber ) return 0.5 * ( entry.cellStartMD + entry.cellEndMD );
    }
    return 0.0;
}

// Range-based lookup: find the segment whose [cellStartMD, cellEndMD] range contains md.
// Used for ICD valves where the valve overlap start always falls within the host main-bore cell.
// Fractures use findOutletSegmentForMD (midpoint-based) to match the tree approach.
int findContainingSegmentForMD( const std::vector<CellSegmentEntry>& cellSegMap, double md )
{
    if ( cellSegMap.empty() ) return 1;

    for ( const auto& entry : cellSegMap )
    {
        if ( md >= entry.cellStartMD && md < entry.cellEndMD ) return entry.lastSubSegmentNumber;
    }

    // Fallback: md is beyond all segments — return the last segment.
    return cellSegMap.back().lastSubSegmentNumber;
}

int findOutletSegmentForMD( const std::vector<CellSegmentEntry>& cellSegMap, double md )
{
    if ( cellSegMap.empty() ) return 1;

    // Match the tree-based logic (findClosestSegmentWithLowerMD): find the sub-segment whose
    // midpoint is closest to, but not greater than, the given MD.  Range-based lookup is
    // incorrect when a cell's midpoint lies above the completion MD (e.g. a fracture near the
    // heel of a long cell), which would assign the wrong outlet segment.
    const CellSegmentEntry* best     = nullptr;
    double                  bestDist = std::numeric_limits<double>::infinity();

    for ( const auto& entry : cellSegMap )
    {
        const double midpoint = 0.5 * ( entry.cellStartMD + entry.cellEndMD );
        const double dist     = md - midpoint;
        if ( dist >= 0.0 && dist < bestDist )
        {
            best     = &entry;
            bestDist = dist;
        }
    }

    // Fallback: md is shallower than all segment midpoints — connect to the first (shallowest) segment.
    return best ? best->lastSubSegmentNumber : cellSegMap.front().lastSubSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMswBranch buildMainBoreBranchFromGeometry( const RimWellPath*                                wellPath,
                                              const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                              const RigMainGrid*                                mainGrid,
                                              const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                              const std::set<const RimPerforationInterval*>&    valvedIntervals,
                                              const std::string&                                infoType,
                                              double                                            heelMD,
                                              double                                            heelTVD,
                                              int                                               branchNumber,
                                              int&                                              segmentNumber,
                                              int                                               outletSegmentNumber,
                                              double                                            maxSegmentLength,
                                              const std::vector<std::pair<double, double>>&     customSegmentIntervals,
                                              const std::optional<QDateTime>&                   exportDate,
                                              RiaDefines::EclipseUnitSystem                     unitSystem,
                                              std::vector<CellSegmentEntry>*                    cellSegMap,
                                              const RigActiveCellInfo*                          activeCellInfo )
{
    std::vector<RigMswSegment> result;

    const double segmentLengthThreshold = 1.0e-3;

    double prevOutMD  = heelMD;
    double prevOutTVD = heelTVD;
    int    prevSegNum = outletSegmentNumber;
    bool   firstSeg   = true;

    for ( const auto& cellInfo : filteredIntersections )
    {
        const double cellLength = std::fabs( cellInfo.endMD - cellInfo.startMD );
        if ( cellLength <= segmentLengthThreshold )
        {
            RiaLogging::info( QString( "Skipping segment, threshold = %1, length = %2" ).arg( segmentLengthThreshold ).arg( cellLength ) );
            continue;
        }

        // Collect COMPSEGS only for bare perforations (no active valve) on the main bore.
        // Valved perforations get their COMPSEGS on the valve segment.
        std::vector<RigMswCellIntersection> cellCompsegs;
        for ( const auto* perf : perforationIntervals )
        {
            if ( valvedIntervals.count( perf ) ) continue; // valve handles this interval
            if ( exportDate.has_value() && !perf->isActiveOnDate( exportDate.value() ) ) continue;

            const double overlapStart = std::max( perf->startMD(), cellInfo.startMD );
            const double overlapEnd   = std::min( perf->endMD(), cellInfo.endMD );
            if ( overlapEnd > overlapStart )
            {
                if ( activeCellInfo && cellInfo.globCellIndex < mainGrid->totalCellCount() &&
                     !activeCellInfo->isActive( ReservoirCellIndex( cellInfo.globCellIndex ) ) )
                    continue;
                if ( auto ci = toMswCellIntersection( cellInfo, mainGrid, overlapStart, overlapEnd ) ) cellCompsegs.push_back( *ci );
            }
        }

        const auto subSegs =
            RicMswTableDataTools::createSubSegmentMDPairs( cellInfo.startMD, cellInfo.endMD, maxSegmentLength, customSegmentIntervals );

        bool firstSubSeg   = true;
        int  lastSubSegNum = prevSegNum;
        for ( const auto& [subStartMD, subEndMD] : subSegs )
        {
            const double midPointMD  = 0.5 * ( subStartMD + subEndMD );
            const double midPointTVD = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, midPointMD );

            double curPrevOutMD  = prevOutMD;
            double curPrevOutTVD = prevOutTVD;
            if ( midPointMD < curPrevOutMD )
            {
                curPrevOutMD  = heelMD;
                curPrevOutTVD = heelTVD;
            }

            double length = 0.0;
            double depth  = 0.0;
            if ( infoType == "INC" )
            {
                length = midPointMD - curPrevOutMD;
                depth  = midPointTVD - curPrevOutTVD;
            }
            else
            {
                length = midPointMD;
                depth  = midPointTVD;
            }

            double diameter  = 0.0;
            double roughness = 0.0;
            if ( exportDate.has_value() )
            {
                diameter  = wellPath->mswCompletionParameters()->getDiameterAtMD( midPointMD, unitSystem, *exportDate );
                roughness = wellPath->mswCompletionParameters()->getRoughnessAtMD( midPointMD, unitSystem, *exportDate );
            }
            else
            {
                diameter  = wellPath->mswCompletionParameters()->getDiameterAtMD( midPointMD, unitSystem );
                roughness = wellPath->mswCompletionParameters()->getRoughnessAtMD( midPointMD, unitSystem );
            }

            RigMswSegment seg;
            seg.segmentNumber       = segmentNumber;
            seg.outletSegmentNumber = prevSegNum;
            seg.length              = length;
            seg.depth               = depth;
            seg.diameter            = diameter;
            seg.roughness           = roughness;
            seg.sourceWellName      = wellPath->name().toStdString();
            if ( firstSeg && firstSubSeg )
            {
                seg.description = "Segments on main bore";
                firstSeg        = false;
            }
            if ( firstSubSeg ) seg.intersections = cellCompsegs;

            lastSubSegNum = segmentNumber;
            prevSegNum    = segmentNumber++;
            prevOutMD     = midPointMD;
            prevOutTVD    = midPointTVD;
            firstSubSeg   = false;
            if ( cellSegMap ) cellSegMap->push_back( { subStartMD, subEndMD, lastSubSegNum } );
            result.push_back( std::move( seg ) );
        }
    }

    return RigMswBranch{ branchNumber, std::nullopt, std::move( result ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildValveBranchesFromGeometry( const RimWellPath*                                wellPath,
                                                          const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                                          const RigMainGrid*                                mainGrid,
                                                          const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                          const std::vector<CellSegmentEntry>&              cellSegMap,
                                                          const std::string&                                infoType,
                                                          const std::string&                                wellNameForExport,
                                                          int&                                              segmentNumber,
                                                          int&                                              branchNumber,
                                                          double                                            maxSegmentLength,
                                                          const std::vector<std::pair<double, double>>&     customSegmentIntervals,
                                                          const std::optional<QDateTime>&                   exportDate,
                                                          RiaDefines::EclipseUnitSystem                     unitSystem )
{
    std::vector<RigMswBranch> result;

    const auto linerDiameter   = wellPath->mswCompletionParameters()->linerDiameter( unitSystem );
    const auto roughnessFactor = wellPath->mswCompletionParameters()->roughnessFactor( unitSystem );

    for ( const auto* perf : perforationIntervals )
    {
        if ( exportDate.has_value() && !perf->isActiveOnDate( exportDate.value() ) ) continue;

        auto valves = perf->descendantsIncludingThisOfType<RimWellPathValve>();
        for ( const auto* valve : valves )
        {
            if ( !valve->isChecked() ) continue;
            if ( exportDate.has_value() && !valve->isActiveOnDate( exportDate.value() ) ) continue;

            const auto valveType = valve->componentType();
            const bool isIcdOrIcv =
                ( valveType == RiaDefines::WellPathComponentType::ICD || valveType == RiaDefines::WellPathComponentType::ICV );
            const bool isAicdOrSicd =
                ( valveType == RiaDefines::WellPathComponentType::AICD || valveType == RiaDefines::WellPathComponentType::SICD );
            if ( !isIcdOrIcv && !isAicdOrSicd ) continue;

            const auto& valveLocations = valve->valveLocations();
            const auto& valveSegs      = valve->valveSegments(); // one (startMD, endMD) per location

            // ICD: one branch per overlapping cell with proportional area, matching tree approach
            if ( valveType == RiaDefines::WellPathComponentType::ICD )
            {
                for ( size_t vi = 0; vi < valveLocations.size(); ++vi )
                {
                    const double valveSegStart = valveSegs[vi].first;
                    const double valveSegEnd   = valveSegs[vi].second;

                    // Collect cells overlapping this ICD's coverage range and total overlap length
                    struct CellEntry
                    {
                        RigMswCellIntersection ci;
                        double                 overlapMD;
                    };
                    std::vector<CellEntry> cells;
                    double                 totalOverlapMD = 0.0;
                    for ( const auto& cellInfo : filteredIntersections )
                    {
                        const double overlapStart = std::max( valveSegStart, cellInfo.startMD );
                        const double overlapEnd   = std::min( valveSegEnd, cellInfo.endMD );
                        if ( overlapEnd <= overlapStart ) continue;
                        if ( auto ci = toMswCellIntersection( cellInfo, mainGrid, overlapStart, overlapEnd ) )
                        {
                            double overlapMD = overlapEnd - overlapStart;
                            cells.push_back( { *ci, overlapMD } );
                            totalOverlapMD += overlapMD;
                        }
                    }

                    // One branch+segment per cell with area proportional to overlap length
                    for ( auto& entry : cells )
                    {
                        const int    cellBranch = ++branchNumber;
                        const int    outletSeg  = findContainingSegmentForMD( cellSegMap, entry.ci.distanceStart );
                        const double valveMD    = segmentMidpointMD( cellSegMap, outletSeg );
                        const double valveEndMD = valveMD + RicMswTableDataTools::valveSegmentLength;
                        const double startTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveMD );
                        const double endTVD     = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveEndMD );

                        double length = 0.0, depth = 0.0;
                        if ( infoType == "INC" )
                        {
                            length = valveEndMD - valveMD;
                            depth  = endTVD - startTVD;
                        }
                        else
                        {
                            length = valveEndMD;
                            depth  = endTVD;
                        }

                        RigMswSegment seg;
                        seg.segmentNumber       = segmentNumber;
                        seg.outletSegmentNumber = outletSeg;
                        seg.length              = length;
                        seg.depth               = depth;
                        seg.diameter            = linerDiameter;
                        seg.roughness           = roughnessFactor;
                        seg.sourceWellName      = wellPath->name().toStdString();
                        seg.description         = QString( "%1 #%2" ).arg( valve->name() ).arg( vi + 1 ).toStdString();
                        seg.intersections       = { entry.ci };

                        WsegvalvRow wv;
                        wv.well          = wellNameForExport;
                        wv.segmentNumber = segmentNumber;
                        wv.cv            = valve->flowCoefficient();
                        wv.area          = totalOverlapMD > 0.0 ? valve->area( unitSystem ) * entry.overlapMD / totalOverlapMD : 0.0;
                        wv.status        = valve->isOpen() ? "OPEN" : "SHUT";
                        wv.description   = valve->name().toStdString();
                        seg.wsegvalvData = wv;

                        segmentNumber++;
                        result.push_back( RigMswBranch{ cellBranch, std::nullopt, { std::move( seg ) } } );
                    }
                }
                continue;
            }

            // AICD: one branch per overlapping cell with proportional flow scaling factor, matching tree approach
            if ( valveType == RiaDefines::WellPathComponentType::AICD )
            {
                const RimWellPathAicdParameters* aicdParams = valve->aicdParameters();
                if ( aicdParams && aicdParams->isValid() )
                {
                    const auto aicdValues = aicdParams->doubleValues();

                    auto setOptional = []( double value ) -> std::optional<double>
                    { return std::isinf( value ) ? std::nullopt : std::optional<double>{ value }; };

                    for ( size_t vi = 0; vi < valveLocations.size(); ++vi )
                    {
                        const double valveSegStart = valveSegs[vi].first;
                        const double valveSegEnd   = valveSegs[vi].second;

                        struct CellEntry
                        {
                            RigMswCellIntersection ci;
                            double                 overlapMD;
                        };
                        std::vector<CellEntry> cells;
                        double                 totalOverlapMD = 0.0;
                        for ( const auto& cellInfo : filteredIntersections )
                        {
                            const double overlapStart = std::max( valveSegStart, cellInfo.startMD );
                            const double overlapEnd   = std::min( valveSegEnd, cellInfo.endMD );
                            if ( overlapEnd <= overlapStart ) continue;
                            if ( auto ci = toMswCellIntersection( cellInfo, mainGrid, overlapStart, overlapEnd ) )
                            {
                                cells.push_back( { *ci, overlapEnd - overlapStart } );
                                totalOverlapMD += overlapEnd - overlapStart;
                            }
                        }

                        for ( auto& entry : cells )
                        {
                            const int    cellBranch = ++branchNumber;
                            const int    outletSeg  = findContainingSegmentForMD( cellSegMap, entry.ci.distanceStart );
                            const double valveMD    = segmentMidpointMD( cellSegMap, outletSeg );
                            const double valveEndMD = valveMD + RicMswTableDataTools::valveSegmentLength;
                            const double startTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveMD );
                            const double endTVD     = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, valveEndMD );

                            double length = 0.0, depth = 0.0;
                            if ( infoType == "INC" )
                            {
                                length = valveEndMD - valveMD;
                                depth  = endTVD - startTVD;
                            }
                            else
                            {
                                length = valveEndMD;
                                depth  = endTVD;
                            }

                            RigMswSegment seg;
                            seg.segmentNumber       = segmentNumber;
                            seg.outletSegmentNumber = outletSeg;
                            seg.length              = length;
                            seg.depth               = depth;
                            seg.diameter            = linerDiameter;
                            seg.roughness           = roughnessFactor;
                            seg.sourceWellName      = wellPath->name().toStdString();
                            seg.description         = QString( "%1 #%2" ).arg( valve->name() ).arg( vi + 1 ).toStdString();
                            seg.intersections       = { entry.ci };

                            WsegaicdRow wa;
                            wa.well                = wellNameForExport;
                            wa.segment1            = segmentNumber;
                            wa.segment2            = segmentNumber;
                            wa.strength            = aicdValues[AICD_STRENGTH];
                            wa.length              = totalOverlapMD > 0.0 ? totalOverlapMD / entry.overlapMD : 0.0;
                            wa.densityCali         = setOptional( aicdValues[AICD_DENSITY_CALIB_FLUID] );
                            wa.viscosityCali       = setOptional( aicdValues[AICD_VISCOSITY_CALIB_FLUID] );
                            wa.criticalValue       = setOptional( aicdValues[AICD_CRITICAL_WATER_IN_LIQUID_FRAC] );
                            wa.widthTrans          = setOptional( aicdValues[AICD_EMULSION_VISC_TRANS_REGION] );
                            wa.maxViscRatio        = setOptional( aicdValues[AICD_MAX_RATIO_EMULSION_VISC] );
                            wa.methodScalingFactor = 1;
                            wa.maxAbsRate          = aicdValues[AICD_MAX_FLOW_RATE];
                            wa.flowRateExponent    = aicdValues[AICD_VOL_FLOW_EXP];
                            wa.viscExponent        = aicdValues[AICD_VISOSITY_FUNC_EXP];
                            wa.status              = aicdParams->isOpen() ? "OPEN" : "SHUT";
                            wa.description         = valve->name().toStdString();
                            seg.wsegaicdData       = wa;

                            segmentNumber++;
                            result.push_back( RigMswBranch{ cellBranch, std::nullopt, { std::move( seg ) } } );
                        }
                    }
                }
                continue;
            }

            // ICV: one branch, one segment per valve location
            const int valveBranch = ++branchNumber;

            std::vector<RigMswSegment> branchSegs;
            for ( size_t vi = 0; vi < valveLocations.size(); ++vi )
            {
                const double valveMD       = valveLocations[vi];
                const double valveEndMD    = valveMD + RicMswTableDataTools::valveSegmentLength;
                const double valveSegStart = valveSegs[vi].first;
                const double valveSegEnd   = valveSegs[vi].second;

                const int outletSeg = findOutletSegmentForMD( cellSegMap, valveMD );

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

                RigMswSegment valveSeg;
                valveSeg.segmentNumber       = segmentNumber;
                valveSeg.outletSegmentNumber = outletSeg;
                valveSeg.length              = length;
                valveSeg.depth               = depth;
                valveSeg.diameter            = linerDiameter;
                valveSeg.roughness           = roughnessFactor;
                valveSeg.sourceWellName      = wellPath->name().toStdString();
                valveSeg.description         = QString( "%1 #%2" ).arg( valve->name() ).arg( vi + 1 ).toStdString();

                // COMPSEGS: cells that overlap this valve's coverage range (valveSegStart..valveSegEnd)
                for ( const auto& cellInfo : filteredIntersections )
                {
                    const double overlapStart = std::max( valveSegStart, cellInfo.startMD );
                    const double overlapEnd   = std::min( valveSegEnd, cellInfo.endMD );
                    if ( overlapEnd > overlapStart )
                    {
                        if ( auto ci = toMswCellIntersection( cellInfo, mainGrid, overlapStart, overlapEnd ) )
                            valveSeg.intersections.push_back( *ci );
                    }
                }

                // WSEGVALV for ICV
                WsegvalvRow wv;
                wv.well               = wellNameForExport;
                wv.segmentNumber      = segmentNumber;
                wv.cv                 = valve->flowCoefficient();
                wv.area               = valve->area( unitSystem );
                wv.status             = valve->isOpen() ? "OPEN" : "SHUT";
                wv.description        = valve->name().toStdString();
                valveSeg.wsegvalvData = wv;

                segmentNumber++;
                branchSegs.push_back( std::move( valveSeg ) );
            }
            result.push_back( RigMswBranch{ valveBranch, std::nullopt, std::move( branchSegs ) } );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildFractureBranchesFromGeometry( RimEclipseCase*                      eclipseCase,
                                                             const RimWellPath*                   wellPath,
                                                             const RigMainGrid*                   mainGrid,
                                                             const std::vector<CellSegmentEntry>& cellSegMap,
                                                             const std::string&                   infoType,
                                                             int&                                 segmentNumber,
                                                             int&                                 branchNumber )
{
    std::vector<RigMswBranch> result;

    const QString wellNameForExport = wellPath->completionSettings()->wellNameForExport();

    for ( RimWellPathFracture* fracture : wellPath->fractureCollection()->activeFractures() )
    {
        fracture->ensureValidNonDarcyProperties();

        double position = fracture->fractureMD();
        double width    = fracture->fractureTemplate()->computeFractureWidth( fracture );

        if ( fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
        {
            double perforationLength = fracture->fractureTemplate()->perforationLength();
            position -= 0.5 * perforationLength;
            width = perforationLength;
        }

        const double endMD    = position + width;
        const double startTVD = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, position );
        const double endTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, endMD );

        double length = 0.0;
        double depth  = 0.0;
        if ( infoType == "INC" )
        {
            length = width;
            depth  = endTVD - startTVD;
        }
        else
        {
            length = endMD;
            depth  = endTVD;
        }

        const int outletSeg  = findOutletSegmentForMD( cellSegMap, position );
        const int fracBranch = ++branchNumber;

        // Get COMPSEGS cell intersections from fracture completion data
        std::vector<RigCompletionData> completionData = RicExportFractureCompletionsImpl::generateCompdatValues( eclipseCase,
                                                                                                                 wellNameForExport,
                                                                                                                 wellPath->wellPathGeometry(),
                                                                                                                 { fracture },
                                                                                                                 nullptr,
                                                                                                                 nullptr );

        std::vector<RigMswCellIntersection> compsegs;
        for ( const auto& compData : completionData )
        {
            const auto& cell = compData.completionDataGridCell();

            // 0-based -> 1-based
            size_t i = cell.localCellIndexI() + 1;
            size_t j = cell.localCellIndexJ() + 1;
            size_t k = cell.localCellIndexK() + 1;

            // Shift K for dual porosity models
            if ( mainGrid->isDualPorosity() ) k += mainGrid->cellCountK();

            RigMswCellIntersection ci;
            ci.i             = i;
            ci.j             = j;
            ci.k             = k;
            ci.distanceStart = position;
            ci.distanceEnd   = endMD;
            ci.gridName      = cell.lgrName().toStdString();
            compsegs.push_back( ci );
        }

        RigMswSegment seg;
        seg.segmentNumber       = segmentNumber++;
        seg.outletSegmentNumber = outletSeg;
        seg.length              = length;
        seg.depth               = depth;
        seg.diameter            = 0.15; // RicMswSegment default (tree approach uses same value)
        seg.roughness           = 5.0e-5; // RicMswSegment default (tree approach uses same value)
        seg.sourceWellName      = wellPath->name().toStdString();
        seg.description         = fracture->name().toStdString();
        seg.intersections       = std::move( compsegs );

        result.push_back( RigMswBranch{ fracBranch, std::nullopt, { std::move( seg ) } } );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildFishbonesBranchesFromGeometry( const RimEclipseCase*                            eclipseCase,
                                                              const RimWellPath*                               wellPath,
                                                              const RigMainGrid*                               mainGrid,
                                                              const std::vector<WellPathCellIntersectionInfo>& filteredIntersections,
                                                              const std::vector<CellSegmentEntry>&             cellSegMap,
                                                              const std::string&                               infoType,
                                                              const std::string&                               wellNameForExport,
                                                              int&                                             segmentNumber,
                                                              int&                                             branchNumber,
                                                              RiaDefines::EclipseUnitSystem                    unitSystem )
{
    std::vector<RigMswBranch> result;

    auto fishbonesSubs = wellPath->completions()->fishbonesCollection()->activeFishbonesSubs();
    for ( const RimFishbones* subs : fishbonesSubs )
    {
        // Group laterals by sub index
        std::map<size_t, std::vector<size_t>> subAndLateralIndices;
        for ( const auto& [subIndex, lateralIndex] : subs->installedLateralIndices() )
            subAndLateralIndices[subIndex].push_back( lateralIndex );

        // Identify which filtered intersections are closest to each sub, and which
        // intersection contains the sub's MD.  This mirrors appendFishbonesMswExportInfo.
        const double fishboneSectionStart = subs->startMD();
        const double fishboneSectionEnd   = subs->endMD();

        std::map<size_t, std::vector<size_t>> closestSubForCellIntersections;
        std::map<size_t, size_t>              cellIntersectionContainingSubIndex;

        for ( size_t ii = 0; ii < filteredIntersections.size(); ++ii )
        {
            const auto& ci = filteredIntersections[ii];
            if ( fishboneSectionEnd < ci.startMD || fishboneSectionStart > ci.endMD ) continue;

            const double midpoint      = 0.5 * ( ci.startMD + ci.endMD );
            size_t       closestSubIdx = 0;
            double       closestDist   = std::numeric_limits<double>::infinity();

            for ( const auto& [subIdx, lats] : subAndLateralIndices )
            {
                const double subMD = subs->measuredDepth( subIdx );
                if ( ci.startMD <= subMD && subMD <= ci.endMD ) cellIntersectionContainingSubIndex[subIdx] = ii;

                const double dist = std::abs( subMD - midpoint );
                if ( dist < closestDist )
                {
                    closestDist   = dist;
                    closestSubIdx = subIdx;
                }
            }
            closestSubForCellIntersections[closestSubIdx].push_back( ii );
        }

        // ICD parameters
        const double icdOrificeRadius = subs->icdOrificeDiameter( unitSystem ) / 2.0;
        const double icdArea          = icdOrificeRadius * icdOrificeRadius * cvf::PI_D * static_cast<double>( subs->icdCount() );
        const double icdCv            = subs->icdFlowCoefficient();

        // Track used global cell indices to deduplicate COMPSEGS, matching the tree's intersectedCells logic.
        std::set<size_t> usedGlobalCellIndices;

        for ( const auto& [subIndex, lateralIndices] : subAndLateralIndices )
        {
            const double subEndMd      = subs->measuredDepth( subIndex );
            const double startValveMd  = subEndMd - RicMswTableDataTools::valveSegmentLength;
            const double startValveTVD = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, startValveMd );
            const double endValveTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( wellPath, subEndMd );

            const int outletSeg = findOutletSegmentForMD( cellSegMap, subEndMd );
            const int icdBranch = ++branchNumber;
            const int icdSegNum = segmentNumber++;

            // COMPSEGS for the ICD segment: cells closest to this sub, clipped to [startValveMd, subEndMd]
            std::set<size_t> icdCellIndices;
            if ( closestSubForCellIntersections.count( subIndex ) )
            {
                for ( auto idx : closestSubForCellIntersections[subIndex] )
                    icdCellIndices.insert( idx );
            }
            if ( cellIntersectionContainingSubIndex.count( subIndex ) )
                icdCellIndices.insert( cellIntersectionContainingSubIndex[subIndex] );

            std::vector<RigMswCellIntersection> icdCompsegs;
            for ( auto idx : icdCellIndices )
            {
                const auto&  ci           = filteredIntersections[idx];
                const double overlapStart = std::max( startValveMd, ci.startMD );
                const double overlapEnd   = std::min( subEndMd, ci.endMD );
                if ( overlapEnd <= overlapStart ) continue;
                if ( auto mci = toMswCellIntersection( ci, mainGrid, overlapStart, overlapEnd ) )
                {
                    icdCompsegs.push_back( *mci );
                    usedGlobalCellIndices.insert( ci.globCellIndex );
                }
            }

            double icdLength = 0.0;
            double icdDepth  = 0.0;
            if ( infoType == "INC" )
            {
                icdLength = subEndMd - startValveMd;
                icdDepth  = endValveTVD - startValveTVD;
            }
            else
            {
                icdLength = subEndMd;
                icdDepth  = endValveTVD;
            }

            RigMswSegment icdSeg;
            icdSeg.segmentNumber       = icdSegNum;
            icdSeg.outletSegmentNumber = outletSeg;
            icdSeg.length              = icdLength;
            icdSeg.depth               = icdDepth;
            // Use RigMswSegment defaults for diameter/roughness (matching tree approach for fishbones ICD)
            icdSeg.sourceWellName = wellPath->name().toStdString();
            icdSeg.description    = QString( "ICD sub %1" ).arg( subIndex + 1 ).toStdString();
            icdSeg.intersections  = std::move( icdCompsegs );

            WsegvalvRow wv;
            wv.well             = wellNameForExport;
            wv.segmentNumber    = icdSegNum;
            wv.cv               = icdCv;
            wv.area             = icdArea;
            wv.status           = "OPEN";
            wv.description      = QString( "ICD sub %1" ).arg( subIndex + 1 ).toStdString();
            icdSeg.wsegvalvData = wv;

            result.push_back( RigMswBranch{ icdBranch, std::nullopt, { std::move( icdSeg ) } } );

            // Lateral sub-segments
            for ( size_t lateralIndex : lateralIndices )
            {
                auto lateralCoordMDPairs = subs->coordsAndMDForLateral( subIndex, lateralIndex );
                if ( lateralCoordMDPairs.empty() ) continue;

                std::vector<cvf::Vec3d> lateralCoords;
                std::vector<double>     lateralMDs;
                for ( auto& [coord, md] : lateralCoordMDPairs )
                {
                    lateralCoords.push_back( coord );
                    lateralMDs.push_back( md );
                }

                auto lateralIntersections = RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                                                              wellPath->name(),
                                                                                                              lateralCoords,
                                                                                                              lateralMDs );

                if ( lateralIntersections.empty() ) continue;

                const int  latBranch    = ++branchNumber;
                double     prevMD       = lateralMDs.front();
                double     prevTVD      = -lateralCoords.front().z();
                int        latOutletSeg = icdSegNum;
                bool       firstLatSeg  = true;
                const auto lateralLabel = QString( "Lateral %1" ).arg( lateralIndex + 1 ).toStdString();

                std::vector<RigMswSegment> latSegs;
                for ( const auto& cellIntInfo : lateralIntersections )
                {
                    if ( auto mci = toMswCellIntersection( cellIntInfo, mainGrid, cellIntInfo.startMD, cellIntInfo.endMD ) )
                    {
                        double len = 0.0;
                        double dep = 0.0;
                        if ( infoType == "INC" )
                        {
                            len = cellIntInfo.endMD - prevMD;
                            dep = cellIntInfo.endTVD() - prevTVD;
                        }
                        else
                        {
                            len = cellIntInfo.endMD;
                            dep = cellIntInfo.endTVD();
                        }

                        // Deduplicate COMPSEGS: skip cells already connected (matching tree intersectedCells logic)
                        const bool isNewCell = usedGlobalCellIndices.insert( cellIntInfo.globCellIndex ).second;

                        RigMswSegment latSeg;
                        latSeg.segmentNumber       = segmentNumber++;
                        latSeg.outletSegmentNumber = latOutletSeg;
                        latSeg.length              = len;
                        latSeg.depth               = dep;
                        latSeg.diameter            = subs->equivalentDiameter( unitSystem );
                        latSeg.roughness           = subs->openHoleRoughnessFactor( unitSystem );
                        latSeg.sourceWellName      = wellPath->name().toStdString();
                        if ( firstLatSeg ) latSeg.description = lateralLabel;
                        latSeg.intersections = isNewCell ? std::vector<RigMswCellIntersection>{ *mci } : std::vector<RigMswCellIntersection>{};

                        latOutletSeg = latSeg.segmentNumber;
                        prevMD       = cellIntInfo.endMD;
                        prevTVD      = cellIntInfo.endTVD();
                        firstLatSeg  = false;
                        latSegs.push_back( std::move( latSeg ) );
                    }
                }
                result.push_back( RigMswBranch{ latBranch, std::nullopt, std::move( latSegs ) } );
            }
        }
    }

    return result;
}

} // namespace RicMswBranchBuilder
