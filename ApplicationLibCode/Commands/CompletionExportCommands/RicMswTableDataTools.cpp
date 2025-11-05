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
#include "RicMswTableFormatterTools.h"

#include "RifTextDataTableFormatter.h"

#include "Well/RigWellPath.h"

#include "RimMswCompletionParameters.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsData( RigMswTableData&  tableData,
                                               RicMswExportInfo& exportInfo,
                                               double            maxSegmentLength,
                                               bool              exportCompletionSegmentsAfterMainBore )
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
    header.flowModel          = exportInfo.lengthAndDepthText().toStdString();
    header.pressureComponents = exportInfo.pressureDropText().toStdString();

    tableData.setWelsegsHeader( header );

    int segmentNumber = 2; // There's an implicit segment number 1.

    // Collect segment data recursively
    collectWelsegsDataRecursively( tableData,
                                   exportInfo,
                                   exportInfo.mainBoreBranch(),
                                   &segmentNumber,
                                   maxSegmentLength,
                                   exportCompletionSegmentsAfterMainBore,
                                   nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsDataRecursively( RigMswTableData&             tableData,
                                                          RicMswExportInfo&            exportInfo,
                                                          gsl::not_null<RicMswBranch*> branch,
                                                          gsl::not_null<int*>          segmentNumber,
                                                          double                       maxSegmentLength,
                                                          bool                         exportCompletionSegmentsAfterMainBore,
                                                          RicMswSegment*               connectedToSegment )
{
    auto         outletSegment = connectedToSegment;
    RicMswValve* outletValve   = nullptr;

    auto branchSegments = branch->segments();
    auto it             = branchSegments.begin();

    // Handle tie-in ICV at the beginning of branch
    if ( outletValve = dynamic_cast<RicMswTieInICV*>( branch.get() ); outletValve != nullptr )
    {
        collectValveWelsegsSegment( tableData, outletSegment, outletValve, exportInfo, maxSegmentLength, segmentNumber );

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

        collectWelsegsSegment( tableData, segment, outletSegment, exportInfo, maxSegmentLength, branch, segmentNumber, branchDescription );
        outletSegment = segment;

        if ( !exportCompletionSegmentsAfterMainBore )
        {
            collectCompletionsForSegment( tableData, outletSegment, segment, &outletValve, exportInfo, maxSegmentLength, segmentNumber );
        }
    }

    if ( exportCompletionSegmentsAfterMainBore )
    {
        it = branchStartSegmentIterator;

        for ( ; it != branchSegments.end(); ++it )
        {
            auto segment = *it;
            collectCompletionsForSegment( tableData, outletSegment, segment, &outletValve, exportInfo, maxSegmentLength, segmentNumber );
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
                                       exportCompletionSegmentsAfterMainBore,
                                       outletSegmentForChildBranch );
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for a single segment with sub-segmentation
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWelsegsSegment( RigMswTableData&             tableData,
                                                  RicMswSegment*               segment,
                                                  const RicMswSegment*         previousSegment,
                                                  RicMswExportInfo&            exportInfo,
                                                  double                       maxSegmentLength,
                                                  gsl::not_null<RicMswBranch*> branch,
                                                  int*                         segmentNumber,
                                                  QString                      branchDescription )
{
    CVF_ASSERT( segment && segmentNumber );

    double startMD = segment->startMD();
    double endMD   = segment->endMD();

    std::vector<std::pair<double, double>> segments = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, maxSegmentLength );

    CVF_ASSERT( branch->wellPath() );

    double prevOutMD  = branch->startMD();
    double prevOutTVD = branch->startTVD();
    if ( previousSegment )
    {
        prevOutMD  = previousSegment->outputMD();
        prevOutTVD = previousSegment->outputTVD();
    }

    bool setDescription = true;

    auto outletSegment = previousSegment;
    for ( const auto& [subStartMD, subEndMD] : segments )
    {
        double depth  = 0;
        double length = 0;

        double midPointMD  = 0.5 * ( subStartMD + subEndMD );
        double midPointTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( branch->wellPath(), midPointMD );

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

        const auto linerDiameter   = branch->wellPath()->mswCompletionParameters()->getDiameterAtMD( midPointMD, exportInfo.unitSystem() );
        const auto roughnessFactor = branch->wellPath()->mswCompletionParameters()->getRoughnessAtMD( midPointMD, exportInfo.unitSystem() );

        WelsegsRow row;
        row.segment1    = segment->segmentNumber();
        row.segment2    = segment->segmentNumber();
        row.joinSegment = outletSegment ? outletSegment->segmentNumber() : 1;
        row.branch      = branch->branchNumber();
        row.length      = length;
        row.depth       = depth;
        row.diameter    = linerDiameter;
        row.roughness   = roughnessFactor;
        if ( setDescription )
        {
            row.description = branchDescription.toStdString();
            setDescription  = false;
        }

        tableData.addWelsegsRow( row );

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
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectValveWelsegsSegment( RigMswTableData&     tableData,
                                                       const RicMswSegment* outletSegment,
                                                       RicMswValve*         valve,
                                                       RicMswExportInfo&    exportInfo,
                                                       double               maxSegmentLength,
                                                       int*                 segmentNumber )
{
    for ( auto valveSegment : valve->segments() )
    {
        valveSegment->setSegmentNumber( *segmentNumber );

        WelsegsRow row;
        row.segment1    = valveSegment->segmentNumber();
        row.segment2    = valveSegment->segmentNumber();
        row.joinSegment = outletSegment ? outletSegment->segmentNumber() : 1;
        row.branch      = valve->branchNumber();
        row.length      = valveSegment->startMD();
        row.depth       = valveSegment->startTVD();
        row.diameter    = valveSegment->equivalentDiameter();
        row.roughness   = valveSegment->openHoleRoughnessFactor();
        // row.description         = QString( "Valve %1" ).arg( valve->label() );

        tableData.addWelsegsRow( row );

        ( *segmentNumber )++;
        outletSegment = valveSegment;
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for completions on a segment
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompletionsForSegment( RigMswTableData&                    tableData,
                                                         gsl::not_null<const RicMswSegment*> outletSegment,
                                                         gsl::not_null<RicMswSegment*>       segment,
                                                         RicMswValve**                       outletValve,
                                                         RicMswExportInfo&                   exportInfo,
                                                         double                              maxSegmentLength,
                                                         int*                                segmentNumber )
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
            collectValveWelsegsSegment( tableData, segment, segmentValve, exportInfo, maxSegmentLength, segmentNumber );
            *outletValve = segmentValve;
        }
        else if ( dynamic_cast<RicMswTieInICV*>( completion ) )
        {
            // Special handling for Tie-in ICVs
            RicMswSegment* outletSegmentForCompletion =
                *outletValve && ( *outletValve )->segmentCount() > 0 ? ( *outletValve )->segments().front() : segment.get();
            collectCompletionWelsegsSegments( tableData, outletSegmentForCompletion, completion, exportInfo, maxSegmentLength, segmentNumber );
        }
        else
        {
            // This is the default case for completions that are not valves
            collectCompletionWelsegsSegments( tableData, segment, completion, exportInfo, maxSegmentLength, segmentNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WELSEGS data for a completion's segments
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompletionWelsegsSegments( RigMswTableData&                    tableData,
                                                             gsl::not_null<const RicMswSegment*> outletSegment,
                                                             gsl::not_null<RicMswCompletion*>    completion,
                                                             RicMswExportInfo&                   exportInfo,
                                                             double                              maxSegmentLength,
                                                             int*                                segmentNumber )
{
    bool isDescriptionAdded = false;

    auto outletNumber = outletSegment->segmentNumber();

    for ( auto completionSegment : completion->segments() )
    {
        completionSegment->setSegmentNumber( *segmentNumber );

        WelsegsRow row;
        row.segment1    = completionSegment->segmentNumber();
        row.segment2    = completionSegment->segmentNumber();
        row.joinSegment = outletNumber;
        row.branch      = completion->branchNumber();
        row.length      = completionSegment->startMD();
        row.depth       = completionSegment->startTVD();
        row.diameter    = completionSegment->equivalentDiameter();
        row.roughness   = completionSegment->openHoleRoughnessFactor();

        if ( !isDescriptionAdded )
        {
            row.description    = completion->label().toStdString();
            isDescriptionAdded = true;
        }

        tableData.addWelsegsRow( row );

        outletNumber = *segmentNumber;
        ( *segmentNumber )++;

        for ( auto comp : completionSegment->completions() )
        {
            collectCompletionWelsegsSegments( tableData, completionSegment, comp, exportInfo, maxSegmentLength, segmentNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompsegData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, bool exportSubGridIntersections )
{
    // Define completion types to export
    std::set<RigCompletionData::CompletionType> perforationTypes = { RigCompletionData::CompletionType::PERFORATION,
                                                                     RigCompletionData::CompletionType::PERFORATION_ICD,
                                                                     RigCompletionData::CompletionType::PERFORATION_ICV,
                                                                     RigCompletionData::CompletionType::PERFORATION_AICD };

    std::set<RigCompletionData::CompletionType> fishbonesTypes = { RigCompletionData::CompletionType::FISHBONES_ICD,
                                                                   RigCompletionData::CompletionType::FISHBONES };

    std::set<RigCompletionData::CompletionType> fractureTypes = { RigCompletionData::CompletionType::FRACTURE };

    std::set<size_t> intersectedCells;

    // Collect in order: perforations, fishbones, fractures
    collectCompsegDataByType( tableData, exportInfo, exportInfo.mainBoreBranch(), exportSubGridIntersections, perforationTypes, &intersectedCells );
    collectCompsegDataByType( tableData, exportInfo, exportInfo.mainBoreBranch(), exportSubGridIntersections, fishbonesTypes, &intersectedCells );
    collectCompsegDataByType( tableData, exportInfo, exportInfo.mainBoreBranch(), exportSubGridIntersections, fractureTypes, &intersectedCells );
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect COMPSEGS data for specific completion types
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectCompsegDataByType( RigMswTableData&                                   tableData,
                                                     RicMswExportInfo&                                  exportInfo,
                                                     gsl::not_null<const RicMswBranch*>                 branch,
                                                     bool                                               exportSubGridIntersections,
                                                     const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                                                     gsl::not_null<std::set<size_t>*>                   intersectedCells )
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
                                          completion->completionType() == RigCompletionData::CompletionType::PERFORATION_ICV;

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

                cvf::Vec3st ijk = intersection->gridLocalCellIJK();
                row.i           = ijk.x() + 1; // Convert to 1-based
                row.j           = ijk.y() + 1;
                row.k           = ijk.z() + 1;

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

            collectCompsegDataByType( tableData, exportInfo, completion, exportSubGridIntersections, exportCompletionTypes, intersectedCells );
        }
    }

    for ( auto childBranch : branch->branches() )
    {
        collectCompsegDataByType( tableData, exportInfo, childBranch, exportSubGridIntersections, exportCompletionTypes, intersectedCells );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegvalvData( RigMswTableData& tableData, RicMswExportInfo& exportInfo )
{
    QString wellNameForExport = exportInfo.mainBoreBranch()->wellPath()->completionSettings()->wellNameForExport();
    collectWsegvalvDataRecursively( tableData, exportInfo.mainBoreBranch(), wellNameForExport.toStdString() );
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WSEGVALV data recursively through branches
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegvalvDataRecursively( RigMswTableData&             tableData,
                                                           gsl::not_null<RicMswBranch*> branch,
                                                           const std::string&           wellNameForExport )
{
    // Handle tie-in ICV at branch level
    {
        auto tieInValve = dynamic_cast<RicMswTieInICV*>( branch.get() );
        if ( tieInValve && !tieInValve->segments().empty() )
        {
            auto firstSubSegment = tieInValve->segments().front();
            CAF_ASSERT( tieInValve->completionType() == RigCompletionData::CompletionType::PERFORATION_ICV );

            auto flowCoefficient = tieInValve->flowCoefficient();

            WsegvalvRow row;
            row.well          = wellNameForExport;
            row.segmentNumber = firstSubSegment->segmentNumber();
            row.cv            = flowCoefficient;
            row.area          = tieInValve->area();

            tableData.addWsegvalvRow( row );
        }
    }

    // Process segments and their completions
    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( RigCompletionData::isWsegValveTypes( completion->completionType() ) )
            {
                auto wsegValve     = static_cast<RicMswWsegValve*>( completion );
                int  segmentNumber = -1;

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

                    tableData.addWsegvalvRow( row );
                }
            }
        }
    }

    // Recurse into child branches
    for ( auto childBranch : branch->branches() )
    {
        collectWsegvalvDataRecursively( tableData, childBranch, wellNameForExport );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegAicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo )
{
    collectWsegAicdDataRecursively( tableData, exportInfo, exportInfo.mainBoreBranch() );
}

//--------------------------------------------------------------------------------------------------
/// Helper function to collect WSEGAICD data recursively through branches
//--------------------------------------------------------------------------------------------------
void RicMswTableDataTools::collectWsegAicdDataRecursively( RigMswTableData&                   tableData,
                                                           RicMswExportInfo&                  exportInfo,
                                                           gsl::not_null<const RicMswBranch*> branch )
{
    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION_AICD )
            {
                auto aicd = static_cast<const RicMswPerforationAICD*>( completion );
                if ( aicd->isValid() )
                {
                    int segmentNumber = -1;
                    for ( auto seg : aicd->segments() )
                    {
                        if ( seg->segmentNumber() > -1 ) segmentNumber = seg->segmentNumber();
                        if ( seg->intersections().empty() ) continue;

                        auto wellName = exportInfo.mainBoreBranch()->wellPath()->completionSettings()->wellNameForExport();
                        auto comment  = aicd->label();

                        WsegaicdRow row;
                        row.well     = wellName.toStdString();
                        row.segment1 = segmentNumber;
                        row.segment2 = segmentNumber;
                        row.strength = aicd->flowScalingFactor();
                        row.length   = aicd->length();

                        // Extract AICD-specific parameters from the values array
                        // auto values = aicd->values();

                        tableData.addWsegaicdRow( row );
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
        collectWsegAicdDataRecursively( tableData, exportInfo, childBranch );
    }
}
