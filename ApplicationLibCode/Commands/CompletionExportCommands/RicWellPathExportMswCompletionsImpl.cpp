/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicWellPathExportMswCompletionsImpl.h"

#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicMswExportInfo.h"
#include "RicMswValveAccumulators.h"
#include "RicWellPathExportCompletionsFileTools.h"

#include "RifTextDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFractureTemplate.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathValve.h"

#include <QFile>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions(
    const RicExportCompletionDataSettingsUi& exportSettings,
    const std::vector<RimWellPath*>&         wellPaths )
{
    std::shared_ptr<QFile> unifiedExportFile;
    if ( exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::UNIFIED_FILE )
    {
        QString unifiedFileName =
            QString( "UnifiedCompletions_MSW_%1" ).arg( exportSettings.caseToApply->caseUserDescription() );
        unifiedExportFile =
            RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, unifiedFileName );
    }

    for ( const auto& wellPath : wellPaths )
    {
        std::shared_ptr<QFile> unifiedWellPathFile;

        bool exportFractures = exportSettings.includeFractures() &&
                               !wellPath->fractureCollection()->activeFractures().empty();
        bool exportPerforations = exportSettings.includePerforations() &&
                                  !wellPath->perforationIntervalCollection()->activePerforations().empty();
        bool exportFishbones = exportSettings.includeFishbones() &&
                               !wellPath->fishbonesCollection()->activeFishbonesSubs().empty();
        bool exportAnyCompletion = exportFractures || exportPerforations || exportFishbones;
        if ( exportAnyCompletion && exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL &&
             !unifiedWellPathFile )
        {
            QString wellFileName = QString( "%1_UnifiedCompletions_MSW_%2" )
                                       .arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
            unifiedWellPathFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, wellFileName );
        }

        if ( exportFractures )
        {
            std::shared_ptr<QFile> fractureExportFile;
            if ( unifiedExportFile )
                fractureExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                fractureExportFile = unifiedWellPathFile;
            else
            {
                QString fileName =
                    QString( "%1_Fracture_MSW_%2" ).arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
                fractureExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForFractures( exportSettings.caseToApply, fractureExportFile, wellPath );
        }

        if ( exportPerforations )
        {
            std::shared_ptr<QFile> perforationsExportFile;
            if ( unifiedExportFile )
                perforationsExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                perforationsExportFile = unifiedWellPathFile;
            else
            {
                QString fileName = QString( "%1_Perforation_MSW_%2" )
                                       .arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
                perforationsExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForPerforations( exportSettings.caseToApply,
                                               perforationsExportFile,
                                               wellPath,
                                               exportSettings.timeStep );
        }

        if ( exportFishbones )
        {
            std::shared_ptr<QFile> fishbonesExportFile;
            if ( unifiedExportFile )
                fishbonesExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                fishbonesExportFile = unifiedWellPathFile;
            else
            {
                QString fileName =
                    QString( "%1_Fishbones_MSW_%2" ).arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
                fishbonesExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForFishbones( exportSettings.caseToApply, fishbonesExportFile, wellPath );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFractures( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath )
{
    auto fractures = wellPath->fractureCollection()->activeFractures();

    if ( eclipseCase == nullptr )
    {
        RiaLogging::error(
            "Export Fracture Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    RicMswExportInfo exportInfo = generateFracturesMswExportInfo( eclipseCase, wellPath, fractures );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );

    double maxSegmentLength = wellPath->fractureCollection()->mswParameters()->maxSegmentLength();

    generateWelsegsTable( formatter, exportInfo, maxSegmentLength );
    generateCompsegTables( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFishbones( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath )
{
    auto fishbonesSubs = wellPath->fishbonesCollection()->activeFishbonesSubs();

    if ( eclipseCase == nullptr )
    {
        RiaLogging::error( "Export Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    RicMswExportInfo exportInfo = generateFishbonesMswExportInfo( eclipseCase, wellPath, fishbonesSubs, true );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );

    double maxSegmentLength = wellPath->fishbonesCollection()->mswParameters()->maxSegmentLength();

    generateWelsegsTable( formatter, exportInfo, maxSegmentLength );
    generateCompsegTables( formatter, exportInfo );
    generateWsegvalvTable( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForPerforations( RimEclipseCase*        eclipseCase,
                                                                             std::shared_ptr<QFile> exportFile,
                                                                             const RimWellPath*     wellPath,
                                                                             int                    timeStep )
{
    auto perforationIntervals = wellPath->perforationIntervalCollection()->activePerforations();

    if ( eclipseCase == nullptr )
    {
        RiaLogging::error( "Export Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    // Check if there exist overlap between valves in a perforation interval
    for ( const auto& perfInterval : perforationIntervals )
    {
        for ( const auto& valve : perfInterval->valves() )
        {
            for ( const auto& otherValve : perfInterval->valves() )
            {
                if ( otherValve != valve )
                {
                    bool hasIntersection =
                        !( ( valve->endMD() < otherValve->startMD() ) || ( otherValve->endMD() < valve->startMD() ) );

                    if ( hasIntersection )
                    {
                        RiaLogging::error(
                            QString( "Valve overlap detected for perforation interval : %1" ).arg( perfInterval->name() ) );

                        RiaLogging::error( "Name of valves" );
                        RiaLogging::error( valve->name() );
                        RiaLogging::error( otherValve->name() );

                        RiaLogging::error( "Failed to export well segments" );

                        return;
                    }
                }
            }
        }
    }

    RicMswExportInfo exportInfo =
        generatePerforationsMswExportInfo( eclipseCase, wellPath, timeStep, perforationIntervals );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );

    double maxSegmentLength = wellPath->perforationIntervalCollection()->mswParameters()->maxSegmentLength();

    generateWelsegsTable( formatter, exportInfo, maxSegmentLength );
    generateCompsegTables( formatter, exportInfo );
    generateWsegvalvTable( formatter, exportInfo );
    generateWsegAicdTable( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWelsegsTable( RifTextDataTableFormatter& formatter,
                                                                const RicMswExportInfo&    exportInfo,
                                                                double                     maxSegmentLength )
{
    formatter.keyword( "WELSEGS" );

    double startMD  = exportInfo.initialMD();
    double startTVD = exportInfo.initialTVD();

    {
        std::vector<RifTextDataTableColumn> header = {
            RifTextDataTableColumn( "Name" ),
            RifTextDataTableColumn( "Dep 1" ),
            RifTextDataTableColumn( "Tlen 1" ),
            RifTextDataTableColumn( "Vol 1" ),
            RifTextDataTableColumn( "Len&Dep" ),
            RifTextDataTableColumn( "PresDrop" ),
        };
        formatter.header( header );

        formatter.add( exportInfo.wellPath()->completionSettings()->wellNameForExport() );
        formatter.add( startTVD );
        formatter.add( startMD );
        formatter.addValueOrDefaultMarker( exportInfo.topWellBoreVolume(), RicMswExportInfo::defaultDoubleValue() );
        formatter.add( exportInfo.lengthAndDepthText() );
        formatter.add( QString( "'%1'" ).arg( exportInfo.pressureDropText() ) );

        formatter.rowCompleted();
    }

    {
        std::vector<RifTextDataTableColumn> header =
            { RifTextDataTableColumn( "First Seg" ),
              RifTextDataTableColumn( "Last Seg" ),
              RifTextDataTableColumn( "Branch Num" ),
              RifTextDataTableColumn( "Outlet Seg" ),
              RifTextDataTableColumn( "Length" ),
              RifTextDataTableColumn( "Depth Change" ),
              RifTextDataTableColumn( "Diam" ),
              RifTextDataTableColumn( "Rough", RifTextDataTableDoubleFormatting( RIF_FLOAT, 7 ) ) };
        formatter.header( header );
    }

    int segmentNumber = 2; // There's an implicit segment number 1.
    {
        formatter.comment( "Main Stem Segments" );
        std::shared_ptr<RicMswSegment> previousSegment;
        for ( std::shared_ptr<RicMswSegment> segment : exportInfo.segments() )
        {
            segment->setSegmentNumber( segmentNumber );

            if ( segment->subIndex() != cvf::UNDEFINED_SIZE_T )
            {
                QString comment = segment->label() + QString( ", sub %1" ).arg( segment->subIndex() );
                formatter.comment( comment );
            }

            writeMainBoreWelsegsSegment( segment, previousSegment, formatter, exportInfo, maxSegmentLength, &segmentNumber );
            previousSegment = segment;
        }
    }

    {
        generateWelsegsSegments( formatter,
                                 exportInfo,
                                 { RigCompletionData::FISHBONES_ICD, RigCompletionData::FISHBONES },
                                 maxSegmentLength,
                                 &segmentNumber );
        generateWelsegsSegments( formatter, exportInfo, { RigCompletionData::FRACTURE }, maxSegmentLength, &segmentNumber );
        generateWelsegsSegments( formatter,
                                 exportInfo,
                                 { RigCompletionData::PERFORATION_ICD,
                                   RigCompletionData::PERFORATION_ICV,
                                   RigCompletionData::PERFORATION_AICD },
                                 maxSegmentLength,
                                 &segmentNumber );
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWelsegsSegments(
    RifTextDataTableFormatter&                         formatter,
    const RicMswExportInfo&                            exportInfo,
    const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
    double                                             maxSegmentLength,
    int*                                               segmentNumber )
{
    bool generatedHeader = false;
    for ( std::shared_ptr<RicMswSegment> segment : exportInfo.segments() )
    {
        int mainSegmentNumber = segment->segmentNumber();
        segment->setSegmentNumber( mainSegmentNumber );
        for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
        {
            if ( exportCompletionTypes.count( completion->completionType() ) )
            {
                if ( !generatedHeader )
                {
                    generateWelsegsCompletionCommentHeader( formatter, completion->completionType() );
                    generatedHeader = true;
                }

                if ( RigCompletionData::isValve( completion->completionType() ) )
                {
                    writeValveWelsegsSegment( segment,
                                              std::dynamic_pointer_cast<RicMswValve>( completion ),
                                              formatter,
                                              exportInfo,
                                              maxSegmentLength,
                                              segmentNumber );
                }
                else
                {
                    writeCompletionWelsegsSegment( segment, completion, formatter, exportInfo, maxSegmentLength, segmentNumber );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWelsegsCompletionCommentHeader( RifTextDataTableFormatter& formatter,
                                                                                  RigCompletionData::CompletionType completionType )
{
    if ( completionType == RigCompletionData::CT_UNDEFINED )
    {
        formatter.comment( "Main stem" );
    }
    else if ( completionType == RigCompletionData::FISHBONES_ICD )
    {
        formatter.comment( "Fishbone Laterals" );
        formatter.comment( "Diam: MSW - Tubing Radius" );
        formatter.comment( "Rough: MSW - Open Hole Roughness Factor" );
    }
    else if ( RigCompletionData::isPerforationValve( completionType ) )
    {
        formatter.comment( "Perforation Valve Segments" );
        formatter.comment( "Diam: MSW - Tubing Radius" );
        formatter.comment( "Rough: MSW - Open Hole Roughness Factor" );
    }
    else if ( completionType == RigCompletionData::FRACTURE )
    {
        formatter.comment( "Fracture Segments" );
        formatter.comment( "Diam: MSW - Default Dummy" );
        formatter.comment( "Rough: MSW - Default Dummy" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateCompsegTables( RifTextDataTableFormatter& formatter,
                                                                 const RicMswExportInfo&    exportInfo )
{
    /*
     * TODO: Creating the regular perforation COMPSEGS table should come in here, before the others
     * should take precedence by appearing later in the output. See #3230.
     */

    {
        std::set<RigCompletionData::CompletionType> fishbonesTypes = { RigCompletionData::FISHBONES_ICD,
                                                                       RigCompletionData::FISHBONES };
        generateCompsegTable( formatter, exportInfo, false, fishbonesTypes );
        if ( exportInfo.hasSubGridIntersections() )
        {
            generateCompsegTable( formatter, exportInfo, true, fishbonesTypes );
        }
    }

    {
        std::set<RigCompletionData::CompletionType> fractureTypes = { RigCompletionData::FRACTURE };
        generateCompsegTable( formatter, exportInfo, false, fractureTypes );
        if ( exportInfo.hasSubGridIntersections() )
        {
            generateCompsegTable( formatter, exportInfo, true, fractureTypes );
        }
    }

    {
        std::set<RigCompletionData::CompletionType> perforationTypes = { RigCompletionData::PERFORATION,
                                                                         RigCompletionData::PERFORATION_ICD,
                                                                         RigCompletionData::PERFORATION_ICV,
                                                                         RigCompletionData::PERFORATION_AICD };
        generateCompsegTable( formatter, exportInfo, false, perforationTypes );
        if ( exportInfo.hasSubGridIntersections() )
        {
            generateCompsegTable( formatter, exportInfo, true, perforationTypes );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateCompsegTable(
    RifTextDataTableFormatter&                         formatter,
    const RicMswExportInfo&                            exportInfo,
    bool                                               exportSubGridIntersections,
    const std::set<RigCompletionData::CompletionType>& exportCompletionTypes )
{
    bool generatedHeader = false;

    for ( std::shared_ptr<RicMswSegment> segment : exportInfo.segments() )
    {
        double startMD = segment->startMD();
        double endMD   = segment->endMD();

        for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
        {
            if ( !completion->subSegments().empty() && exportCompletionTypes.count( completion->completionType() ) )
            {
                if ( !generatedHeader )
                {
                    generateCompsegHeader( formatter, exportInfo, completion->completionType(), exportSubGridIntersections );
                    generatedHeader = true;
                }

                for ( const std::shared_ptr<RicMswSubSegment>& subSegment : completion->subSegments() )
                {
                    if ( completion->completionType() == RigCompletionData::FISHBONES_ICD )
                    {
                        startMD = subSegment->startMD();
                        endMD   = subSegment->endMD();
                    }

                    for ( const std::shared_ptr<RicMswSubSegmentCellIntersection>& intersection :
                          subSegment->intersections() )
                    {
                        bool isSubGridIntersection = !intersection->gridName().isEmpty();
                        if ( isSubGridIntersection == exportSubGridIntersections )
                        {
                            double startLength = subSegment->startMD();
                            double endLength   = subSegment->endMD();
                            if ( completion->completionType() == RigCompletionData::PERFORATION_ICD ||
                                 completion->completionType() == RigCompletionData::PERFORATION_AICD ||
                                 completion->completionType() == RigCompletionData::PERFORATION_ICV )
                            {
                                startLength = startMD;
                                endLength   = endMD;
                            }

                            if ( exportSubGridIntersections )
                            {
                                formatter.add( intersection->gridName() );
                            }
                            cvf::Vec3st ijk = intersection->gridLocalCellIJK();
                            formatter.addOneBasedCellIndex( ijk.x() ).addOneBasedCellIndex( ijk.y() ).addOneBasedCellIndex(
                                ijk.z() );
                            formatter.add( completion->branchNumber() );

                            formatter.add( startLength );
                            formatter.add( endLength );

                            formatter.rowCompleted();
                        }
                    }
                }
            }
        }
    }
    if ( generatedHeader )
    {
        formatter.tableCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateCompsegHeader( RifTextDataTableFormatter&        formatter,
                                                                 const RicMswExportInfo&           exportInfo,
                                                                 RigCompletionData::CompletionType completionType,
                                                                 bool exportSubGridIntersections )
{
    if ( exportSubGridIntersections )
    {
        formatter.keyword( "COMPSEGL" );
    }
    else
    {
        formatter.keyword( "COMPSEGS" );
    }

    if ( completionType == RigCompletionData::FISHBONES_ICD )
    {
        formatter.comment( "Fishbones" );
    }
    else if ( completionType == RigCompletionData::FRACTURE )
    {
        formatter.comment( "Fractures" );
    }

    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "Name" ) };
        formatter.header( header );
        formatter.add( exportInfo.wellPath()->completionSettings()->wellNameForExport() );
        formatter.rowCompleted();
    }

    {
        std::vector<RifTextDataTableColumn> allHeaders;
        if ( exportSubGridIntersections )
        {
            allHeaders.push_back( RifTextDataTableColumn( "Grid" ) );
        }

        std::vector<RifTextDataTableColumn> commonHeaders = { RifTextDataTableColumn( "I" ),
                                                              RifTextDataTableColumn( "J" ),
                                                              RifTextDataTableColumn( "K" ),
                                                              RifTextDataTableColumn( "Branch no" ),
                                                              RifTextDataTableColumn( "Start Length" ),
                                                              RifTextDataTableColumn( "End Length" ),
                                                              RifTextDataTableColumn( "Dir Pen" ),
                                                              RifTextDataTableColumn( "End Range" ),
                                                              RifTextDataTableColumn( "Connection Depth" ) };
        allHeaders.insert( allHeaders.end(), commonHeaders.begin(), commonHeaders.end() );
        formatter.header( allHeaders );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWsegvalvTable( RifTextDataTableFormatter& formatter,
                                                                 const RicMswExportInfo&    exportInfo )
{
    bool foundValve = false;

    for ( std::shared_ptr<RicMswSegment> segment : exportInfo.segments() )
    {
        for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
        {
            if ( RigCompletionData::isWsegValveTypes( completion->completionType() ) )
            {
                if ( !foundValve )
                {
                    formatter.keyword( "WSEGVALV" );
                    std::vector<RifTextDataTableColumn> header = {
                        RifTextDataTableColumn( "Well Name" ),
                        RifTextDataTableColumn( "Seg No" ),
                        RifTextDataTableColumn( "Cv" ),
                        RifTextDataTableColumn( "Ac" ),
                    };
                    formatter.header( header );

                    foundValve = true;
                }

                std::shared_ptr<RicMswWsegValve> icd = std::static_pointer_cast<RicMswWsegValve>( completion );
                if ( !icd->subSegments().empty() )
                {
                    CVF_ASSERT( icd->subSegments().size() == 1u );

                    auto firstSubSegment = icd->subSegments().front();
                    if ( !firstSubSegment->intersections().empty() )
                    {
                        if ( icd->completionType() == RigCompletionData::PERFORATION_ICD ||
                             icd->completionType() == RigCompletionData::PERFORATION_ICV )
                        {
                            formatter.comment( icd->label() );
                        }
                        formatter.add( exportInfo.wellPath()->completionSettings()->wellNameForExport() );
                        formatter.add( firstSubSegment->segmentNumber() );
                        formatter.add( icd->flowCoefficient() );
                        formatter.add( QString( "%1" ).arg( icd->area(), 8, 'g', 4 ) );
                        formatter.rowCompleted();
                    }
                }
            }
        }
    }
    if ( foundValve )
    {
        formatter.tableCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWsegAicdTable( RifTextDataTableFormatter& formatter,
                                                                 const RicMswExportInfo&    exportInfo )
{
    RifTextDataTableFormatter tighterFormatter( formatter );
    tighterFormatter.setColumnSpacing( 1 );
    tighterFormatter.setTableRowPrependText( "   " );

    bool foundValve = false;

    for ( std::shared_ptr<RicMswSegment> segment : exportInfo.segments() )
    {
        for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::PERFORATION_AICD )
            {
                std::shared_ptr<RicMswPerforationAICD> aicd = std::static_pointer_cast<RicMswPerforationAICD>( completion );
                if ( aicd->isValid() )
                {
                    if ( !foundValve )
                    {
                        std::vector<QString> columnDescriptions =
                            { "Well Name",
                              "Segment Number",
                              "Segment Number",
                              "Strength of AICD",
                              "Flow Scaling Factor for AICD",
                              "Density of Calibration Fluid",
                              "Viscosity of Calibration Fluid",
                              "Critical water in liquid fraction for emulsions viscosity model",
                              "Emulsion viscosity transition region",
                              "Max ratio of emulsion viscosity to continuous phase viscosity",
                              "Flow scaling factor method",
                              "Maximum flowrate for AICD device",
                              "Volume flow rate exponent, x",
                              "Viscosity function exponent, y",
                              "Device OPEN/SHUT",
                              "Exponent of the oil flowing fraction in the density mixture calculation",
                              "Exponent of the water flowing fraction in the density mixture calculation",
                              "Exponent of the gas flowing fraction in the density mixture calculation",
                              "Exponent of the oil flowing fraction in the density viscosity calculation",
                              "Exponent of the water flowing fraction in the density viscosity calculation",
                              "Exponent of the gas flowing fraction in the density viscosity calculation" };

                        tighterFormatter.keyword( "WSEGAICD" );
                        tighterFormatter.comment( "Column Overview:" );
                        for ( size_t i = 0; i < columnDescriptions.size(); ++i )
                        {
                            tighterFormatter.comment(
                                QString( "%1: %2" ).arg( i + 1, 2, 10, QChar( '0' ) ).arg( columnDescriptions[i] ) );
                        }

                        std::vector<RifTextDataTableColumn> header;
                        for ( size_t i = 1; i <= 21; ++i )
                        {
                            QString                cName = QString( "%1" ).arg( i, 2, 10, QChar( '0' ) );
                            RifTextDataTableColumn col( cName,
                                                        RifTextDataTableDoubleFormatting(
                                                            RifTextDataTableDoubleFormat::RIF_CONSISE ),
                                                        RIGHT );
                            header.push_back( col );
                        }
                        tighterFormatter.header( header );

                        foundValve = true;
                    }
                    if ( !aicd->subSegments().empty() )
                    {
                        CVF_ASSERT( aicd->subSegments().size() == 1u );
                        tighterFormatter.comment( aicd->label() );
                        tighterFormatter.add( exportInfo.wellPath()->completionSettings()->wellNameForExport() ); // #1
                        tighterFormatter.add( aicd->subSegments().front()->segmentNumber() );
                        tighterFormatter.add( aicd->subSegments().front()->segmentNumber() );

                        std::array<double, AICD_NUM_PARAMS> values = aicd->values();
                        tighterFormatter.add( values[AICD_STRENGTH] );

                        tighterFormatter.add( aicd->flowScalingFactor() ); // #5 Flow scaling factor used when item #11
                                                                           // is set to '1'

                        tighterFormatter.add( values[AICD_DENSITY_CALIB_FLUID] );
                        tighterFormatter.add( values[AICD_VISCOSITY_CALIB_FLUID] );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_CRITICAL_WATER_IN_LIQUID_FRAC],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EMULSION_VISC_TRANS_REGION],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_MAX_RATIO_EMULSION_VISC],
                                                                  RicMswExportInfo::defaultDoubleValue() ); // #10

                        tighterFormatter.add( 1 ); // #11 : Always use method "b. Scale factor". The value of the scale
                                                   // factor is given in item #5

                        tighterFormatter.addValueOrDefaultMarker( values[AICD_MAX_FLOW_RATE],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.add( values[AICD_VOL_FLOW_EXP] );
                        tighterFormatter.add( values[AICD_VISOSITY_FUNC_EXP] );
                        tighterFormatter.add( aicd->isOpen() ? "OPEN" : "SHUT" ); // #15
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_OIL_FRAC_DENSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_WATER_FRAC_DENSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_GAS_FRAC_DENSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_OIL_FRAC_VISCOSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_WATER_FRAC_VISCOSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() ); // #20
                        tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_GAS_FRAC_VISCOSITY],
                                                                  RicMswExportInfo::defaultDoubleValue() );
                        tighterFormatter.rowCompleted();
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
    if ( foundValve )
    {
        tighterFormatter.tableCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RicWellPathExportMswCompletionsImpl::createSubSegmentMDPairs( double startMD, double endMD, double maxSegmentLength )
{
    int subSegmentCount = (int)( std::trunc( ( endMD - startMD ) / maxSegmentLength ) + 1 );

    double subSegmentLength = ( endMD - startMD ) / subSegmentCount;

    std::vector<std::pair<double, double>> subSegmentMDPairs;

    double subStartMD = startMD;
    double subEndMD   = startMD + subSegmentLength;
    for ( int i = 0; i < subSegmentCount; ++i )
    {
        subSegmentMDPairs.push_back( std::make_pair( subStartMD, subEndMD ) );
        subStartMD += subSegmentLength;
        subEndMD += std::min( subSegmentLength, endMD );
    }
    return subSegmentMDPairs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RicWellPathExportMswCompletionsImpl::calculateOverlapWithActiveCells(
    double                                           startMD,
    double                                           endMD,
    const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
    const RigActiveCellInfo*                         activeCellInfo )
{
    for ( const WellPathCellIntersectionInfo& intersection : wellPathIntersections )
    {
        if ( intersection.globCellIndex < activeCellInfo->reservoirCellCount() &&
             activeCellInfo->isActive( intersection.globCellIndex ) )
        {
            double overlapStart = std::max( startMD, intersection.startMD );
            double overlapEnd   = std::min( endMD, intersection.endMD );
            if ( overlapEnd > overlapStart )
            {
                return std::make_pair( overlapStart, overlapEnd );
            }
        }
    }
    return std::make_pair( 0.0, 0.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfo( const RimEclipseCase* caseToApply,
                                                                                      const RimWellPath*    wellPath,
                                                                                      bool enableSegmentSplitting )
{
    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs = wellPath->fishbonesCollection()->activeFishbonesSubs();

    return generateFishbonesMswExportInfo( caseToApply, wellPath, fishbonesSubs, enableSegmentSplitting );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfo(
    const RimEclipseCase*                         caseToApply,
    const RimWellPath*                            wellPath,
    const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs,
    bool                                          enableSegmentSplitting )
{
    RiaDefines::EclipseUnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 wellPath->fishbonesCollection()->startMD(),
                                 wellPath->fishbonesCollection()->mswParameters()->lengthAndDepth().text(),
                                 wellPath->fishbonesCollection()->mswParameters()->pressureDrop().text() );
    exportInfo.setLinerDiameter( wellPath->fishbonesCollection()->mswParameters()->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor( wellPath->fishbonesCollection()->mswParameters()->roughnessFactor( unitSystem ) );

    double maxSegmentLength          = enableSegmentSplitting
                                           ? wellPath->fishbonesCollection()->mswParameters()->maxSegmentLength()
                                           : std::numeric_limits<double>::infinity();
    bool   foundSubGridIntersections = false;

    double subStartMD  = wellPath->fishbonesCollection()->startMD();
    double subStartTVD = RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, subStartMD );

    for ( RimFishbonesMultipleSubs* subs : fishbonesSubs )
    {
        for ( auto& sub : subs->installedLateralIndices() )
        {
            double subEndMD  = subs->measuredDepth( sub.subIndex );
            double subEndTVD = RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, subEndMD );

            {
                std::shared_ptr<RicMswSegment> segment(
                    new RicMswSegment( subs->generatedName(), subStartMD, subEndMD, subStartTVD, subEndTVD, sub.subIndex ) );
                segment->setEffectiveDiameter( subs->effectiveDiameter( unitSystem ) );
                segment->setHoleDiameter( subs->holeDiameter( unitSystem ) );
                segment->setOpenHoleRoughnessFactor( subs->openHoleRoughnessFactor( unitSystem ) );
                segment->setSkinFactor( subs->skinFactor() );
                segment->setSourcePdmObject( subs );

                // Add completion for ICD
                std::shared_ptr<RicMswFishbonesICD> icdCompletion( new RicMswFishbonesICD( QString( "ICD" ), nullptr ) );
                std::shared_ptr<RicMswSubSegment>   icdSegment(
                    new RicMswSubSegment( subEndMD, subEndMD + 0.1, subEndTVD, subEndTVD ) );
                icdCompletion->setFlowCoefficient( subs->icdFlowCoefficient() );
                double icdOrificeRadius = subs->icdOrificeDiameter( unitSystem ) / 2;
                icdCompletion->setArea( icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount() );

                icdCompletion->addSubSegment( icdSegment );
                segment->addCompletion( icdCompletion );

                for ( size_t lateralIndex : sub.lateralIndices )
                {
                    QString label = QString( "Lateral %1" ).arg( lateralIndex );
                    segment->addCompletion( std::make_shared<RicMswFishbones>( label, lateralIndex ) );
                }
                assignFishbonesLateralIntersections( caseToApply,
                                                     wellPath,
                                                     subs,
                                                     segment,
                                                     &foundSubGridIntersections,
                                                     maxSegmentLength );

                exportInfo.addSegment( segment );
            }

            subStartMD  = subEndMD;
            subStartTVD = subEndTVD;
        }
    }
    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortSegments();

    assignBranchNumbers( caseToApply, &exportInfo );

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportMswCompletionsImpl::generateFracturesMswExportInfo( RimEclipseCase*    caseToApply,
                                                                                      const RimWellPath* wellPath )
{
    std::vector<RimWellPathFracture*> fractures = wellPath->fractureCollection()->activeFractures();

    return generateFracturesMswExportInfo( caseToApply, wellPath, fractures );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo
    RicWellPathExportMswCompletionsImpl::generateFracturesMswExportInfo( RimEclipseCase*    caseToApply,
                                                                         const RimWellPath* wellPath,
                                                                         const std::vector<RimWellPathFracture*>& fractures )
{
    const RigMainGrid*       grid = caseToApply->eclipseCaseData()->mainGrid();
    const RigActiveCellInfo* activeCellInfo =
        caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RiaDefines::EclipseUnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->measuredDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseToApply->eclipseCaseData(),
                                                                          wellPath->name(),
                                                                          coords,
                                                                          mds );

    double initialMD = 0.0;
    if ( wellPath->fractureCollection()->mswParameters()->referenceMDType() ==
         RimMswCompletionParameters::MANUAL_REFERENCE_MD )
    {
        initialMD = wellPath->fractureCollection()->mswParameters()->manualReferenceMD();
    }
    else
    {
        for ( WellPathCellIntersectionInfo intersection : intersections )
        {
            if ( activeCellInfo->isActive( intersection.globCellIndex ) )
            {
                initialMD = intersection.startMD;
                break;
            }
        }

        double startOfFirstCompletion = std::numeric_limits<double>::infinity();
        {
            for ( auto* fracture : fractures )
            {
                if ( fracture->isEnabled() && fracture->startMD() < startOfFirstCompletion )
                {
                    startOfFirstCompletion = fracture->startMD();
                }
            }
        }

        // Initial MD is the lowest MD based on grid intersection and start of fracture completions
        // https://github.com/OPM/ResInsight/issues/6071
        initialMD = std::min( initialMD, startOfFirstCompletion );
    }

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 wellPath->fractureCollection()->mswParameters()->lengthAndDepth().text(),
                                 wellPath->fractureCollection()->mswParameters()->pressureDrop().text() );

    exportInfo.setLinerDiameter( wellPath->fractureCollection()->mswParameters()->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor( wellPath->fractureCollection()->mswParameters()->roughnessFactor( unitSystem ) );

    bool foundSubGridIntersections = false;

    // Main bore
    int mainBoreSegment = 1;
    for ( const auto& cellIntInfo : intersections )
    {
        size_t             localGridIdx = 0u;
        const RigGridBase* localGrid =
            grid->gridAndGridLocalIdxFromGlobalCellIdx( cellIntInfo.globCellIndex, &localGridIdx );
        QString gridName;
        if ( localGrid != grid )
        {
            gridName                  = QString::fromStdString( localGrid->gridName() );
            foundSubGridIntersections = true;
        }

        size_t i = 0u, j = 0u, k = 0u;
        localGrid->ijkFromCellIndex( localGridIdx, &i, &j, &k );
        QString                        label = QString( "Main stem segment %1" ).arg( ++mainBoreSegment );
        std::shared_ptr<RicMswSegment> segment(
            new RicMswSegment( label, cellIntInfo.startMD, cellIntInfo.endMD, cellIntInfo.startTVD(), cellIntInfo.endTVD() ) );

        // Check if fractures are to be assigned to current main bore segment
        for ( RimWellPathFracture* fracture : fractures )
        {
            double fractureStartMD = fracture->fractureMD();
            if ( fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
            {
                double perforationLength = fracture->fractureTemplate()->perforationLength();
                fractureStartMD -= 0.5 * perforationLength;
            }

            if ( cvf::Math::valueInRange( fractureStartMD, cellIntInfo.startMD, cellIntInfo.endMD ) )
            {
                std::vector<RigCompletionData> completionData =
                    RicExportFractureCompletionsImpl::generateCompdatValues( caseToApply,
                                                                             wellPath->completionSettings()->wellNameForExport(),
                                                                             wellPath->wellPathGeometry(),
                                                                             { fracture },
                                                                             nullptr,
                                                                             nullptr );

                assignFractureCompletionsToCellSegment( caseToApply, fracture, completionData, segment, &foundSubGridIntersections );
            }
        }

        exportInfo.addSegment( segment );
    }
    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortSegments();
    assignBranchNumbers( caseToApply, &exportInfo );

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportMswCompletionsImpl::generatePerforationsMswExportInfo(
    RimEclipseCase*                                   eclipseCase,
    const RimWellPath*                                wellPath,
    int                                               timeStep,
    const std::vector<const RimPerforationInterval*>& perforationIntervals )
{
    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    const RigActiveCellInfo* activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    std::vector<WellPathCellIntersectionInfo> intersections = generateCellSegments( eclipseCase, wellPath, initialMD );
    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( intersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 wellPath->perforationIntervalCollection()->mswParameters()->lengthAndDepth().text(),
                                 wellPath->perforationIntervalCollection()->mswParameters()->pressureDrop().text() );

    exportInfo.setLinerDiameter( wellPath->perforationIntervalCollection()->mswParameters()->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor(
        wellPath->perforationIntervalCollection()->mswParameters()->roughnessFactor( unitSystem ) );

    bool foundSubGridIntersections = false;

    MainBoreSegments mainBoreSegments = createMainBoreSegmentsForPerforations( filteredIntersections,
                                                                               perforationIntervals,
                                                                               wellPath,
                                                                               timeStep,
                                                                               eclipseCase,
                                                                               &foundSubGridIntersections );

    createValveCompletions( mainBoreSegments, perforationIntervals, wellPath, unitSystem );
    assignValveContributionsToSuperICDsOrAICDs( mainBoreSegments,
                                                perforationIntervals,
                                                filteredIntersections,
                                                activeCellInfo,
                                                unitSystem );
    moveIntersectionsToICVs( mainBoreSegments, perforationIntervals, unitSystem );
    moveIntersectionsToSuperICDsOrAICDs( mainBoreSegments );

    for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
    {
        exportInfo.addSegment( segment );
    }

    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortSegments();
    assignBranchNumbers( eclipseCase, &exportInfo );

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::generateCellSegments( const RimEclipseCase* eclipseCase,
                                                               const RimWellPath*    wellPath,
                                                               double&               initialMD )
{
    const RigActiveCellInfo* activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->measuredDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    std::vector<WellPathCellIntersectionInfo> allIntersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                          wellPath->name(),
                                                                          coords,
                                                                          mds );
    std::vector<WellPathCellIntersectionInfo> continuousIntersections =
        RigWellPathIntersectionTools::buildContinuousIntersections( allIntersections, mainGrid );

    if ( wellPath->perforationIntervalCollection()->mswParameters()->referenceMDType() ==
         RimMswCompletionParameters::MANUAL_REFERENCE_MD )
    {
        initialMD = wellPath->perforationIntervalCollection()->mswParameters()->manualReferenceMD();
    }
    else
    {
        for ( const WellPathCellIntersectionInfo& intersection : continuousIntersections )
        {
            if ( activeCellInfo->isActive( intersection.globCellIndex ) )
            {
                initialMD = intersection.startMD;
                break;
            }
        }

        double startOfFirstCompletion = std::numeric_limits<double>::infinity();
        {
            std::vector<const RimWellPathComponentInterface*> allCompletions = wellPath->completions()->allCompletions();

            for ( const RimWellPathComponentInterface* completion : allCompletions )
            {
                if ( completion->isEnabled() && completion->startMD() < startOfFirstCompletion )
                {
                    startOfFirstCompletion = completion->startMD();
                }
            }
        }

        // Initial MD is the lowest MD based on grid intersection and start of fracture completions
        // https://github.com/OPM/ResInsight/issues/6071
        initialMD = std::min( initialMD, startOfFirstCompletion );
    }

    return continuousIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                              double                               initialMD,
                                                              gsl::not_null<const RigWellPath*>    wellPathGeometry,
                                                              gsl::not_null<const RimEclipseCase*> eclipseCase )
{
    std::vector<WellPathCellIntersectionInfo> filteredIntersections;

    if ( !intersections.empty() && intersections[0].startMD > initialMD )
    {
        WellPathCellIntersectionInfo firstIntersection = intersections[0];

        // Add a segment from user defined MD to start of grid
        cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

        WellPathCellIntersectionInfo extraIntersection;

        extraIntersection.globCellIndex         = std::numeric_limits<size_t>::max();
        extraIntersection.startPoint            = intersectionPoint;
        extraIntersection.endPoint              = firstIntersection.startPoint;
        extraIntersection.startMD               = initialMD;
        extraIntersection.endMD                 = firstIntersection.startMD;
        extraIntersection.intersectedCellFaceIn = cvf::StructGridInterface::NO_FACE;
        extraIntersection.intersectedCellFaceOut =
            cvf::StructGridInterface::oppositeFace( firstIntersection.intersectedCellFaceIn );
        extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;

        filteredIntersections.push_back( extraIntersection );
    }

    const double epsilon = 1.0e-3;

    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        if ( ( intersection.endMD - initialMD ) < epsilon )
        {
            // Skip all intersections before initial measured depth
            continue;
        }
        else if ( ( intersection.startMD - initialMD ) > epsilon )
        {
            filteredIntersections.push_back( intersection );
        }
        else
        {
            // InitialMD is inside intersection, split based on intersection point

            cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

            WellPathCellIntersectionInfo extraIntersection;

            extraIntersection.globCellIndex          = intersection.globCellIndex;
            extraIntersection.startPoint             = intersectionPoint;
            extraIntersection.endPoint               = intersection.endPoint;
            extraIntersection.startMD                = initialMD;
            extraIntersection.endMD                  = intersection.endMD;
            extraIntersection.intersectedCellFaceIn  = cvf::StructGridInterface::NO_FACE;
            extraIntersection.intersectedCellFaceOut = intersection.intersectedCellFaceOut;

            const RigMainGrid* grid = eclipseCase->mainGrid();

            if ( intersection.globCellIndex < grid->cellCount() )
            {
                extraIntersection.intersectionLengthsInCellCS =
                    RigWellPathIntersectionTools::calculateLengthInCell( grid,
                                                                         intersection.globCellIndex,
                                                                         intersectionPoint,
                                                                         intersection.endPoint );
            }
            else
            {
                extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;
            }

            filteredIntersections.push_back( extraIntersection );
        }
    }

    return filteredIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathExportMswCompletionsImpl::MainBoreSegments
    RicWellPathExportMswCompletionsImpl::createMainBoreSegmentsForPerforations(
        const std::vector<WellPathCellIntersectionInfo>&  cellSegmentIntersections,
        const std::vector<const RimPerforationInterval*>& perforationIntervals,
        const RimWellPath*                                wellPath,
        int                                               timeStep,
        RimEclipseCase*                                   eclipseCase,
        bool*                                             foundSubGridIntersections )
{
    MainBoreSegments mainBoreSegments;

    // Intersections along the well path with grid geometry is handled by well log extraction tools.
    // The threshold in RigWellLogExtractionTools::isEqualDepth is currently set to 0.1m, and this
    // is a pretty large threshold based on the indicated threshold of 0.001m for MSW segments
    const double segmentLengthThreshold = 1.0e-3;

    for ( const auto& cellIntInfo : cellSegmentIntersections )
    {
        const double segmentLength = std::fabs( cellIntInfo.endMD - cellIntInfo.startMD );

        if ( segmentLength > segmentLengthThreshold )
        {
            QString                        label = QString( "Main stem segment %1" ).arg( mainBoreSegments.size() + 2 );
            std::shared_ptr<RicMswSegment> segment( new RicMswSegment( label,
                                                                       cellIntInfo.startMD,
                                                                       cellIntInfo.endMD,
                                                                       cellIntInfo.startTVD(),
                                                                       cellIntInfo.endTVD() ) );

            for ( const RimPerforationInterval* interval : perforationIntervals )
            {
                double overlapStart = std::max( interval->startMD(), segment->startMD() );
                double overlapEnd   = std::min( interval->endMD(), segment->endMD() );
                double overlap      = std::max( 0.0, overlapEnd - overlapStart );
                if ( overlap > 0.0 )
                {
                    std::shared_ptr<RicMswCompletion> intervalCompletion( new RicMswPerforation( interval->name() ) );
                    std::vector<RigCompletionData>    completionData =
                        generatePerforationIntersections( wellPath, interval, timeStep, eclipseCase );
                    assignPerforationIntersections( completionData,
                                                    intervalCompletion,
                                                    cellIntInfo,
                                                    overlapStart,
                                                    overlapEnd,
                                                    foundSubGridIntersections );
                    segment->addCompletion( intervalCompletion );
                }
            }
            mainBoreSegments.push_back( segment );
        }
        else
        {
            QString text =
                QString( "Skipping segment , threshold = %1, length = %2" ).arg( segmentLengthThreshold ).arg( segmentLength );
            RiaLogging::info( text );
        }
    }
    return mainBoreSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::createValveCompletions(
    std::vector<std::shared_ptr<RicMswSegment>>&      mainBoreSegments,
    const std::vector<const RimPerforationInterval*>& perforationIntervals,
    const RimWellPath*                                wellPath,
    RiaDefines::EclipseUnitSystem                     unitSystem )
{
    for ( size_t nMainSegment = 0u; nMainSegment < mainBoreSegments.size(); ++nMainSegment )
    {
        std::shared_ptr<RicMswSegment> segment = mainBoreSegments[nMainSegment];

        std::shared_ptr<RicMswPerforationICV>  ICV;
        std::shared_ptr<RicMswPerforationICD>  superICD;
        std::shared_ptr<RicMswPerforationAICD> superAICD;

        double totalICDOverlap  = 0.0;
        double totalAICDOverlap = 0.0;

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType( perforationValves );

            for ( const RimWellPathValve* valve : perforationValves )
            {
                if ( !valve->isChecked() ) continue;

                for ( size_t nSubValve = 0u; nSubValve < valve->valveLocations().size(); ++nSubValve )
                {
                    double valveMD = valve->valveLocations()[nSubValve];

                    std::pair<double, double> valveSegment = valve->valveSegments()[nSubValve];
                    double                    overlapStart = std::max( valveSegment.first, segment->startMD() );
                    double                    overlapEnd   = std::min( valveSegment.second, segment->endMD() );
                    double                    overlap      = std::max( 0.0, overlapEnd - overlapStart );

                    // "Dummy" values for the new branch created for the valve.
                    // Will be added to the main segments start MD,
                    double exportStartMD = 0.0;
                    double exportEndMD   = 0.1;

                    double exportStartTVD =
                        RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, valveMD + exportStartMD );
                    double exportEndTVD =
                        RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, valveMD + exportEndMD );

                    double overlapStartTVD =
                        RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, overlapStart );
                    double overlapEndTVD =
                        RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( wellPath, overlapEnd );

                    if ( segment->startMD() <= valveMD && valveMD < segment->endMD() )
                    {
                        if ( valve->componentType() == RiaDefines::WellPathComponentType::AICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto subSegment =
                                std::make_shared<RicMswSubSegment>( exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            superAICD = std::make_shared<RicMswPerforationAICD>( valveLabel, valve );
                            superAICD->addSubSegment( subSegment );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto subSegment =
                                std::make_shared<RicMswSubSegment>( exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            superICD = std::make_shared<RicMswPerforationICD>( valveLabel, valve );
                            superICD->addSubSegment( subSegment );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICV )
                        {
                            QString valveLabel =
                                QString( "ICV %1 at segment #%2" ).arg( valve->name() ).arg( nMainSegment + 2 );
                            auto subSegment =
                                std::make_shared<RicMswSubSegment>( exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            ICV = std::make_shared<RicMswPerforationICV>( valveLabel, valve );
                            ICV->addSubSegment( subSegment );
                            ICV->setFlowCoefficient( valve->flowCoefficient() );
                            double orificeRadius = valve->orificeDiameter( unitSystem ) / 2;
                            ICV->setArea( orificeRadius * orificeRadius * cvf::PI_D );
                        }
                    }
                    else if ( overlap > 0.0 &&
                              ( valve->componentType() == RiaDefines::WellPathComponentType::ICD && !superICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment =
                            std::make_shared<RicMswSubSegment>( exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );
                        superICD = std::make_shared<RicMswPerforationICD>( valveLabel, valve );
                        superICD->addSubSegment( subSegment );
                    }
                    else if ( overlap > 0.0 &&
                              ( valve->componentType() == RiaDefines::WellPathComponentType::AICD && !superAICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment =
                            std::make_shared<RicMswSubSegment>( exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );
                        superAICD = std::make_shared<RicMswPerforationAICD>( valveLabel, valve );
                        superAICD->addSubSegment( subSegment );
                    }

                    if ( valve->componentType() == RiaDefines::WellPathComponentType::AICD )
                    {
                        totalAICDOverlap += overlap;
                    }
                    else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD )
                    {
                        totalICDOverlap += overlap;
                    }
                }
            }
        }

        if ( ICV )
        {
            segment->addCompletion( ICV );
        }
        else
        {
            if ( totalICDOverlap > 0.0 || totalAICDOverlap > 0.0 )
            {
                if ( totalAICDOverlap > totalICDOverlap )
                {
                    segment->addCompletion( superAICD );
                }
                else
                {
                    segment->addCompletion( superICD );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignValveContributionsToSuperICDsOrAICDs(
    const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
    const std::vector<const RimPerforationInterval*>&  perforationIntervals,
    const std::vector<WellPathCellIntersectionInfo>&   wellPathIntersections,
    const RigActiveCellInfo*                           activeCellInfo,
    RiaDefines::EclipseUnitSystem                      unitSystem )
{
    ValveContributionMap assignedRegularValves;

    std::map<std::shared_ptr<RicMswSegment>, std::shared_ptr<RicMswValveAccumulator>> accumulators;

    for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
    {
        std::shared_ptr<RicMswValve> superValve;
        for ( auto completion : segment->completions() )
        {
            std::shared_ptr<RicMswValve> valve = std::dynamic_pointer_cast<RicMswValve>( completion );
            if ( valve )
            {
                superValve = valve;
                break;
            }
        }
        if ( std::dynamic_pointer_cast<const RicMswPerforationICD>( superValve ) )
        {
            accumulators[segment] = std::make_shared<RicMswICDAccumulator>( superValve, unitSystem );
        }
        else if ( std::dynamic_pointer_cast<const RicMswPerforationAICD>( superValve ) )
        {
            accumulators[segment] = std::make_shared<RicMswAICDAccumulator>( superValve, unitSystem );
        }
    }

    for ( const RimPerforationInterval* interval : perforationIntervals )
    {
        if ( !interval->isChecked() ) continue;

        std::vector<const RimWellPathValve*> perforationValves;
        interval->descendantsIncludingThisOfType( perforationValves );

        double totalPerforationLength = 0.0;
        for ( const RimWellPathValve* valve : perforationValves )
        {
            if ( !valve->isChecked() ) continue;

            for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
            {
                double intervalOverlapStart           = std::max( interval->startMD(), segment->startMD() );
                double intervalOverlapEnd             = std::min( interval->endMD(), segment->endMD() );
                auto   intervalOverlapWithActiveCells = calculateOverlapWithActiveCells( intervalOverlapStart,
                                                                                       intervalOverlapEnd,
                                                                                       wellPathIntersections,
                                                                                       activeCellInfo );

                totalPerforationLength += intervalOverlapWithActiveCells.second - intervalOverlapWithActiveCells.first;
            }
        }

        for ( const RimWellPathValve* valve : perforationValves )
        {
            if ( !valve->isChecked() ) continue;

            for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
            {
                double intervalOverlapStart = std::max( interval->startMD(), segment->startMD() );
                double intervalOverlapEnd   = std::min( interval->endMD(), segment->endMD() );

                auto intervalOverlapWithActiveCells = calculateOverlapWithActiveCells( intervalOverlapStart,
                                                                                       intervalOverlapEnd,
                                                                                       wellPathIntersections,
                                                                                       activeCellInfo );

                double overlapLength = intervalOverlapWithActiveCells.second - intervalOverlapWithActiveCells.first;
                if ( overlapLength > 0.0 )
                {
                    auto it = accumulators.find( segment );

                    if ( it != accumulators.end() )
                    {
                        it->second->accumulateValveParameters( valve, overlapLength, totalPerforationLength );
                        assignedRegularValves[it->second->superValve()].push_back( valve );
                    }
                }
            }
        }
    }

    for ( auto accumulator : accumulators )
    {
        accumulator.second->applyToSuperValve();
    }

    for ( auto regularValvePair : assignedRegularValves )
    {
        if ( !regularValvePair.second.empty() )
        {
            QStringList valveLabels;
            for ( const RimWellPathValve* regularValve : regularValvePair.second )
            {
                QString valveLabel = QString( "%1" ).arg( regularValve->name() );
                valveLabels.push_back( valveLabel );
            }
            QString valveContribLabel = QString( " with contribution from: %1" ).arg( valveLabels.join( ", " ) );
            regularValvePair.first->setLabel( regularValvePair.first->label() + valveContribLabel );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::moveIntersectionsToICVs(
    const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
    const std::vector<const RimPerforationInterval*>&  perforationIntervals,
    RiaDefines::EclipseUnitSystem                      unitSystem )
{
    std::map<const RimWellPathValve*, std::shared_ptr<RicMswPerforationICV>> icvCompletionMap;

    for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
    {
        for ( auto completion : segment->completions() )
        {
            std::shared_ptr<RicMswPerforationICV> icv = std::dynamic_pointer_cast<RicMswPerforationICV>( completion );
            if ( icv )
            {
                icvCompletionMap[icv->wellPathValve()] = icv;
            }
        }
    }

    for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
    {
        std::vector<std::shared_ptr<RicMswCompletion>> perforations;
        for ( auto completionPtr : segment->completions() )
        {
            if ( completionPtr->completionType() == RigCompletionData::PERFORATION )
            {
                perforations.push_back( completionPtr );
            }
        }

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType( perforationValves );

            for ( const RimWellPathValve* valve : perforationValves )
            {
                if ( !valve->isChecked() ) continue;
                if ( valve->componentType() != RiaDefines::WellPathComponentType::ICV ) continue;

                auto icvIt = icvCompletionMap.find( valve );
                if ( icvIt == icvCompletionMap.end() ) continue;

                std::shared_ptr<RicMswPerforationICV> icvCompletion = icvIt->second;
                CVF_ASSERT( icvCompletion );

                std::pair<double, double> valveSegment = valve->valveSegments().front();
                double                    overlapStart = std::max( valveSegment.first, segment->startMD() );
                double                    overlapEnd   = std::min( valveSegment.second, segment->endMD() );
                double                    overlap      = std::max( 0.0, overlapEnd - overlapStart );

                if ( overlap > 0.0 )
                {
                    CVF_ASSERT( icvCompletion->subSegments().size() == 1u );
                    for ( auto perforationPtr : perforations )
                    {
                        for ( auto subSegmentPtr : perforationPtr->subSegments() )
                        {
                            for ( auto intersectionPtr : subSegmentPtr->intersections() )
                            {
                                icvCompletion->subSegments()[0]->addIntersection( intersectionPtr );
                            }
                        }
                        segment->removeCompletion( perforationPtr );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::writeMainBoreWelsegsSegment( std::shared_ptr<RicMswSegment> segment,
                                                                       std::shared_ptr<RicMswSegment> previousSegment,
                                                                       RifTextDataTableFormatter&     formatter,
                                                                       const RicMswExportInfo&        exportInfo,
                                                                       double                         maxSegmentLength,
                                                                       int*                           segmentNumber )
{
    CVF_ASSERT( segment && segmentNumber );

    double startMD = segment->startMD();
    double endMD   = segment->endMD();

    std::vector<std::pair<double, double>> subSegments = createSubSegmentMDPairs( startMD, endMD, maxSegmentLength );

    CVF_ASSERT( exportInfo.wellPath() );
    auto wellPathGeometry = exportInfo.wellPath()->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    double prevOutMD  = exportInfo.initialMD();
    double prevOutTVD = exportInfo.initialTVD();
    if ( previousSegment )
    {
        prevOutMD  = previousSegment->outputMD();
        prevOutTVD = previousSegment->outputTVD();
    }
    for ( const auto& [subStartMD, subEndMD] : subSegments )
    {
        auto startPoint = wellPathGeometry->interpolatedPointAlongWellPath( subStartMD );
        auto endPoint   = wellPathGeometry->interpolatedPointAlongWellPath( subEndMD );

        double subStartTVD = -startPoint.z();
        double subEndTVD   = -endPoint.z();

        double depth  = 0;
        double length = 0;

        double midPointMD  = 0.5 * ( subStartMD + subEndMD );
        double midPointTVD = 0.5 * ( subStartTVD + subEndTVD );

        if ( exportInfo.lengthAndDepthText() == QString( "INC" ) )
        {
            depth  = midPointTVD - prevOutTVD;
            length = midPointMD - prevOutMD;
        }
        else
        {
            depth  = midPointTVD;
            length = midPointMD;
        }
        segment->setOutputMD( midPointMD );
        segment->setOutputTVD( midPointTVD );

        formatter.add( *segmentNumber ).add( *segmentNumber );
        formatter.add( 1 ); // All segments on main stem are branch 1
        formatter.add( *segmentNumber - 1 ); // All main stem segments are connected to the segment below
                                             // them
        formatter.add( length );
        formatter.add( depth );
        formatter.add( exportInfo.linerDiameter() );
        formatter.add( exportInfo.roughnessFactor() );
        formatter.rowCompleted();
        ( *segmentNumber )++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::writeValveWelsegsSegment( std::shared_ptr<RicMswSegment> segment,
                                                                    std::shared_ptr<RicMswValve>   valve,
                                                                    RifTextDataTableFormatter&     formatter,
                                                                    const RicMswExportInfo&        exportInfo,
                                                                    double                         maxSegmentLength,
                                                                    int*                           segmentNumber )
{
    CVF_ASSERT( valve );
    if ( !valve->isValid() ) return;

    formatter.comment( valve->label() );

    auto subSegment = valve->subSegments().front();
    subSegment->setSegmentNumber( *segmentNumber );

    double startMD = subSegment->startMD();
    double endMD   = subSegment->endMD();

    if ( valve->completionType() != RigCompletionData::FISHBONES_ICD )
    {
        startMD += segment->outputMD();
        endMD += segment->outputMD();
    }

    std::vector<std::pair<double, double>> splitSegments = createSubSegmentMDPairs( startMD, endMD, maxSegmentLength );

    CVF_ASSERT( exportInfo.wellPath() );
    auto wellPathGeometry = exportInfo.wellPath()->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    for ( const auto& [subStartMD, subEndMD] : splitSegments )
    {
        int subSegmentNumber = ( *segmentNumber )++;

        auto startPoint = wellPathGeometry->interpolatedPointAlongWellPath( subStartMD );
        auto endPoint   = wellPathGeometry->interpolatedPointAlongWellPath( subEndMD );

        double subStartTVD = -startPoint.z();
        double subEndTVD   = -endPoint.z();

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
        formatter.add( subSegmentNumber );
        formatter.add( subSegmentNumber );
        formatter.add( valve->branchNumber() );
        formatter.add( segment->segmentNumber() );

        formatter.add( length );
        formatter.add( depth );
        formatter.add( exportInfo.linerDiameter() );
        formatter.add( exportInfo.roughnessFactor() );
        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::writeCompletionWelsegsSegment( std::shared_ptr<RicMswSegment>    segment,
                                                                         std::shared_ptr<RicMswCompletion> completion,
                                                                         RifTextDataTableFormatter&        formatter,
                                                                         const RicMswExportInfo&           exportInfo,
                                                                         double maxSegmentLength,
                                                                         int*   segmentNumber )
{
    if ( completion->completionType() == RigCompletionData::FISHBONES )
    {
        formatter.comment(
            QString( "%1 : Sub index %2 - %3" ).arg( segment->label() ).arg( segment->subIndex() ).arg( completion->label() ) );
    }
    else if ( completion->completionType() == RigCompletionData::FRACTURE )
    {
        formatter.comment( QString( "%1 connected to %2" ).arg( completion->label() ).arg( segment->label() ) );
    }

    CVF_ASSERT( exportInfo.wellPath() );
    auto wellPathGeometry = exportInfo.wellPath()->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    for ( std::shared_ptr<RicMswSubSegment> subSegment : completion->subSegments() )
    {
        double startMD = subSegment->startMD();
        double endMD   = subSegment->endMD();

        std::vector<std::pair<double, double>> splitSegments = createSubSegmentMDPairs( startMD, endMD, maxSegmentLength );

        for ( const auto& [subStartMD, subEndMD] : splitSegments )
        {
            int subSegmentNumber = ( *segmentNumber )++;

            auto startPoint = wellPathGeometry->interpolatedPointAlongWellPath( subStartMD );
            auto endPoint   = wellPathGeometry->interpolatedPointAlongWellPath( subEndMD );

            double subStartTVD = -startPoint.z();
            double subEndTVD   = -endPoint.z();

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
            double diameter = segment->effectiveDiameter();
            formatter.add( subSegmentNumber );
            formatter.add( subSegmentNumber );
            formatter.add( completion->branchNumber() );
            formatter.add( segment->segmentNumber() );
            formatter.add( length );
            formatter.add( depth );
            formatter.add( diameter );
            formatter.add( segment->openHoleRoughnessFactor() );
            formatter.rowCompleted();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::moveIntersectionsToSuperICDsOrAICDs( MainBoreSegments mainBoreSegments )
{
    for ( auto segmentPtr : mainBoreSegments )
    {
        std::shared_ptr<RicMswCompletion>              superValve;
        std::vector<std::shared_ptr<RicMswCompletion>> perforations;
        for ( auto completionPtr : segmentPtr->completions() )
        {
            if ( RigCompletionData::isPerforationValve( completionPtr->completionType() ) )
            {
                superValve = completionPtr;
            }
            else
            {
                CVF_ASSERT( completionPtr->completionType() == RigCompletionData::PERFORATION );
                perforations.push_back( completionPtr );
            }
        }

        if ( superValve == nullptr ) continue;

        CVF_ASSERT( superValve->subSegments().size() == 1u );
        segmentPtr->completions().clear();
        segmentPtr->addCompletion( superValve );
        for ( auto perforationPtr : perforations )
        {
            for ( auto subSegmentPtr : perforationPtr->subSegments() )
            {
                for ( auto intersectionPtr : subSegmentPtr->intersections() )
                {
                    superValve->subSegments()[0]->addIntersection( intersectionPtr );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignFishbonesLateralIntersections( const RimEclipseCase* caseToApply,
                                                                               const RimWellPath*    wellPath,
                                                                               const RimFishbonesMultipleSubs* fishbonesSubs,
                                                                               std::shared_ptr<RicMswSegment> segment,
                                                                               bool*  foundSubGridIntersections,
                                                                               double maxSegmentLength )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();

    for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
    {
        if ( completion->completionType() != RigCompletionData::FISHBONES )
        {
            continue;
        }

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            fishbonesSubs->coordsAndMDForLateral( segment->subIndex(), completion->index() );

        if ( lateralCoordMDPairs.empty() )
        {
            continue;
        }

        std::vector<cvf::Vec3d> lateralCoords;
        std::vector<double>     lateralMDs;

        lateralCoords.reserve( lateralCoordMDPairs.size() );
        lateralMDs.reserve( lateralCoordMDPairs.size() );

        for ( auto& coordMD : lateralCoordMDPairs )
        {
            lateralCoords.push_back( coordMD.first );
            lateralMDs.push_back( coordMD.second );
        }

        std::vector<WellPathCellIntersectionInfo> intersections =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseToApply->eclipseCaseData(),
                                                                              wellPath->name(),
                                                                              lateralCoords,
                                                                              lateralMDs );

        RigWellPath pathGeometry( lateralCoords, lateralMDs );

        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = -lateralCoords.front().z();

        for ( const auto& cellIntInfo : intersections )
        {
            size_t             localGridIdx = 0u;
            const RigGridBase* localGrid =
                grid->gridAndGridLocalIdxFromGlobalCellIdx( cellIntInfo.globCellIndex, &localGridIdx );
            QString gridName;
            if ( localGrid != grid )
            {
                gridName                   = QString::fromStdString( localGrid->gridName() );
                *foundSubGridIntersections = true;
            }

            size_t i = 0u, j = 0u, k = 0u;
            localGrid->ijkFromCellIndex( localGridIdx, &i, &j, &k );
            std::shared_ptr<RicMswSubSegment> subSegment(
                new RicMswSubSegment( previousExitMD, cellIntInfo.endMD, previousExitTVD, cellIntInfo.endTVD() ) );

            std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(
                new RicMswSubSegmentCellIntersection( gridName,
                                                      cellIntInfo.globCellIndex,
                                                      cvf::Vec3st( i, j, k ),
                                                      cellIntInfo.intersectionLengthsInCellCS ) );
            subSegment->addIntersection( intersection );
            completion->addSubSegment( subSegment );

            previousExitMD  = cellIntInfo.endMD;
            previousExitTVD = cellIntInfo.endTVD();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignFractureCompletionsToCellSegment( const RimEclipseCase* caseToApply,
                                                                                  const RimWellPathFracture* fracture,
                                                                                  const std::vector<RigCompletionData>& completionData,
                                                                                  std::shared_ptr<RicMswSegment> segment,
                                                                                  bool* foundSubGridIntersections )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    std::shared_ptr<RicMswFracture> fractureCompletion( new RicMswFracture( fracture->name() ) );
    double                          position = fracture->fractureMD();
    double                          width    = fracture->fractureTemplate()->computeFractureWidth( fracture );

    if ( fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
    {
        double perforationLength = fracture->fractureTemplate()->perforationLength();
        position -= 0.5 * perforationLength;
        width = perforationLength;
    }

    std::shared_ptr<RicMswSubSegment> subSegment( new RicMswSubSegment( position, position + width, 0.0, 0.0 ) );
    for ( const RigCompletionData& compIntersection : completionData )
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        cvf::Vec3st localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

        std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(
            new RicMswSubSegmentCellIntersection( cell.lgrName(), cell.globalCellIndex(), localIJK, cvf::Vec3d::ZERO ) );
        subSegment->addIntersection( intersection );
    }
    fractureCompletion->addSubSegment( subSegment );
    segment->addCompletion( fractureCompletion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportMswCompletionsImpl::generatePerforationIntersections(
    gsl::not_null<const RimWellPath*>            wellPath,
    gsl::not_null<const RimPerforationInterval*> perforationInterval,
    int                                          timeStep,
    gsl::not_null<RimEclipseCase*>               eclipseCase )
{
    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo*       activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );
    bool hasDate  = (size_t)timeStep < eclipseCase->timeStepDates().size();
    bool isActive = !hasDate || perforationInterval->isActiveOnDate( eclipseCase->timeStepDates()[timeStep] );

    if ( wellPath->perforationIntervalCollection()->isChecked() && perforationInterval->isChecked() && isActive )
    {
        std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
            wellPathGeometry->clippedPointSubset( perforationInterval->startMD(), perforationInterval->endMD() );

        std::vector<WellPathCellIntersectionInfo> intersectedCells =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                              wellPath->name(),
                                                                              perforationPointsAndMD.first,
                                                                              perforationPointsAndMD.second );

        for ( auto& cell : intersectedCells )
        {
            bool cellIsActive = activeCellInfo->isActive( cell.globCellIndex );
            if ( !cellIsActive ) continue;

            RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                          RigCompletionDataGridCell( cell.globCellIndex, eclipseCase->mainGrid() ),
                                          cell.startMD );

            completion.setSourcePdmObject( perforationInterval );
            completionData.push_back( completion );
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignPerforationIntersections(
    const std::vector<RigCompletionData>& completionData,
    std::shared_ptr<RicMswCompletion>     perforationCompletion,
    const WellPathCellIntersectionInfo&   cellIntInfo,
    double                                overlapStart,
    double                                overlapEnd,
    bool*                                 foundSubGridIntersections )
{
    size_t currCellId = cellIntInfo.globCellIndex;

    std::shared_ptr<RicMswSubSegment> subSegment(
        new RicMswSubSegment( overlapStart, overlapEnd, cellIntInfo.startTVD(), cellIntInfo.endTVD() ) );
    for ( const RigCompletionData& compIntersection : completionData )
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        if ( !cell.isMainGridCell() )
        {
            *foundSubGridIntersections = true;
        }

        if ( cell.globalCellIndex() != currCellId ) continue;

        cvf::Vec3st localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

        std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(
            new RicMswSubSegmentCellIntersection( cell.lgrName(),
                                                  cell.globalCellIndex(),
                                                  localIJK,
                                                  cellIntInfo.intersectionLengthsInCellCS ) );
        subSegment->addIntersection( intersection );
    }
    perforationCompletion->addSubSegment( subSegment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignBranchNumbers( const RimEclipseCase*          caseToApply,
                                                               std::shared_ptr<RicMswSegment> segment,
                                                               int*                           branchNum )
{
    for ( std::shared_ptr<RicMswCompletion> completion : segment->completions() )
    {
        if ( completion->completionType() == RigCompletionData::PERFORATION )
        {
            completion->setBranchNumber( 1 );
        }
        else if ( completion->completionType() != RigCompletionData::FISHBONES_ICD )
        {
            ++( *branchNum );
            completion->setBranchNumber( *branchNum );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignBranchNumbers( const RimEclipseCase* caseToApply,
                                                               RicMswExportInfo*     exportInfo )
{
    int branchNumber = 1;

    // First loop over the segments so that each segment on the main stem is an incremental number
    for ( auto segment : exportInfo->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::FISHBONES_ICD )
            {
                completion->setBranchNumber( ++branchNumber );
            }
        }
    }

    // Then assign branch numbers to each completion sub segment
    for ( auto segment : exportInfo->segments() )
    {
        assignBranchNumbers( caseToApply, segment, &branchNumber );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportMswCompletionsImpl::tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath,
                                                                  double                            measuredDepth )
{
    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    double tvdValue = -wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth ).z();

    return tvdValue;
}
