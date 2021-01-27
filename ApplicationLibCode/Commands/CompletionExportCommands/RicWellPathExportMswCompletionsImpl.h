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
#pragma once

#include "RicMswExportInfo.h"
#include "RigCompletionData.h"

#include <gsl/gsl>

class RicExportCompletionDataSettingsUi;
class RifTextDataTableFormatter;
class RigActiveCellInfo;
class RimEclipseCase;
class RimFishbones;
class RimPerforationInterval;
class RimWellPath;
class RimWellPathValve;
class RimWellPathFracture;
class SubSegmentIntersectionInfo;
class RigWellPath;

struct WellPathCellIntersectionInfo;

class QFile;

class RicWellPathExportMswCompletionsImpl
{
public:
    static void exportWellSegmentsForAllCompletions( const RicExportCompletionDataSettingsUi& exportSettings,
                                                     const std::vector<RimWellPath*>&         wellPaths );

    static void exportWellSegmentsForFractures( RimEclipseCase*        eclipseCase,
                                                std::shared_ptr<QFile> exportFile,
                                                const RimWellPath*     wellPath );

    static void exportWellSegmentsForFishbones( RimEclipseCase*        eclipseCase,
                                                std::shared_ptr<QFile> exportFile,
                                                const RimWellPath*     wellPath );

    static void exportWellSegmentsForPerforations( RimEclipseCase*        eclipseCase,
                                                   std::shared_ptr<QFile> exportFile,
                                                   const RimWellPath*     wellPath,
                                                   int                    timeStep );

    static RicMswExportInfo generateFishbonesMswExportInfo( const RimEclipseCase* caseToApply,
                                                            const RimWellPath*    wellPath,
                                                            bool                  enableSegmentSplitting );

private:
    static RicMswExportInfo generateFishbonesMswExportInfo( const RimEclipseCase*             caseToApply,
                                                            const RimWellPath*                wellPath,
                                                            const std::vector<RimFishbones*>& fishbonesSubs,
                                                            bool                              enableSegmentSplitting );

    static RicMswExportInfo generateFracturesMswExportInfo( RimEclipseCase* caseToApply, const RimWellPath* wellPath );

    static RicMswExportInfo generateFracturesMswExportInfo( RimEclipseCase*                          caseToApply,
                                                            const RimWellPath*                       wellPath,
                                                            const std::vector<RimWellPathFracture*>& fractures );

    static RicMswExportInfo
        generatePerforationsMswExportInfo( RimEclipseCase*                                   eclipseCase,
                                           const RimWellPath*                                wellPath,
                                           int                                               timeStep,
                                           const std::vector<const RimPerforationInterval*>& perforationIntervals );

    static std::vector<WellPathCellIntersectionInfo>
        generateCellSegments( const RimEclipseCase* eclipseCase, const RimWellPath* wellPath, double& initialMD );

    static std::vector<WellPathCellIntersectionInfo>
        filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                             double                                           initialMD,
                             gsl::not_null<const RigWellPath*>                wellPathGeometry,
                             gsl::not_null<const RimEclipseCase*>             eclipseCase );

    static void generateWelsegsTable( RifTextDataTableFormatter& formatter,
                                      const RicMswExportInfo&    exportInfo,
                                      double                     maxSegmentLength );

    static void writeMainBoreWelsegsSegment( std::shared_ptr<RicMswSegment> segment,
                                             std::shared_ptr<RicMswSegment> previousSegment,
                                             RifTextDataTableFormatter&     formatter,
                                             const RicMswExportInfo&        exportInfo,
                                             double                         maxSegmentLength,
                                             int*                           segmentNumber );
    static void writeValveWelsegsSegment( std::shared_ptr<RicMswSegment> segment,
                                          std::shared_ptr<RicMswValve>   valve,
                                          RifTextDataTableFormatter&     formatter,
                                          const RicMswExportInfo&        exportInfo,
                                          double                         maxSegmentLength,
                                          int*                           segmentNumber );
    static void writeCompletionWelsegsSegment( std::shared_ptr<RicMswSegment>    segment,
                                               std::shared_ptr<RicMswCompletion> completion,
                                               RifTextDataTableFormatter&        formatter,
                                               const RicMswExportInfo&           exportInfo,
                                               double                            maxSegmentLength,
                                               int*                              segmentNumber );

    static void generateWelsegsSegments( RifTextDataTableFormatter&                         formatter,
                                         const RicMswExportInfo&                            exportInfo,
                                         const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                                         double                                             maxSegmentLength,
                                         int*                                               segmentNumber );
    static void generateWelsegsCompletionCommentHeader( RifTextDataTableFormatter&        formatter,
                                                        RigCompletionData::CompletionType completionType );
    static void generateCompsegTables( RifTextDataTableFormatter& formatter, const RicMswExportInfo& exportInfo );
    static void generateCompsegTable( RifTextDataTableFormatter&                         formatter,
                                      const RicMswExportInfo&                            exportInfo,
                                      bool                                               exportSubGridIntersections,
                                      const std::set<RigCompletionData::CompletionType>& exportCompletionTypes );
    static void generateCompsegHeader( RifTextDataTableFormatter&        formatter,
                                       const RicMswExportInfo&           exportInfo,
                                       RigCompletionData::CompletionType completionType,
                                       bool                              exportSubGridIntersections );
    static void generateWsegvalvTable( RifTextDataTableFormatter& formatter, const RicMswExportInfo& exportInfo );
    static void generateWsegAicdTable( RifTextDataTableFormatter& formatter, const RicMswExportInfo& exportInfo );

    static std::pair<double, double>
        calculateOverlapWithActiveCells( double                                           startMD,
                                         double                                           endMD,
                                         const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                         const RigActiveCellInfo*                         activeCellInfo );

