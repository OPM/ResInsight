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
/// Internal definitions
//--------------------------------------------------------------------------------------------------
class SubSegmentIntersectionInfo
{
public:
    SubSegmentIntersectionInfo( size_t     globCellIndex,
                                double     startTVD,
                                double     endTVD,
                                double     startMD,
                                double     endMD,
                                cvf::Vec3d lengthsInCell );
    static std::vector<SubSegmentIntersectionInfo>
               spiltIntersectionSegmentsToMaxLength( const RigWellPath*                               pathGeometry,
                                                     const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                     double                                           maxSegmentLength );
    static int numberOfSplittedSegments( double startMd, double endMd, double maxSegmentLength );

    size_t     globCellIndex;
    double     startTVD;
    double     endTVD;
    double     startMD;
    double     endMD;
    cvf::Vec3d intersectionLengthsInCellCS;
};

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
            exportWellSegmentsForFractures( exportSettings.caseToApply,
                                            fractureExportFile,
                                            wellPath,
                                            wellPath->fractureCollection()->activeFractures() );
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
                                               exportSettings.timeStep,
                                               wellPath->perforationIntervalCollection()->activePerforations() );
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
            exportWellSegmentsForFishbones( exportSettings.caseToApply,
                                            fishbonesExportFile,
                                            wellPath,
                                            wellPath->fishbonesCollection()->activeFishbonesSubs() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFractures( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath,
                                                                          const std::vector<RimWellPathFracture*>& fractures )
{
    if ( eclipseCase == nullptr )
    {
        RiaLogging::error(
            "Export Fracture Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    RicMswExportInfo exportInfo = generateFracturesMswExportInfo( eclipseCase, wellPath, fractures );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );
    generateWelsegsTable( formatter, exportInfo );
    generateCompsegTables( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFishbones( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath,
                                                                          const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs )
{
    if ( eclipseCase == nullptr )
    {
        RiaLogging::error( "Export Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    RicMswExportInfo exportInfo = generateFishbonesMswExportInfo( eclipseCase, wellPath, fishbonesSubs, true );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );

    generateWelsegsTable( formatter, exportInfo );
    generateCompsegTables( formatter, exportInfo );
    generateWsegvalvTable( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForPerforations(
    RimEclipseCase*                                   eclipseCase,
    std::shared_ptr<QFile>                            exportFile,
    const RimWellPath*                                wellPath,
    int                                               timeStep,
    const std::vector<const RimPerforationInterval*>& perforationIntervals )
{
    if ( eclipseCase == nullptr )
    {
        RiaLogging::error( "Export Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    RicMswExportInfo exportInfo =
        generatePerforationsMswExportInfo( eclipseCase, wellPath, timeStep, perforationIntervals );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );

    generateWelsegsTable( formatter, exportInfo );
    generateCompsegTables( formatter, exportInfo );
    generateWsegvalvTable( formatter, exportInfo );
    generateWsegAicdTable( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWelsegsTable( RifTextDataTableFormatter& formatter,
                                                                const RicMswExportInfo&    exportInfo )
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

        formatter.add( exportInfo.wellPath()->completions()->wellNameForExport() );
        formatter.add( startTVD );
        formatter.add( startMD );
        formatter.addValueOrDefaultMarker( exportInfo.topWellBoreVolume(), RicMswExportInfo::defaultDoubleValue() );
        formatter.add( exportInfo.lengthAndDepthText() );
        formatter.add( QString( "'%1'" ).arg( exportInfo.pressureDropText() ) );

        formatter.rowCompleted();
    }

    {
        std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( "First Seg" ),
                                                      RifTextDataTableColumn( "Last Seg" ),
                                                      RifTextDataTableColumn( "Branch Num" ),
                                                      RifTextDataTableColumn( "Outlet Seg" ),
                                                      RifTextDataTableColumn( "Length" ),
                                                      RifTextDataTableColumn( "Depth Change" ),
                                                      RifTextDataTableColumn( "Diam" ),
                                                      RifTextDataTableColumn( "Rough",
                                                                              RifTextDataTableDoubleFormatting( RIF_FLOAT,
                                                                                                                7 ) )};
        formatter.header( header );
    }

    {
        double prevMD  = exportInfo.initialMD();
        double prevTVD = exportInfo.initialTVD();
        formatter.comment( "Main Stem Segments" );
        for ( std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations() )
        {
            double depth  = 0;
            double length = 0;

            double midPointMD  = 0.5 * ( location->startMD() + location->endMD() );
            double midPointTVD = 0.5 * ( location->startTVD() + location->endTVD() );

            if ( exportInfo.lengthAndDepthText() == QString( "INC" ) )
            {
                depth  = midPointTVD - prevTVD;
                length = midPointMD - prevMD;
            }
            else
            {
                depth  = midPointTVD;
                length = midPointMD;
            }

            if ( location->subIndex() != cvf::UNDEFINED_SIZE_T )
            {
                QString comment = location->label() + QString( ", sub %1" ).arg( location->subIndex() );
                formatter.comment( comment );
            }

            formatter.add( location->segmentNumber() ).add( location->segmentNumber() );
            formatter.add( 1 ); // All segments on main stem are branch 1
            formatter.add( location->segmentNumber() - 1 ); // All main stem segments are connected to the segment below
                                                            // them
            formatter.add( length );
            formatter.add( depth );
            formatter.add( exportInfo.linerDiameter() );
            formatter.add( exportInfo.roughnessFactor() );
            formatter.rowCompleted();
            prevMD  = midPointMD;
            prevTVD = midPointTVD;
        }
    }

    {
        generateWelsegsSegments( formatter, exportInfo, {RigCompletionData::FISHBONES_ICD, RigCompletionData::FISHBONES} );
        generateWelsegsSegments( formatter, exportInfo, {RigCompletionData::FRACTURE} );
        generateWelsegsSegments( formatter,
                                 exportInfo,
                                 {RigCompletionData::PERFORATION_ICD,
                                  RigCompletionData::PERFORATION_ICV,
                                  RigCompletionData::PERFORATION_AICD} );
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateWelsegsSegments(
    RifTextDataTableFormatter&                         formatter,
    const RicMswExportInfo&                            exportInfo,
    const std::set<RigCompletionData::CompletionType>& exportCompletionTypes )
{
    bool generatedHeader = false;
    for ( std::shared_ptr<RicMswSegment> segment : exportInfo.wellSegmentLocations() )
    {
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
                    if ( !completion->subSegments().empty() )
                    {
                        formatter.comment( completion->label() );

                        formatter.add( completion->subSegments().front()->segmentNumber() );
                        formatter.add( completion->subSegments().front()->segmentNumber() );
                        formatter.add( completion->branchNumber() );
                        formatter.add( segment->segmentNumber() );
                        formatter.add( 0.1 ); // ICDs have 0.1 length
                        formatter.add( 0 ); // Depth change
                        formatter.add( exportInfo.linerDiameter() );
                        formatter.add( exportInfo.roughnessFactor() );
                        formatter.rowCompleted();
                    }
                }
                else
                {
                    if ( completion->completionType() == RigCompletionData::FISHBONES )
                    {
                        formatter.comment( QString( "%1 : Sub index %2 - %3" )
                                               .arg( segment->label() )
                                               .arg( segment->subIndex() )
                                               .arg( completion->label() ) );
                    }
                    else if ( completion->completionType() == RigCompletionData::FRACTURE )
                    {
                        formatter.comment(
                            QString( "%1 connected to %2" ).arg( completion->label() ).arg( segment->label() ) );
                    }

                    for ( std::shared_ptr<RicMswSubSegment> subSegment : completion->subSegments() )
                    {
                        double depth  = 0;
                        double length = 0;

                        if ( exportInfo.lengthAndDepthText() == QString( "INC" ) )
                        {
                            depth  = subSegment->deltaTVD();
                            length = subSegment->deltaMD();
                        }
                        else
                        {
                            depth  = subSegment->endTVD();
                            length = subSegment->endMD();
                        }
                        double diameter = segment->effectiveDiameter();
                        formatter.add( subSegment->segmentNumber() );
                        formatter.add( subSegment->segmentNumber() );
                        formatter.add( completion->branchNumber() );
                        formatter.add( subSegment->attachedSegmentNumber() );
                        formatter.add( length );
                        formatter.add( depth );
                        formatter.add( diameter );
                        formatter.add( segment->openHoleRoughnessFactor() );
                        formatter.rowCompleted();
                    }
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
        std::set<RigCompletionData::CompletionType> fishbonesTypes = {RigCompletionData::FISHBONES_ICD,
                                                                      RigCompletionData::FISHBONES};
        generateCompsegTable( formatter, exportInfo, false, fishbonesTypes );
        if ( exportInfo.hasSubGridIntersections() )
        {
            generateCompsegTable( formatter, exportInfo, true, fishbonesTypes );
        }
    }

    {
        std::set<RigCompletionData::CompletionType> fractureTypes = {RigCompletionData::FRACTURE};
        generateCompsegTable( formatter, exportInfo, false, fractureTypes );
        if ( exportInfo.hasSubGridIntersections() )
        {
            generateCompsegTable( formatter, exportInfo, true, fractureTypes );
        }
    }

    {
        std::set<RigCompletionData::CompletionType> perforationTypes = {RigCompletionData::PERFORATION,
                                                                        RigCompletionData::PERFORATION_ICD,
                                                                        RigCompletionData::PERFORATION_ICV,
                                                                        RigCompletionData::PERFORATION_AICD};
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

    for ( std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations() )
    {
        double startMD = location->startMD();

        for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
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
                    }

                    for ( const std::shared_ptr<RicMswSubSegmentCellIntersection>& intersection :
                          subSegment->intersections() )
                    {
                        bool isSubGridIntersection = !intersection->gridName().isEmpty();
                        if ( isSubGridIntersection == exportSubGridIntersections )
                        {
                            if ( exportSubGridIntersections )
                            {
                                formatter.add( intersection->gridName() );
                            }
                            cvf::Vec3st ijk = intersection->gridLocalCellIJK();
                            formatter.addOneBasedCellIndex( ijk.x() ).addOneBasedCellIndex( ijk.y() ).addOneBasedCellIndex(
                                ijk.z() );
                            formatter.add( completion->branchNumber() );

                            double startLength = subSegment->startMD();
                            if ( exportInfo.lengthAndDepthText() == QString( "INC" ) && completion->branchNumber() != 1 )
                            {
                                startLength -= startMD;
                            }
                            formatter.add( startLength );
                            formatter.add( startLength + subSegment->deltaMD() );

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
        std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( "Name" )};
        formatter.header( header );
        formatter.add( exportInfo.wellPath()->completions()->wellNameForExport() );
        formatter.rowCompleted();
    }

    {
        std::vector<RifTextDataTableColumn> allHeaders;
        if ( exportSubGridIntersections )
        {
            allHeaders.push_back( RifTextDataTableColumn( "Grid" ) );
        }

        std::vector<RifTextDataTableColumn> commonHeaders = {RifTextDataTableColumn( "I" ),
                                                             RifTextDataTableColumn( "J" ),
                                                             RifTextDataTableColumn( "K" ),
                                                             RifTextDataTableColumn( "Branch no" ),
                                                             RifTextDataTableColumn( "Start Length" ),
                                                             RifTextDataTableColumn( "End Length" ),
                                                             RifTextDataTableColumn( "Dir Pen" ),
                                                             RifTextDataTableColumn( "End Range" ),
                                                             RifTextDataTableColumn( "Connection Depth" )};
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

    for ( std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations() )
    {
        for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
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
                    if ( icd->completionType() == RigCompletionData::PERFORATION_ICD ||
                         icd->completionType() == RigCompletionData::PERFORATION_ICV )
                    {
                        formatter.comment( icd->label() );
                    }
                    formatter.add( exportInfo.wellPath()->completions()->wellNameForExport() );
                    formatter.add( icd->subSegments().front()->segmentNumber() );
                    formatter.add( icd->flowCoefficient() );
                    formatter.add( QString( "%1" ).arg( icd->area(), 8, 'g', 4 ) );
                    formatter.rowCompleted();
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

    for ( std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations() )
    {
        for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
        {
            if ( completion->completionType() == RigCompletionData::PERFORATION_AICD )
            {
                std::shared_ptr<RicMswPerforationAICD> aicd = std::static_pointer_cast<RicMswPerforationAICD>( completion );
                if ( !aicd->isValid() )
                {
                    RiaLogging::error( QString( "Export AICD Valve (%1): Valve is invalid. At least one required "
                                                "template parameter is not set." )
                                           .arg( aicd->label() ) );
                }

                if ( !foundValve )
                {
                    std::vector<QString> columnDescriptions =
                        {"Well Name",
                         "Segment Number",
                         "Segment Number",
                         "Strength of AICD",
                         "Length of AICD",
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
                         "Exponent of the gas flowing fraction in the density viscosity calculation"};

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
                    tighterFormatter.add( exportInfo.wellPath()->completions()->wellNameForExport() ); // 1
                    tighterFormatter.add( aicd->subSegments().front()->segmentNumber() );
                    tighterFormatter.add( aicd->subSegments().front()->segmentNumber() );

                    std::array<double, AICD_NUM_PARAMS> values = aicd->values();
                    tighterFormatter.add( values[AICD_STRENGTH] );
                    tighterFormatter.add( aicd->length() ); // 5
                    tighterFormatter.add( values[AICD_DENSITY_CALIB_FLUID] );
                    tighterFormatter.add( values[AICD_VISCOSITY_CALIB_FLUID] );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_CRITICAL_WATER_IN_LIQUID_FRAC],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EMULSION_VISC_TRANS_REGION],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_MAX_RATIO_EMULSION_VISC],
                                                              RicMswExportInfo::defaultDoubleValue() ); // 10
                    tighterFormatter.add( 1 );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_MAX_FLOW_RATE],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.add( values[AICD_VOL_FLOW_EXP] );
                    tighterFormatter.add( values[AICD_VISOSITY_FUNC_EXP] );
                    tighterFormatter.add( aicd->isOpen() ? "OPEN" : "SHUT" ); // 15
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_OIL_FRAC_DENSITY],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_WATER_FRAC_DENSITY],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_GAS_FRAC_DENSITY],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_OIL_FRAC_VISCOSITY],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_WATER_FRAC_VISCOSITY],
                                                              RicMswExportInfo::defaultDoubleValue() ); // 20
                    tighterFormatter.addValueOrDefaultMarker( values[AICD_EXP_GAS_FRAC_VISCOSITY],
                                                              RicMswExportInfo::defaultDoubleValue() );
                    tighterFormatter.rowCompleted();
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
    RiaEclipseUnitTools::UnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 wellPath->fishbonesCollection()->startMD(),
                                 wellPath->fishbonesCollection()->mswParameters()->lengthAndDepth().text(),
                                 wellPath->fishbonesCollection()->mswParameters()->pressureDrop().text() );
    exportInfo.setLinerDiameter( wellPath->fishbonesCollection()->mswParameters()->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor( wellPath->fishbonesCollection()->mswParameters()->roughnessFactor( unitSystem ) );

    double maxSegmentLength = enableSegmentSplitting
                                  ? wellPath->fishbonesCollection()->mswParameters()->maxSegmentLength()
                                  : std::numeric_limits<double>::infinity();
    bool   foundSubGridIntersections = false;
    double subStartMD                = wellPath->fishbonesCollection()->startMD();
    for ( RimFishbonesMultipleSubs* subs : fishbonesSubs )
    {
        for ( auto& sub : subs->installedLateralIndices() )
        {
            double subEndMD  = subs->measuredDepth( sub.subIndex );
            double subEndTVD = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( subEndMD ).z();
            int    subSegCount =
                SubSegmentIntersectionInfo::numberOfSplittedSegments( subStartMD, subEndMD, maxSegmentLength );
            double subSegLen = ( subEndMD - subStartMD ) / subSegCount;

            double startMd  = subStartMD;
            double startTvd = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( startMd ).z();
            for ( int ssi = 0; ssi < subSegCount; ssi++ )
            {
                double endMd  = startMd + subSegLen;
                double endTvd = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMd ).z();

                std::shared_ptr<RicMswSegment> location(
                    new RicMswSegment( subs->generatedName(), startMd, endMd, startTvd, endTvd, sub.subIndex ) );
                location->setEffectiveDiameter( subs->effectiveDiameter( unitSystem ) );
                location->setHoleDiameter( subs->holeDiameter( unitSystem ) );
                location->setOpenHoleRoughnessFactor( subs->openHoleRoughnessFactor( unitSystem ) );
                location->setSkinFactor( subs->skinFactor() );
                location->setSourcePdmObject( subs );

                if ( ssi == 0 )
                {
                    // Add completion for ICD
                    std::shared_ptr<RicMswFishbonesICD> icdCompletion( new RicMswFishbonesICD( QString( "ICD" ), nullptr ) );
                    std::shared_ptr<RicMswSubSegment> icdSegment(
                        new RicMswSubSegment( subEndMD, subEndMD + 0.1, subEndTVD, subEndTVD ) );
                    icdCompletion->setFlowCoefficient( subs->icdFlowCoefficient() );
                    double icdOrificeRadius = subs->icdOrificeDiameter( unitSystem ) / 2;
                    icdCompletion->setArea( icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount() );

                    icdCompletion->addSubSegment( icdSegment );
                    location->addCompletion( icdCompletion );

                    for ( size_t lateralIndex : sub.lateralIndices )
                    {
                        QString label = QString( "Lateral %1" ).arg( lateralIndex );
                        location->addCompletion( std::make_shared<RicMswFishbones>( label, lateralIndex ) );
                    }
                    assignFishbonesLateralIntersections( caseToApply, subs, location, &foundSubGridIntersections, maxSegmentLength );
                }

                exportInfo.addWellSegment( location );

                startMd  = endMd;
                startTvd = endTvd;
            }

            subStartMD = subEndMD;
        }
    }
    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortLocations();

    assignBranchAndSegmentNumbers( caseToApply, &exportInfo );

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
    const RigMainGrid*       grid           = caseToApply->eclipseCaseData()->mainGrid();
    const RigActiveCellInfo* activeCellInfo = caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL );
    RiaEclipseUnitTools::UnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    const RigWellPath*             wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords           = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds              = wellPathGeometry->measureDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseToApply->eclipseCaseData(), coords, mds );

    double maxSegmentLength = wellPath->fractureCollection()->mswParameters()->maxSegmentLength();
    std::vector<SubSegmentIntersectionInfo> subSegIntersections =
        SubSegmentIntersectionInfo::spiltIntersectionSegmentsToMaxLength( wellPathGeometry, intersections, maxSegmentLength );

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
    for ( const auto& cellIntInfo : subSegIntersections )
    {
        double startTVD = cellIntInfo.startTVD;
        double endTVD   = cellIntInfo.endTVD;

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
        std::shared_ptr<RicMswSegment> location(
            new RicMswSegment( label, cellIntInfo.startMD, cellIntInfo.endMD, startTVD, endTVD ) );

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
                                                                             wellPath->completions()->wellNameForExport(),
                                                                             wellPath->wellPathGeometry(),
                                                                             {fracture},
                                                                             nullptr,
                                                                             nullptr );

                assignFractureIntersections( caseToApply, fracture, completionData, location, &foundSubGridIntersections );
            }
        }

        exportInfo.addWellSegment( location );
    }
    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortLocations();
    assignBranchAndSegmentNumbers( caseToApply, &exportInfo );

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
    RiaEclipseUnitTools::UnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    std::vector<SubSegmentIntersectionInfo> subSegIntersections = generateSubSegments( eclipseCase, wellPath, initialMD );

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 wellPath->perforationIntervalCollection()->mswParameters()->lengthAndDepth().text(),
                                 wellPath->perforationIntervalCollection()->mswParameters()->pressureDrop().text() );

    exportInfo.setLinerDiameter( wellPath->perforationIntervalCollection()->mswParameters()->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor(
        wellPath->perforationIntervalCollection()->mswParameters()->roughnessFactor( unitSystem ) );

    bool foundSubGridIntersections = false;

    MainBoreSegments mainBoreSegments = createMainBoreSegmentsForPerforations( subSegIntersections,
                                                                               perforationIntervals,
                                                                               wellPath,
                                                                               timeStep,
                                                                               eclipseCase,
                                                                               &foundSubGridIntersections );

    createValveCompletions( mainBoreSegments, perforationIntervals, unitSystem );
    assignValveContributionsToSuperICDsOrAICDs( mainBoreSegments, perforationIntervals, unitSystem );
    moveIntersectionsToICVs( mainBoreSegments, perforationIntervals, unitSystem );
    moveIntersectionsToSuperICDsOrAICDs( mainBoreSegments );

    for ( std::shared_ptr<RicMswSegment> segment : mainBoreSegments )
    {
        exportInfo.addWellSegment( segment );
    }

    exportInfo.setHasSubGridIntersections( foundSubGridIntersections );
    exportInfo.sortLocations();
    assignBranchAndSegmentNumbers( eclipseCase, &exportInfo );

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SubSegmentIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::generateSubSegments( const RimEclipseCase* eclipseCase,
                                                              const RimWellPath*    wellPath,
                                                              double&               initialMD )
{
    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL );
    const RigWellPath*       wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords     = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds        = wellPathGeometry->measureDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(), coords, mds );

    if ( wellPath->perforationIntervalCollection()->mswParameters()->referenceMDType() ==
         RimMswCompletionParameters::MANUAL_REFERENCE_MD )
    {
        initialMD = wellPath->perforationIntervalCollection()->mswParameters()->manualReferenceMD();
    }
    else
    {
        for ( const WellPathCellIntersectionInfo& intersection : intersections )
        {
            if ( activeCellInfo->isActive( intersection.globCellIndex ) )
            {
                initialMD = intersection.startMD;
                break;
            }
        }
    }

    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( intersections, initialMD, wellPathGeometry, eclipseCase );

    const double maxSegmentLength = wellPath->perforationIntervalCollection()->mswParameters()->maxSegmentLength();

    return SubSegmentIntersectionInfo::spiltIntersectionSegmentsToMaxLength( wellPathGeometry,
                                                                             filteredIntersections,
                                                                             maxSegmentLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                              double                initialMD,
                                                              const RigWellPath*    wellPathGeometry,
                                                              const RimEclipseCase* eclipseCase )
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

            extraIntersection.intersectionLengthsInCellCS =
                RigWellPathIntersectionTools::calculateLengthInCell( grid,
                                                                     intersection.globCellIndex,
                                                                     intersectionPoint,
                                                                     intersection.endPoint );
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
        const std::vector<SubSegmentIntersectionInfo>&    subSegIntersections,
        const std::vector<const RimPerforationInterval*>& perforationIntervals,
        const RimWellPath*                                wellPath,
        int                                               timeStep,
        RimEclipseCase*                                   eclipseCase,
        bool*                                             foundSubGridIntersections )
{
    MainBoreSegments mainBoreSegments;

    // Intersections along the well path with grid geometry is handled by well log extraction tools. The threshold in
    // RigWellLogExtractionTools::isEqualDepth is currently set to 0.1m, and this is a pretty large threshold based on
    // the indicated threshold of 0.001m for MSW segments
    const double segmentLengthThreshold = 1.0e-3;

    for ( const auto& cellIntInfo : subSegIntersections )
    {
        const double segmentLength = std::fabs( cellIntInfo.endMD - cellIntInfo.startMD );

        if ( segmentLength > segmentLengthThreshold )
        {
            QString                        label = QString( "Main stem segment %1" ).arg( mainBoreSegments.size() + 2 );
            std::shared_ptr<RicMswSegment> segment(
                new RicMswSegment( label, cellIntInfo.startMD, cellIntInfo.endMD, cellIntInfo.startTVD, cellIntInfo.endTVD ) );

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
    RiaEclipseUnitTools::UnitSystem                   unitSystem )
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

                    if ( segment->startMD() <= valveMD && valveMD < segment->endMD() )
                    {
                        if ( valve->componentType() == RiaDefines::AICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            std::shared_ptr<RicMswSubSegment> subSegment(
                                new RicMswSubSegment( valveMD, valveMD + 0.1, 0.0, 0.0 ) );

                            superAICD = std::make_shared<RicMswPerforationAICD>( valveLabel, valve );
                            superAICD->addSubSegment( subSegment );
                        }
                        else if ( valve->componentType() == RiaDefines::ICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            std::shared_ptr<RicMswSubSegment> subSegment(
                                new RicMswSubSegment( valveMD, valveMD + 0.1, 0.0, 0.0 ) );

                            superICD = std::make_shared<RicMswPerforationICD>( valveLabel, valve );
                            superICD->addSubSegment( subSegment );
                        }
                        else if ( valve->componentType() == RiaDefines::ICV )
                        {
                            QString valveLabel =
                                QString( "ICV %1 at segment #%2" ).arg( valve->name() ).arg( nMainSegment + 2 );
                            std::shared_ptr<RicMswSubSegment> subSegment(
                                new RicMswSubSegment( valveMD, valveMD + 0.1, 0.0, 0.0 ) );

                            ICV = std::make_shared<RicMswPerforationICV>( valveLabel, valve );
                            ICV->addSubSegment( subSegment );
                            ICV->setFlowCoefficient( valve->flowCoefficient() );
                            double orificeRadius = valve->orificeDiameter( unitSystem ) / 2;
                            ICV->setArea( orificeRadius * orificeRadius * cvf::PI_D );
                        }
                    }
                    else if ( overlap > 0.0 && ( valve->componentType() == RiaDefines::ICD && !superICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                        std::shared_ptr<RicMswSubSegment> subSegment(
                            new RicMswSubSegment( overlapStart, overlapStart + 0.1, 0.0, 0.0 ) );
                        superICD = std::make_shared<RicMswPerforationICD>( valveLabel, valve );
                        superICD->addSubSegment( subSegment );
                    }
                    else if ( overlap > 0.0 && ( valve->componentType() == RiaDefines::AICD && !superAICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                        std::shared_ptr<RicMswSubSegment> subSegment(
                            new RicMswSubSegment( overlapStart, overlapStart + 0.1, 0.0, 0.0 ) );
                        superAICD = std::make_shared<RicMswPerforationAICD>( valveLabel, valve );
                        superAICD->addSubSegment( subSegment );
                    }

                    if ( valve->componentType() == RiaDefines::AICD )
                    {
                        totalAICDOverlap += overlap;
                    }
                    else if ( valve->componentType() == RiaDefines::ICD )
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
    RiaEclipseUnitTools::UnitSystem                    unitSystem )
{
    ValveContributionMap assignedRegularValves;
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

        std::shared_ptr<RicMswValveAccumulator> accumulator;
        if ( std::dynamic_pointer_cast<const RicMswPerforationICD>( superValve ) )
        {
            accumulator = std::make_shared<RicMswICDAccumulator>( unitSystem );
        }
        else if ( std::dynamic_pointer_cast<const RicMswPerforationAICD>( superValve ) )
        {
            accumulator = std::make_shared<RicMswAICDAccumulator>( unitSystem );
        }

        if ( !accumulator ) continue;

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType( perforationValves );

            for ( const RimWellPathValve* valve : perforationValves )
            {
                if ( !valve->isChecked() ) continue;

                for ( size_t nSubValve = 0u; nSubValve < valve->valveSegments().size(); ++nSubValve )
                {
                    std::pair<double, double> valveSegment       = valve->valveSegments()[nSubValve];
                    double                    valveSegmentLength = valveSegment.second - valveSegment.first;
                    double                    overlapStart       = std::max( valveSegment.first, segment->startMD() );
                    double                    overlapEnd         = std::min( valveSegment.second, segment->endMD() );
                    double                    overlap            = std::max( 0.0, overlapEnd - overlapStart );

                    if ( overlap > 0.0 && accumulator )
                    {
                        if ( accumulator->accumulateValveParameters( valve, nSubValve, overlap / valveSegmentLength ) )
                        {
                            assignedRegularValves[superValve].insert( std::make_pair( valve, nSubValve ) );
                        }
                    }
                }
            }
        }
        if ( superValve && accumulator )
        {
            accumulator->applyToSuperValve( superValve );
        }
    }

    for ( auto regularValvePair : assignedRegularValves )
    {
        if ( !regularValvePair.second.empty() )
        {
            QStringList valveLabels;
            for ( std::pair<const RimWellPathValve*, size_t> regularValve : regularValvePair.second )
            {
                QString valveLabel = QString( "%1 #%2" ).arg( regularValve.first->name() ).arg( regularValve.second + 1 );
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
    RiaEclipseUnitTools::UnitSystem                    unitSystem )
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
                if ( valve->componentType() != RiaDefines::ICV ) continue;

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
                                                                               const RimFishbonesMultipleSubs* fishbonesSubs,
                                                                               std::shared_ptr<RicMswSegment> location,
                                                                               bool*  foundSubGridIntersections,
                                                                               double maxSegmentLength )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();

    for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
    {
        if ( completion->completionType() != RigCompletionData::FISHBONES )
        {
            continue;
        }

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            fishbonesSubs->coordsAndMDForLateral( location->subIndex(), completion->index() );

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
                                                                              lateralCoords,
                                                                              lateralMDs );

        RigWellPath pathGeometry;
        pathGeometry.m_wellPathPoints = lateralCoords;
        pathGeometry.m_measuredDepths = lateralMDs;
        std::vector<SubSegmentIntersectionInfo> subSegIntersections =
            SubSegmentIntersectionInfo::spiltIntersectionSegmentsToMaxLength( &pathGeometry, intersections, maxSegmentLength );

        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = -lateralCoords.front().z();

        for ( const auto& cellIntInfo : subSegIntersections )
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
                new RicMswSubSegment( previousExitMD, cellIntInfo.endMD, previousExitTVD, cellIntInfo.endTVD ) );

            std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(
                new RicMswSubSegmentCellIntersection( gridName,
                                                      cellIntInfo.globCellIndex,
                                                      cvf::Vec3st( i, j, k ),
                                                      cellIntInfo.intersectionLengthsInCellCS ) );
            subSegment->addIntersection( intersection );
            completion->addSubSegment( subSegment );

            previousExitMD  = cellIntInfo.endMD;
            previousExitTVD = cellIntInfo.endTVD;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignFractureIntersections( const RimEclipseCase*      caseToApply,
                                                                       const RimWellPathFracture* fracture,
                                                                       const std::vector<RigCompletionData>& completionData,
                                                                       std::shared_ptr<RicMswSegment>        location,
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
    location->addCompletion( fractureCompletion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportMswCompletionsImpl::generatePerforationIntersections( const RimWellPath*            wellPath,
                                                                           const RimPerforationInterval* perforationInterval,
                                                                           int                           timeStep,
                                                                           RimEclipseCase*               eclipseCase )
{
    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL );

    if ( wellPath->perforationIntervalCollection()->isChecked() && perforationInterval->isChecked() &&
         perforationInterval->isActiveOnDate( eclipseCase->timeStepDates()[timeStep] ) )
    {
        std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
            wellPath->wellPathGeometry()->clippedPointSubset( perforationInterval->startMD(),
                                                              perforationInterval->endMD() );

        std::vector<WellPathCellIntersectionInfo> intersectedCells =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                              perforationPointsAndMD.first,
                                                                              perforationPointsAndMD.second );

        for ( auto& cell : intersectedCells )
        {
            bool cellIsActive = activeCellInfo->isActive( cell.globCellIndex );
            if ( !cellIsActive ) continue;

            RigCompletionData completion( wellPath->completions()->wellNameForExport(),
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
    const SubSegmentIntersectionInfo&     cellIntInfo,
    double                                overlapStart,
    double                                overlapEnd,
    bool*                                 foundSubGridIntersections )
{
    size_t currCellId = cellIntInfo.globCellIndex;

    std::shared_ptr<RicMswSubSegment> subSegment(
        new RicMswSubSegment( overlapStart, overlapEnd, cellIntInfo.startTVD, cellIntInfo.endTVD ) );
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
void RicWellPathExportMswCompletionsImpl::assignBranchAndSegmentNumbers( const RimEclipseCase*          caseToApply,
                                                                         std::shared_ptr<RicMswSegment> location,
                                                                         int*                           branchNum,
                                                                         int*                           segmentNum )
{
    int icdSegmentNumber = cvf::UNDEFINED_INT;
    for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
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

        int attachedSegmentNumber = location->segmentNumber();
        if ( icdSegmentNumber != cvf::UNDEFINED_INT )
        {
            attachedSegmentNumber = icdSegmentNumber;
        }

        for ( auto subSegment : completion->subSegments() )
        {
            if ( completion->completionType() == RigCompletionData::FISHBONES_ICD )
            {
                subSegment->setSegmentNumber( location->segmentNumber() + 1 );
                icdSegmentNumber = subSegment->segmentNumber();
            }
            else if ( completion->completionType() != RigCompletionData::PERFORATION )
            {
                ++( *segmentNum );
                subSegment->setSegmentNumber( *segmentNum );
            }
            subSegment->setAttachedSegmentNumber( attachedSegmentNumber );
            attachedSegmentNumber = *segmentNum;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignBranchAndSegmentNumbers( const RimEclipseCase* caseToApply,
                                                                         RicMswExportInfo*     exportInfo )
{
    int segmentNumber = 1;
    int branchNumber  = 1;

    // First loop over the locations so that each segment on the main stem is an incremental number
    for ( auto location : exportInfo->wellSegmentLocations() )
    {
        location->setSegmentNumber( ++segmentNumber );
        for ( auto completion : location->completions() )
        {
            if ( completion->completionType() == RigCompletionData::FISHBONES_ICD )
            {
                ++segmentNumber; // Skip a segment number because we need one for the ICD
                completion->setBranchNumber( ++branchNumber );
            }
        }
    }

    // Then assign branch and segment numbers to each completion sub segment
    for ( auto location : exportInfo->wellSegmentLocations() )
    {
        assignBranchAndSegmentNumbers( caseToApply, location, &branchNumber, &segmentNumber );
    }
}

SubSegmentIntersectionInfo::SubSegmentIntersectionInfo( size_t     globCellIndex,
                                                        double     startTVD,
                                                        double     endTVD,
                                                        double     startMD,
                                                        double     endMD,
                                                        cvf::Vec3d lengthsInCell )
    : globCellIndex( globCellIndex )
    , startTVD( startTVD )
    , endTVD( endTVD )
    , startMD( startMD )
    , endMD( endMD )
    , intersectionLengthsInCellCS( lengthsInCell )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SubSegmentIntersectionInfo> SubSegmentIntersectionInfo::spiltIntersectionSegmentsToMaxLength(
    const RigWellPath*                               pathGeometry,
    const std::vector<WellPathCellIntersectionInfo>& intersections,
    double                                           maxSegmentLength )
{
    std::vector<SubSegmentIntersectionInfo> out;

    if ( !pathGeometry ) return out;

    for ( const auto& intersection : intersections )
    {
        double segLen   = intersection.endMD - intersection.startMD;
        int    segCount = (int)std::trunc( segLen / maxSegmentLength ) + 1;

        // Calc effective max length
        double effectiveMaxSegLen = segLen / segCount;

        if ( segCount == 1 )
        {
            out.emplace_back( intersection.globCellIndex,
                              -intersection.startPoint.z(),
                              -intersection.endPoint.z(),
                              intersection.startMD,
                              intersection.endMD,
                              intersection.intersectionLengthsInCellCS );
        }
        else
        {
            double currStartMd = intersection.startMD;
            double currEndMd   = currStartMd;
            double lastTvd     = -intersection.startPoint.z();

            for ( int segIndex = 0; segIndex < segCount; segIndex++ )
            {
                bool lasti = segIndex == ( segCount - 1 );
                currEndMd  = currStartMd + effectiveMaxSegLen;

                cvf::Vec3d segEndPoint = pathGeometry->interpolatedPointAlongWellPath( currEndMd );
                out.emplace_back( intersection.globCellIndex,
                                  lastTvd,
                                  lasti ? -intersection.endPoint.z() : -segEndPoint.z(),
                                  currStartMd,
                                  lasti ? intersection.endMD : currEndMd,
                                  intersection.intersectionLengthsInCellCS / segCount );

                currStartMd = currEndMd;
                lastTvd     = -segEndPoint.z();
            }
        }
    }
    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int SubSegmentIntersectionInfo::numberOfSplittedSegments( double startMd, double endMd, double maxSegmentLength )
{
    return (int)( std::trunc( ( endMd - startMd ) / maxSegmentLength ) + 1 );
}
