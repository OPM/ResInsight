/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicMswTableDataTools.h"

#include "RiaLogging.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "RicMswCompletions.h"
#include "RicMswExportInfo.h"

#include "Well/RigWellPath.h"

#include "RimMswCompletionParameters.h"
#include "RimPerforationInterval.h"
#include "RimValveCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathValve.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsData( RigMswTableData&                              tableData,
                                               RicMswExportInfo&                             exportInfo,
                                               double                                        maxSegmentLength,
                                               const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                               bool                                          exportCompletionSegmentsAfterMainBore,
                                               const std::optional<QDateTime>&               exportDate )
{
    // Set up WELSEGS header
    WelsegsHeader header;
    header.well      = exportInfo.mainBoreBranch()->wellPath()->completionSettings()->wellNameForExport().toStdString();
    header.topLength = exportInfo.mainBoreBranch()->startMD();
    header.topDepth  = exportInfo.mainBoreBranch()->startTVD();

    if ( exportInfo.topWellBoreVolume() != RicMswExportInfo::defaultDoubleValue() )
    {
        header.wellboreVolume = exportInfo.topWellBoreVolume();
    }
    header.infoType           = exportInfo.lengthAndDepthText().toStdString();
    header.pressureComponents = exportInfo.pressureDropText().toStdString();

    tableData.setWelsegsHeader( header );

    int segmentNumber = 2; // There's an implicit segment number 1.

    // Collect segment data recursively
    collectWelsegsDataRecursively( tableData,
                                   exportInfo,
                                   exportInfo.mainBoreBranch(),
                                   &segmentNumber,
                                   maxSegmentLength,
                                   customSegmentIntervals,
                                   exportCompletionSegmentsAfterMainBore,
                                   nullptr,
                                   exportDate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsDataRecursively( RigMswTableData&                              tableData,
                                                          RicMswExportInfo&                             exportInfo,
                                                          gsl::not_null<RicMswBranch*>                  branch,
                                                          gsl::not_null<int*>                           segmentNumber,
                                                          double                                        maxSegmentLength,
                                                          const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                                          bool                            exportCompletionSegmentsAfterMainBore,
                                                          RicMswSegment*                  connectedToSegment,
                                                          const std::optional<QDateTime>& exportDate )
{
    auto         outletSegment = connectedToSegment;
    RicMswValve* outletValve   = nullptr;

    auto branchSegments = branch->segments();
    auto it             = branchSegments.begin();

    // Handle tie-in ICV at the beginning of branch
    if ( outletValve = dynamic_cast<RicMswTieInICV*>( branch.get() ); outletValve != nullptr )
    {
        collectValveWelsegsSegment( tableData, outletSegment, outletValve, exportInfo, maxSegmentLength, customSegmentIntervals, segmentNumber );

        auto valveSegments = outletValve->segments();
        outletSegment      = valveSegments.front();

        *segmentNumber = outletSegment->segmentNumber() + 1;
        ++it; // skip segment below
    }

    auto branchStartSegmentIterator = it;
    for ( ; it != branchSegments.end(); ++it )
    {
        auto segment = *it;
        segment->setSegmentNumber( *segmentNumber );

        QString branchDescription;
        if ( it == branchSegments.begin() )
        {
            branchDescription = QString( "Segments on branch %1" ).arg( branch->label() );
        }

        collectWelsegsSegment( tableData,
                               segment,
                               outletSegment,
                               exportInfo,
                               maxSegmentLength,
                               customSegmentIntervals,
                               branch,
                               segmentNumber,
                               branchDescription,
                               exportDate );
        outletSegment = segment;

        if ( !exportCompletionSegmentsAfterMainBore )
        {
            collectCompletionsForSegment( tableData,
                                          outletSegment,
                                          segment,
                                          &outletValve,
                                          exportInfo,
                                          maxSegmentLength,
                                          customSegmentIntervals,
                                          segmentNumber );
        }
    }

    if ( exportCompletionSegmentsAfterMainBore )
    {
        it = branchStartSegmentIterator;

        for ( ; it != branchSegments.end(); ++it )
        {
            auto segment = *it;
            collectCompletionsForSegment( tableData,
                                          outletSegment,
                                          segment,
                                          &outletValve,
                                          exportInfo,
                                          maxSegmentLength,
                                          customSegmentIntervals,
                                          segmentNumber );
        }
    }

    for ( auto childBranch : branch->branches() )
    {
        RicMswSegment* outletSegmentForChildBranch = outletSegment;

        RicMswSegment* tieInSegmentOnParentBranch = branch->findClosestSegmentWithLowerMD( childBranch->startMD() );
        if ( tieInSegmentOnParentBranch ) outletSegmentForChildBranch = tieInSegmentOnParentBranch;

        collectWelsegsDataRecursively( tableData,
                                       exportInfo,
                                       childBranch,
                                       segmentNumber,
                                       maxSegmentLength,
                                       customSegmentIntervals,
                                       exportCompletionSegmentsAfterMainBore,
                                       outletSegmentForChildBranch,
                                       exportDate );
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for a single segment with sub-segmentation
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsSegment( RigMswTableData&                              tableData,
                                                  RicMswSegment*                                segment,
                                                  const RicMswSegment*                          previousSegment,
                                                  RicMswExportInfo&                             exportInfo,
                                                  double                                        maxSegmentLength,
                                                  const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                                  gsl::not_null<RicMswBranch*>                  branch,
                                                  int*                                          segmentNumber,
                                                  QString                                       branchDescription,
                                                  const std::optional<QDateTime>&               exportDate )
{
    CVF_ASSERT( segment && segmentNumber );

    double startMD = segment->startMD();
    double endMD   = segment->endMD();

    std::vector<std::pair<double, double>> segments =
        RicMswTableDataTools::createSubSegmentMDPairs( startMD, endMD, maxSegmentLength, customSegmentIntervals );

    CVF_ASSERT( branch->wellPath() );

    double prevOutMD  = branch->startMD();
    double prevOutTVD = branch->startTVD();
    if ( previousSegment )
    {
        prevOutMD  = previousSegment->outputMD();
        prevOutTVD = previousSegment->outputTVD();
    }

    bool setDescription = true;

    auto outletSegment         = previousSegment;
    int  previousSegmentNumber = outletSegment ? outletSegment->segmentNumber() : 1;
    for ( const auto& [subStartMD, subEndMD] : segments )
    {
        double depth  = 0;
        double length = 0;

        double midPointMD  = 0.5 * ( subStartMD + subEndMD );
        double midPointTVD = RicMswTableDataTools::tvdFromMeasuredDepth( branch->wellPath(), midPointMD );

        if ( midPointMD < prevOutMD )
        {
            // The first segment of parent branch may sometimes have a MD that is larger than the first segment on the
            // lateral. If this is the case, use the startMD of the branch instead
            prevOutMD  = branch->startMD();
            prevOutTVD = branch->startTVD();
        }

        if ( exportInfo.lengthAndDepthText() == "INC" )
        {
            depth  = midPointTVD - prevOutTVD;
            length = midPointMD - prevOutMD;
        }
        else
        {
            depth  = midPointTVD;
            length = midPointMD;
        }

        double linerDiameter   = 0.0;
        double roughnessFactor = 0.0;
        if ( exportDate.has_value() )
        {
            linerDiameter = branch->wellPath()->mswCompletionParameters()->getDiameterAtMD( midPointMD, exportInfo.unitSystem(), *exportDate );
            roughnessFactor =
                branch->wellPath()->mswCompletionParameters()->getRoughnessAtMD( midPointMD, exportInfo.unitSystem(), *exportDate );
        }
        else
        {
            linerDiameter   = branch->wellPath()->mswCompletionParameters()->getDiameterAtMD( midPointMD, exportInfo.unitSystem() );
            roughnessFactor = branch->wellPath()->mswCompletionParameters()->getRoughnessAtMD( midPointMD, exportInfo.unitSystem() );
        }

        WelsegsRow row;
        row.sourceWellName = branch->wellPath()->name().toStdString();
        row.segment1       = segment->segmentNumber();
        row.segment2       = segment->segmentNumber();
        row.joinSegment    = previousSegmentNumber;
        row.branch         = branch->branchNumber();
        row.length         = length;
        row.depth          = depth;
        row.diameter       = linerDiameter;
        row.roughness      = roughnessFactor;
        if ( setDescription )
        {
            row.description = branchDescription.toStdString();
            setDescription  = false;
        }

        tableData.addWelsegsRow( row );

        previousSegmentNumber++;

        if ( segments.size() > 1 )
        {
            ( *segmentNumber )++;
            segment->setSegmentNumber( *segmentNumber );
        }

        segment->setOutputMD( midPointMD );
        segment->setOutputTVD( midPointTVD );
        segment->setSegmentNumber( *segmentNumber );

        outletSegment = segment;
        prevOutMD     = midPointMD;
        prevOutTVD    = midPointTVD;
    }

    if ( segments.size() <= 1 )
    {
        ( *segmentNumber )++;
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for valve completions
/// Ported from RicMswTableFormatterTools::writeValveWelsegsSegment()
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectValveWelsegsSegment( RigMswTableData&                              tableData,
                                                       const RicMswSegment*                          outletSegment,
                                                       RicMswValve*                                  valve,
                                                       RicMswExportInfo&                             exportInfo,
                                                       double                                        maxSegmentLength,
                                                       const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                                       int*                                          segmentNumber )
{
    if ( !valve ) return;
    if ( !valve->isValid() ) return;
    if ( !valve->wellPath() ) return;

    auto segments = valve->segments();

    double startMD = 0.0;
    double endMD   = 0.0;

    if ( valve->completionType() == RigCompletionData::CompletionType::PERFORATION_ICD ||
         valve->completionType() == RigCompletionData::CompletionType::PERFORATION_AICD ||
         valve->completionType() == RigCompletionData::CompletionType::PERFORATION_SICD )
    {
        CVF_ASSERT( segments.size() > 1 );

        // The 0.1 valve segment is the first, the perforated segment is the second
        auto subSegment = segments[0];
        subSegment->setSegmentNumber( *segmentNumber );

        double midPointMD = subSegment->outputMD();
        startMD           = midPointMD;
        endMD             = startMD + 0.1;
    }
    else
    {
        auto subSegment = segments.front();
        subSegment->setSegmentNumber( *segmentNumber );

        startMD = subSegment->startMD();
        endMD   = subSegment->endMD();
    }

    auto splitSegments = RicMswTableDataTools::createSubSegmentMDPairs( startMD, endMD, maxSegmentLength, customSegmentIntervals );

    int        outletSegmentNumber = outletSegment ? outletSegment->segmentNumber() : 1;
    const auto linerDiameter       = valve->wellPath()->mswCompletionParameters()->linerDiameter( exportInfo.unitSystem() );
    const auto roughnessFactor     = valve->wellPath()->mswCompletionParameters()->roughnessFactor( exportInfo.unitSystem() );

    bool isCommentAdded = false;
    for ( const auto& [subStartMD, subEndMD] : splitSegments )
    {
        WelsegsRow row;
        row.sourceWellName = valve->wellPath()->name().toStdString();

        if ( !isCommentAdded )
        {
            row.description = valve->label().toStdString();
            isCommentAdded  = true;
        }

        row.segment1    = *segmentNumber;
        row.segment2    = *segmentNumber;
        row.joinSegment = outletSegmentNumber;
        row.branch      = valve->branchNumber();

        const double subStartTVD = RicMswTableDataTools::tvdFromMeasuredDepth( valve->wellPath(), subStartMD );
        const double subEndTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( valve->wellPath(), subEndMD );

        double depth  = 0;
        double length = 0;

        if ( exportInfo.lengthAndDepthText() == QString( "INC" ) )
        {
            depth  = subEndTVD - subStartTVD;
            length = subEndMD - subStartMD;
        }
        else
        {
            depth  = subEndTVD;
            length = subEndMD;
        }

        row.length    = length;
        row.depth     = depth;
        row.diameter  = linerDiameter;
        row.roughness = roughnessFactor;

        tableData.addWelsegsRow( row );

        outletSegmentNumber = *segmentNumber;
        ( *segmentNumber )++;
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for completions on a segment
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompletionsForSegment( RigMswTableData&                              tableData,
                                                         gsl::not_null<const RicMswSegment*>           outletSegment,
                                                         gsl::not_null<RicMswSegment*>                 segment,
                                                         RicMswValve**                                 outletValve,
                                                         RicMswExportInfo&                             exportInfo,
                                                         double                                        maxSegmentLength,
                                                         const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                                         int*                                          segmentNumber )
{
    for ( auto& completion : segment->completions() )
    {
        // For a well with perforation intervals, the WELSEGS segments are reported twice if we include the
        // RicMswPerforation completions. Investigate when this class is intended to be exported to file
        auto performationMsw = dynamic_cast<RicMswPerforation*>( completion );
        if ( performationMsw ) continue;

        auto segmentValve = dynamic_cast<RicMswValve*>( completion );
        auto fishboneIcd  = dynamic_cast<RicMswFishbonesICD*>( completion );
        if ( !fishboneIcd && segmentValve != nullptr )
        {
            collectValveWelsegsSegment( tableData, segment, segmentValve, exportInfo, maxSegmentLength, customSegmentIntervals, segmentNumber );
            *outletValve = segmentValve;
        }
        else if ( dynamic_cast<RicMswTieInICV*>( completion ) )
        {
            // Special handling for Tie-in ICVs
            RicMswSegment* outletSegmentForCompletion =
                *outletValve && ( *outletValve )->segmentCount() > 0 ? ( *outletValve )->segments().front() : segment.get();
            collectCompletionWelsegsSegments( tableData,
                                              outletSegmentForCompletion,
                                              completion,
                                              exportInfo,
                                              maxSegmentLength,
                                              customSegmentIntervals,
                                              segmentNumber );
        }
        else
        {
            // This is the default case for completions that are not valves
            collectCompletionWelsegsSegments( tableData, segment, completion, exportInfo, maxSegmentLength, customSegmentIntervals, segmentNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for a completion's segments
/// Porting of RicMswTableFormatterTools::writeCompletionWelsegsSegments()
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompletionWelsegsSegments( RigMswTableData&                              tableData,
                                                             gsl::not_null<const RicMswSegment*>           outletSegment,
                                                             gsl::not_null<RicMswCompletion*>              completion,
                                                             RicMswExportInfo&                             exportInfo,
                                                             double                                        maxSegmentLength,
                                                             const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                                             int*                                          segmentNumber )
{
    bool isDescriptionAdded = false;

    auto outletNumber = outletSegment->segmentNumber();

    for ( auto segment : completion->segments() )
    {
        segment->setSegmentNumber( *segmentNumber );

        double startMD  = segment->startMD();
        double endMD    = segment->endMD();
        double startTVD = segment->startTVD();
        double endTVD   = segment->endTVD();

        auto splitSegments = RicMswTableDataTools::createSubSegmentMDPairs( startMD, endMD, maxSegmentLength, customSegmentIntervals );
        for ( const auto& [subStartMD, subEndMD] : splitSegments )
        {
            int subSegmentNumber = ( *segmentNumber )++;

            // TODO: Verify this calculation for fractures
            double subStartTVD = RicMswTableDataTools::tvdFromMeasuredDepth( completion->wellPath(), subStartMD );
            double subEndTVD   = RicMswTableDataTools::tvdFromMeasuredDepth( completion->wellPath(), subEndMD );

            if ( completion->completionType() == RigCompletionData::CompletionType::FISHBONES )
            {
                // Not possible to do interpolation based on well path geometry here
                // Use linear interpolation based on start/end TVD for segment
                {
                    auto normalizedWeight = ( subStartMD - startMD ) / ( endMD - startMD );
                    subStartTVD           = startTVD * ( 1.0 - normalizedWeight ) + endTVD * normalizedWeight;
                }
                {
                    auto normalizedWeight = ( subEndMD - startMD ) / ( endMD - startMD );

                    subEndTVD = startTVD * ( 1.0 - normalizedWeight ) + endTVD * normalizedWeight;
                }
            }

            double depth  = 0;
            double length = 0;

            if ( exportInfo.lengthAndDepthText() == QString( "INC" ) )
            {
                depth  = subEndTVD - subStartTVD;
                length = subEndMD - subStartMD;
            }
            else
            {
                depth  = subEndTVD;
                length = subEndMD;
            }

            double diameter = segment->equivalentDiameter();
            if ( segment->effectiveDiameter() > 0.0 ) diameter = segment->effectiveDiameter();

            WelsegsRow row;
            row.sourceWellName = completion->wellPath()->name().toStdString();
            row.segment1       = subSegmentNumber;
            row.segment2       = subSegmentNumber;
            row.joinSegment    = outletNumber;
            row.branch         = completion->branchNumber();
            row.length         = length;
            row.depth          = depth;
            row.diameter       = diameter;
            row.roughness      = segment->openHoleRoughnessFactor();

            if ( !isDescriptionAdded )
            {
                row.description    = completion->label().toStdString();
                isDescriptionAdded = true;
            }

            tableData.addWelsegsRow( row );

            outletNumber = subSegmentNumber;
        }

        for ( auto comp : segment->completions() )
        {
            collectCompletionWelsegsSegments( tableData, segment, comp, exportInfo, maxSegmentLength, customSegmentIntervals, segmentNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::generateWsegAicdTableRecursively( RicMswExportInfo&                                 exportInfo,
                                                             gsl::not_null<const RicMswBranch*>                branch,
                                                             std::map<size_t, std::vector<AicdWsegvalveData>>& aicdValveData,
                                                             const std::optional<QDateTime>&                   exportDate )
{
    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION_AICD )
            {
                auto aicd = static_cast<const RicMswPerforationAICD*>( completion );

                // Filter by export date if specified
                if ( exportDate.has_value() && aicd->wellPathValve() && !aicd->wellPathValve()->isActiveOnDate( *exportDate ) ) continue;

                if ( aicd->isValid() )
                {
                    int segmentNumber = -1;
                    for ( auto seg : aicd->segments() )
                    {
                        if ( seg->segmentNumber() > -1 ) segmentNumber = seg->segmentNumber();
                        if ( seg->intersections().empty() ) continue;

                        size_t cellIndex = seg->intersections().front()->globalCellIndex();

                        auto wellName = exportInfo.mainBoreBranch()->wellPath()->completionSettings()->wellNameForExport();
                        auto comment  = aicd->label();
                        aicdValveData[cellIndex].push_back(
                            AicdWsegvalveData( wellName, comment, segmentNumber, aicd->flowScalingFactor(), aicd->isOpen(), aicd->values() ) );
                    }
                }
                else
                {
                    RiaLogging::error( QString( "Export AICD Valve (%1): Valve is invalid. At least one required "
                                                "template parameter is not set." )
                                           .arg( aicd->label() ) );
                }
            }
        }
    }

    for ( auto childBranch : branch->branches() )
    {
        generateWsegAicdTableRecursively( exportInfo, childBranch, aicdValveData, exportDate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompsegData( RigMswTableData&                tableData,
                                               RicMswExportInfo&               exportInfo,
                                               bool                            exportSubGridIntersections,
                                               const std::optional<QDateTime>& exportDate )
{
    // Define completion types to export
    std::set<RigCompletionData::CompletionType> perforationTypes = { RigCompletionData::CompletionType::PERFORATION,
                                                                     RigCompletionData::CompletionType::PERFORATION_ICD,
                                                                     RigCompletionData::CompletionType::PERFORATION_ICV,
                                                                     RigCompletionData::CompletionType::PERFORATION_AICD,
                                                                     RigCompletionData::CompletionType::PERFORATION_SICD };

    std::set<RigCompletionData::CompletionType> fishbonesTypes = { RigCompletionData::CompletionType::FISHBONES_ICD,
                                                                   RigCompletionData::CompletionType::FISHBONES };

    std::set<RigCompletionData::CompletionType> fractureTypes = { RigCompletionData::CompletionType::FRACTURE };

    std::set<size_t> intersectedCells;

    // Collect in order: perforations, fishbones, fractures
    collectCompsegDataByType( tableData,
                              exportInfo,
                              exportInfo.mainBoreBranch(),
                              exportSubGridIntersections,
                              perforationTypes,
                              &intersectedCells,
                              exportDate );
    collectCompsegDataByType( tableData,
                              exportInfo,
                              exportInfo.mainBoreBranch(),
                              exportSubGridIntersections,
                              fishbonesTypes,
                              &intersectedCells,
                              exportDate );
    collectCompsegDataByType( tableData, exportInfo, exportInfo.mainBoreBranch(), exportSubGridIntersections, fractureTypes, &intersectedCells, exportDate );
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect COMPSEGS data for specific completion types
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompsegDataByType( RigMswTableData&                                   tableData,
                                                     RicMswExportInfo&                                  exportInfo,
                                                     gsl::not_null<const RicMswBranch*>                 branch,
                                                     bool                                               exportSubGridIntersections,
                                                     const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                                                     gsl::not_null<std::set<size_t>*>                   intersectedCells,
                                                     const std::optional<QDateTime>&                    exportDate )
{
    for ( auto segment : branch->segments() )
    {
        auto completion = dynamic_cast<const RicMswCompletion*>( branch.get() );

        for ( auto intersection : segment->intersections() )
        {
            bool isSubGridIntersection = !intersection->gridName().isEmpty();
            if ( isSubGridIntersection != exportSubGridIntersections ) continue;

            double startLength = segment->startMD();
            double endLength   = segment->endMD();

            if ( completion )
            {
                bool isPerforationValve = completion->completionType() == RigCompletionData::CompletionType::PERFORATION_ICD ||
                                          completion->completionType() == RigCompletionData::CompletionType::PERFORATION_AICD ||
                                          completion->completionType() == RigCompletionData::CompletionType::PERFORATION_ICV ||
                                          completion->completionType() == RigCompletionData::CompletionType::PERFORATION_SICD;

                if ( isPerforationValve )
                {
                    startLength = segment->startMD();
                    endLength   = segment->endMD();
                }
            }

            size_t globalCellIndex = intersection->globalCellIndex();

            // Check if the cell is already reported. Make sure we report intersections before other completions
            // on the segment to be able to connect the branch with most flow
            if ( !intersectedCells->count( globalCellIndex ) )
            {
                CompsegsRow row;

                auto ijk = intersection->gridLocalCellIJK().toOneBased();
                row.i    = ijk.i();
                row.j    = ijk.j();
                row.k    = ijk.k();

                int branchNumber = -1;
                if ( completion ) branchNumber = completion->branchNumber();
                row.branch = branchNumber;

                row.distanceStart = startLength;
                row.distanceEnd   = endLength;
                row.gridName      = exportSubGridIntersections ? intersection->gridName().toStdString() : "";

                tableData.addCompsegsRow( row );
                intersectedCells->insert( globalCellIndex );
            }
        }

        // Report connected completions after the intersection on current segment has been reported
        for ( auto completion : segment->completions() )
        {
            if ( completion->segments().empty() || !exportCompletionTypes.count( completion->completionType() ) ) continue;

            // Filter perforations by export date if specified
            if ( exportDate.has_value() )
            {
                auto* perf = dynamic_cast<const RicMswPerforation*>( completion );
                if ( perf && perf->perforationInterval() )
                {
                    if ( !perf->perforationInterval()->isActiveOnDate( exportDate.value() ) )
                    {
                        continue;
                    }
                }
            }

            collectCompsegDataByType( tableData, exportInfo, completion, exportSubGridIntersections, exportCompletionTypes, intersectedCells, exportDate );
        }
    }

    for ( auto childBranch : branch->branches() )
    {
        collectCompsegDataByType( tableData, exportInfo, childBranch, exportSubGridIntersections, exportCompletionTypes, intersectedCells, exportDate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegvalvData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate )
{
    auto mainBore          = exportInfo.mainBoreBranch();
    auto wellNameForExport = mainBore->wellPath()->completionSettings()->wellNameForExport().toStdString();

    collectStandaloneWsegvalvDataRecursively( tableData, mainBore, wellNameForExport );

    collectWsegvalvDataRecursively( tableData, mainBore, wellNameForExport, exportDate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectStandaloneWsegvalvDataRecursively( RigMswTableData& tableData, RicMswBranch* branch, std::string wellName )
{
    auto valveColl = branch->wellPath()->valveCollection();
    for ( auto valve : valveColl->activeValves() )
    {
        auto segment       = branch->findClosestSegmentWithLowerMD( valve->startMD() );
        auto segmentNumber = segment ? segment->segmentNumber() : 1;
        if ( segmentNumber > 1 )
        {
            if ( valve->startMD() < segment->startMD() ) segmentNumber--;
        }

        WsegvalvRow row;
        row.well          = wellName;
        row.segmentNumber = segmentNumber;
        row.cv            = valve->flowCoefficient();
        row.area          = valve->area( branch->wellPath()->unitSystem() );
        row.status        = "OPEN";

        row.description = valve->name().toStdString();

        tableData.addWsegvalvRow( row );
    }

    for ( auto subBranch : branch->branches() )
    {
        collectStandaloneWsegvalvDataRecursively( tableData, subBranch, wellName );
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WSEGVALV data recursively through branches
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegvalvDataRecursively( RigMswTableData&                tableData,
                                                           gsl::not_null<RicMswBranch*>    branch,
                                                           const std::string&              wellNameForExport,
                                                           const std::optional<QDateTime>& exportDate )
{
    // Handle tie-in ICV at branch level
    {
        auto tieInValve = dynamic_cast<RicMswTieInICV*>( branch.get() );
        if ( tieInValve && !tieInValve->segments().empty() )
        {
            // Check if valve is active on export date (or no date filter is set)
            bool isActiveOnDate = true;
            if ( exportDate.has_value() && tieInValve->wellPathValve() )
            {
                isActiveOnDate = tieInValve->wellPathValve()->isActiveOnDate( exportDate.value() );
            }

            if ( isActiveOnDate )
            {
                auto firstSubSegment = tieInValve->segments().front();
                CAF_ASSERT( tieInValve->completionType() == RigCompletionData::CompletionType::PERFORATION_ICV );

                auto flowCoefficient = tieInValve->flowCoefficient();

                WsegvalvRow row;
                row.well          = wellNameForExport;
                row.segmentNumber = firstSubSegment->segmentNumber();
                row.cv            = flowCoefficient;
                row.area          = tieInValve->area();
                row.status        = tieInValve->isOpen() ? "OPEN" : "SHUT";
                row.description   = tieInValve->label().toStdString();

                tableData.addWsegvalvRow( row );
            }
        }
    }

    // Process segments and their completions
    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( RigCompletionData::isWsegValveTypes( completion->completionType() ) )
            {
                auto wsegValve = static_cast<RicMswWsegValve*>( completion );

                // Filter by export date if specified
                if ( exportDate.has_value() && wsegValve->wellPathValve() && !wsegValve->wellPathValve()->isActiveOnDate( *exportDate ) )
                    continue;

                int segmentNumber = -1;

                for ( auto seg : wsegValve->segments() )
                {
                    if ( seg->segmentNumber() > -1 ) segmentNumber = seg->segmentNumber();
                    if ( seg->intersections().empty() ) continue;

                    QString comment;
                    if ( wsegValve->completionType() == RigCompletionData::CompletionType::PERFORATION_ICD ||
                         wsegValve->completionType() == RigCompletionData::CompletionType::PERFORATION_ICV )
                    {
                        comment = wsegValve->label();
                    }

                    WsegvalvRow row;
                    row.well          = wellNameForExport;
                    row.segmentNumber = segmentNumber;
                    row.cv            = wsegValve->flowCoefficient();
                    row.area          = wsegValve->area();
                    row.status        = wsegValve->isOpen() ? "OPEN" : "SHUT";
                    row.description   = wsegValve->label().toStdString();

                    tableData.addWsegvalvRow( row );
                }
            }
        }
    }

    // Recurse into child branches
    for ( auto childBranch : branch->branches() )
    {
        collectWsegvalvDataRecursively( tableData, childBranch, wellNameForExport, exportDate );
    }
}

//--------------------------------------------------------------------------------------------------
/// Based on RicMswTableFormatterTools::generateWsegAicdTable()
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegAicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate )
{
    std::map<size_t, std::vector<AicdWsegvalveData>> aicdValveData;
    generateWsegAicdTableRecursively( exportInfo, exportInfo.mainBoreBranch(), aicdValveData, exportDate );

    // Export data for each cell with AICD valves

    for ( auto [globalCellIndex, aicdDataForSameCell] : aicdValveData )
    {
        if ( aicdDataForSameCell.empty() ) continue;

        double      accumulatedFlowScalingFactorDivisor = 0.0;
        QStringList comments;

        // See RicMswAICDAccumulator::accumulateValveParameters for similar accumulation for multiple valves in same
        // segment

        for ( const auto& aicdData : aicdDataForSameCell )
        {
            accumulatedFlowScalingFactorDivisor += 1.0 / aicdData.m_flowScalingFactor;
            comments.push_back( aicdData.m_comment );
        }

        WsegaicdRow row;

        auto commentsCombined = comments.join( "; " );
        row.description       = commentsCombined.toStdString();

        auto firstDataObject = aicdDataForSameCell.front();

        row.well     = firstDataObject.m_wellName.toStdString(); // #1
        row.segment1 = firstDataObject.m_segmentNumber; // #2
        row.segment2 = firstDataObject.m_segmentNumber; // #3

        std::array<double, AICD_NUM_PARAMS> values = firstDataObject.m_values;

        row.strength = values[AICD_STRENGTH]; // #4 : AICD Strength

        double flowScalingFactor = 1.0 / accumulatedFlowScalingFactorDivisor;
        row.length               = flowScalingFactor; // #5 : AICD Length is used to store the flow scaling factor

        auto setOptional = []( double value ) -> std::optional<double>
        {
            if ( value == RicMswExportInfo::defaultDoubleValue() ) return std::nullopt;

            return value;
        };

        row.densityCali   = setOptional( values[AICD_DENSITY_CALIB_FLUID] ); //           #6
        row.viscosityCali = setOptional( values[AICD_VISCOSITY_CALIB_FLUID] ); //         #7
        row.criticalValue = setOptional( values[AICD_CRITICAL_WATER_IN_LIQUID_FRAC] ); // #8
        row.widthTrans    = setOptional( values[AICD_EMULSION_VISC_TRANS_REGION] ); //    #9
        row.maxViscRatio  = setOptional( values[AICD_MAX_RATIO_EMULSION_VISC] ); //      #10

        row.methodScalingFactor = 1; // #11 : Always use method "b. Scale factor". The value of the
                                     // scale factor is given in item #5

        row.maxAbsRate        = values[AICD_MAX_FLOW_RATE]; //                           #12
        row.flowRateExponent  = values[AICD_VOL_FLOW_EXP]; //                            #13
        row.viscExponent      = values[AICD_VISOSITY_FUNC_EXP]; //                       #14
        row.status            = firstDataObject.m_isOpen ? "OPEN" : "SHUT"; //           #15
        row.oilFlowFraction   = setOptional( values[AICD_EXP_OIL_FRAC_DENSITY] ); //     #16
        row.waterFlowFraction = setOptional( values[AICD_EXP_WATER_FRAC_DENSITY] ); //   #17
        row.gasFlowFraction   = setOptional( values[AICD_EXP_GAS_FRAC_DENSITY] ); //     #18
        row.oilViscFraction   = setOptional( values[AICD_EXP_OIL_FRAC_VISCOSITY] ); //   #19
        row.waterViscFraction = setOptional( values[AICD_EXP_WATER_FRAC_VISCOSITY] ); // #20
        row.gasViscFraction   = setOptional( values[AICD_EXP_GAS_FRAC_VISCOSITY] ); //   #21

        tableData.addWsegaicdRow( row );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegSicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate )
{
    std::map<size_t, std::vector<SicdWsegvalveData>> sicdValveData;
    generateWsegSicdTableRecursively( exportInfo, exportInfo.mainBoreBranch(), sicdValveData, exportDate );

    // Export data for each cell with SICD valves

    for ( auto [globalCellIndex, sicdDataForSameCell] : sicdValveData )
    {
        if ( sicdDataForSameCell.empty() ) continue;

        double      accumulatedFlowScalingFactorDivisor = 0.0;
        QStringList comments;

        for ( const auto& aicdData : sicdDataForSameCell )
        {
            accumulatedFlowScalingFactorDivisor += 1.0 / aicdData.m_flowScalingFactor;
            comments.push_back( aicdData.m_comment );
        }

        WsegsicdRow row;

        auto commentsCombined = comments.join( "; " );
        row.description       = commentsCombined.toStdString();

        auto firstDataObject = sicdDataForSameCell.front();

        row.well     = firstDataObject.m_wellName.toStdString(); // #1
        row.segment1 = firstDataObject.m_segmentNumber; // #2
        row.segment2 = firstDataObject.m_segmentNumber; // #3

        std::array<double, SICD_NUM_PARAMS> values = firstDataObject.m_values;

        row.strength = values[SICD_STRENGTH]; // #4 : Strength

        double flowScalingFactor = 1.0 / accumulatedFlowScalingFactorDivisor;
        row.length               = flowScalingFactor; // #5 : Length is used to store the flow scaling factor

        auto setOptional = []( double value ) -> std::optional<double>
        {
            if ( value == RicMswExportInfo::defaultDoubleValue() ) return std::nullopt;

            return value;
        };

        row.densityCali   = setOptional( values[SICD_CALIBRATION_DENSITY] ); //   #6
        row.viscosityCali = setOptional( values[SICD_CALIBRATION_VISCOSITY] ); // #7
        row.criticalValue = setOptional( values[SICD_EML_CRT] ); //               #8
        row.widthTrans    = setOptional( values[SICD_EML_TRANS] ); //             #9
        row.maxViscRatio  = setOptional( values[SICD_EML_MAX] ); //               #10

        row.methodScalingFactor = 1; // #11 : Always use method "b. Scale factor". The value of the
                                     // scale factor is given in item #5

        row.maxAbsRate = values[SICD_MAX_CALIB_RATE]; //                   #12
        row.status     = firstDataObject.m_isOpen ? "OPEN" : "SHUT"; //    #13

        tableData.addWsegsicdRow( row );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::generateWsegSicdTableRecursively( RicMswExportInfo&                                 exportInfo,
                                                             gsl::not_null<const RicMswBranch*>                branch,
                                                             std::map<size_t, std::vector<SicdWsegvalveData>>& sicdValveData,
                                                             const std::optional<QDateTime>&                   exportDate )
{
    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION_SICD )
            {
                auto sicd = static_cast<const RicMswPerforationSICD*>( completion );

                // Filter by export date if specified
                if ( exportDate.has_value() && sicd->wellPathValve() && !sicd->wellPathValve()->isActiveOnDate( *exportDate ) ) continue;

                if ( sicd->isValid() )
                {
                    int segmentNumber = -1;
                    for ( auto seg : sicd->segments() )
                    {
                        if ( seg->segmentNumber() > -1 ) segmentNumber = seg->segmentNumber();
                        if ( seg->intersections().empty() ) continue;

                        size_t cellIndex = seg->intersections().front()->globalCellIndex();

                        auto wellName = exportInfo.mainBoreBranch()->wellPath()->completionSettings()->wellNameForExport();
                        auto comment  = sicd->label();
                        sicdValveData[cellIndex].push_back(
                            SicdWsegvalveData( wellName, comment, segmentNumber, sicd->flowScalingFactor(), sicd->isOpen(), sicd->values() ) );
                    }
                }
                else
                {
                    RiaLogging::error( QString( "Export SICD Valve (%1): Valve is invalid. At least one required "
                                                "template parameter is not set." )
                                           .arg( sicd->label() ) );
                }
            }
        }
    }

    for ( auto childBranch : branch->branches() )
    {
        generateWsegSicdTableRecursively( exportInfo, childBranch, sicdValveData, exportDate );
    }
}

//--------------------------------------------------------------------------------------------------
/// Creates sub-segment MD pairs by combining custom intervals with max segment length subdivision
/// Custom intervals define exact segment boundaries where specified
/// Areas without custom intervals use max segment length subdivision (if maxSegmentLength > 0)
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RicMswTableDataTools::createSubSegmentMDPairs( double                                        startMD,
                                                   double                                        endMD,
                                                   double                                        maxSegmentLength,
                                                   const std::vector<std::pair<double, double>>& customSegmentIntervals )
{
    std::vector<std::pair<double, double>> subSegmentMDPairs;

    // If no custom intervals, use original logic with maxSegmentLength subdivision
    if ( customSegmentIntervals.empty() || maxSegmentLength <= 0.0 )
    {
        int    subSegmentCount  = maxSegmentLength > 0.0 ? (int)( std::trunc( ( endMD - startMD ) / maxSegmentLength ) + 1 ) : 1;
        double subSegmentLength = ( endMD - startMD ) / subSegmentCount;

        double subStartMD = startMD;
        double subEndMD   = startMD + subSegmentLength;
        for ( int i = 0; i < subSegmentCount; ++i )
        {
            subSegmentMDPairs.push_back( std::make_pair( subStartMD, subEndMD ) );
            subStartMD += subSegmentLength;
            subEndMD = std::min( subEndMD + subSegmentLength, endMD );
        }
        return subSegmentMDPairs;
    }

    // Combine custom intervals with maxSegmentLength subdivision
    // Collect all boundaries (start, end, custom interval boundaries) and sort them
    std::set<double> boundaries;
    boundaries.insert( startMD );
    boundaries.insert( endMD );

    // Add custom interval boundaries that overlap with [startMD, endMD]
    for ( const auto& [customStart, customEnd] : customSegmentIntervals )
    {
        // Check if custom interval overlaps with [startMD, endMD]
        if ( customEnd > startMD && customStart < endMD )
        {
            // Clip custom interval to [startMD, endMD] range
            double clippedStart = std::max( customStart, startMD );
            double clippedEnd   = std::min( customEnd, endMD );

            if ( clippedStart < clippedEnd )
            {
                boundaries.insert( clippedStart );
                boundaries.insert( clippedEnd );
            }
        }
    }

    // Convert boundaries to sorted vector
    std::vector<double> sortedBoundaries( boundaries.begin(), boundaries.end() );

    // For each gap between boundaries, either:
    // - Use exact boundary if it's from a custom interval
    // - Subdivide using maxSegmentLength if it's a gap
    for ( size_t i = 0; i + 1 < sortedBoundaries.size(); ++i )
    {
        double gapStart = sortedBoundaries[i];
        double gapEnd   = sortedBoundaries[i + 1];

        // Check if this gap is covered by a custom interval
        bool coveredByCustomInterval = false;
        for ( const auto& [customStart, customEnd] : customSegmentIntervals )
        {
            double clippedStart = std::max( customStart, startMD );
            double clippedEnd   = std::min( customEnd, endMD );

            // If the gap is fully within a custom interval, use exact boundaries
            if ( gapStart >= clippedStart && gapEnd <= clippedEnd && std::abs( gapStart - clippedStart ) < 1e-6 &&
                 std::abs( gapEnd - clippedEnd ) < 1e-6 )
            {
                coveredByCustomInterval = true;
                subSegmentMDPairs.push_back( std::make_pair( gapStart, gapEnd ) );
                break;
            }
        }

        // If not covered by custom interval, subdivide using maxSegmentLength
        if ( !coveredByCustomInterval && maxSegmentLength > 0.0 )
        {
            double gapLength = gapEnd - gapStart;
            int    subCount  = (int)( std::trunc( gapLength / maxSegmentLength ) + 1 );
            double subLength = gapLength / subCount;

            double subStart = gapStart;
            for ( int j = 0; j < subCount; ++j )
            {
                double subEnd = ( j == subCount - 1 ) ? gapEnd : subStart + subLength;
                subSegmentMDPairs.push_back( std::make_pair( subStart, subEnd ) );
                subStart = subEnd;
            }
        }
        else if ( !coveredByCustomInterval )
        {
            // No maxSegmentLength, use gap as-is
            subSegmentMDPairs.push_back( std::make_pair( gapStart, gapEnd ) );
        }
    }

    return subSegmentMDPairs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswTableDataTools::tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath, double measuredDepth )
{
    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    double tvdValue = -wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth ).z();

    return tvdValue;
}