private:
    typedef std::vector<std::shared_ptr<RicMswSegment>>                                       MainBoreSegments;
    typedef std::map<std::shared_ptr<RicMswCompletion>, std::vector<const RimWellPathValve*>> ValveContributionMap;

    static std::vector<std::pair<double, double>>
        createSubSegmentMDPairs( double startMD, double endMD, double maxSegmentLength );

    static MainBoreSegments
        createMainBoreSegmentsForPerforations( const std::vector<WellPathCellIntersectionInfo>& cellSegmentIntersections,
                                               const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                               const RimWellPath*                                wellPath,
                                               int                                               timeStep,
                                               RimEclipseCase*                                   eclipseCase,
                                               bool* foundSubGridIntersections );

    static void createValveCompletions( std::vector<std::shared_ptr<RicMswSegment>>&      mainBoreSegments,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        const RimWellPath*                                wellPath,
                                        RiaDefines::EclipseUnitSystem                     unitSystem );

    static void
        assignValveContributionsToSuperICDsOrAICDs( const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
                                                    const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                    const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                                    const RigActiveCellInfo*                         activeCellInfo,
                                                    RiaDefines::EclipseUnitSystem                    unitSystem );

    static void moveIntersectionsToICVs( const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
                                         const std::vector<const RimPerforationInterval*>&  perforationIntervals,
                                         RiaDefines::EclipseUnitSystem                      unitSystem );

    static void moveIntersectionsToSuperICDsOrAICDs( MainBoreSegments mainBoreSegments );

    static void assignFishbonesLateralIntersections( const RimEclipseCase*          caseToApply,
                                                     const RimWellPath*             wellPath,
                                                     const RimFishbones*            fishbonesSubs,
                                                     std::shared_ptr<RicMswSegment> segment,
                                                     bool*                          foundSubGridIntersections,
                                                     double                         maxSegmentLength );

    static void assignFractureCompletionsToCellSegment( const RimEclipseCase*                 caseToApply,
                                                        const RimWellPathFracture*            fracture,
                                                        const std::vector<RigCompletionData>& completionData,
                                                        std::shared_ptr<RicMswSegment>        segment,
                                                        bool* foundSubGridIntersections );

    static std::vector<RigCompletionData>
        generatePerforationIntersections( gsl::not_null<const RimWellPath*>            wellPath,
                                          gsl::not_null<const RimPerforationInterval*> perforationInterval,
                                          int                                          timeStep,
                                          gsl::not_null<RimEclipseCase*>               eclipseCase );

    static void assignPerforationIntersections( const std::vector<RigCompletionData>& completionData,
                                                std::shared_ptr<RicMswCompletion>     perforationCompletion,
                                                const WellPathCellIntersectionInfo&   cellIntInfo,
                                                double                                overlapStart,
                                                double                                overlapEnd,
                                                bool*                                 foundSubGridIntersections );

    static void
        assignBranchNumbers( const RimEclipseCase* caseToApply, std::shared_ptr<RicMswSegment> segment, int* branchNum );
    static void assignBranchNumbers( const RimEclipseCase* caseToApply, RicMswExportInfo* exportInfo );

    static double tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath, double measuredDepth );
};
