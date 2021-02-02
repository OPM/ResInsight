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

#include "RicMswBranch.h"
#include "RicMswCompletions.h"
#include "RicMswExportInfo.h"
#include "RicMswSegment.h"
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

    static bool generatePerforationsMswExportInfo( RimEclipseCase*                                  eclipseCase,
                                                   const RimWellPath*                               wellPath,
                                                   int                                              timeStep,
                                                   double                                           initialMD,
                                                   const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                                   gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                                   gsl::not_null<RicMswBranch*>                     branch );

    static std::vector<WellPathCellIntersectionInfo> generateCellSegments( const RimEclipseCase*  eclipseCase,
                                                                           const RimWellPath*     wellPath,
                                                                           gsl::not_null<double*> initialMD );

    static std::vector<WellPathCellIntersectionInfo>
        filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                             double                                           initialMD,
                             gsl::not_null<const RigWellPath*>                wellPathGeometry,
                             gsl::not_null<const RimEclipseCase*>             eclipseCase );

    static void generateWelsegsTable( RifTextDataTableFormatter& formatter,
                                      RicMswExportInfo&          exportInfo,
                                      double                     maxSegmentLength );

    static void writeWelsegsSegmentsRecursively( RifTextDataTableFormatter&   formatter,
                                                 RicMswExportInfo&            exportInfo,
                                                 gsl::not_null<RicMswBranch*> branch,
                                                 gsl::not_null<int*>          segmentNumber,
                                                 double                       maxSegmentLength,
                                                 RicMswSegment*               connectedToSegment = nullptr );

    static void writeWelsegsSegment( RicMswSegment*             segment,
                                     const RicMswSegment*       previousSegment,
                                     RifTextDataTableFormatter& formatter,
                                     RicMswExportInfo&          exportInfo,
                                     double                     maxSegmentLength,
                                     int                        branchNumber,
                                     int*                       segmentNumber );
    static void writeValveWelsegsSegment( const RicMswSegment*       outletSegment,
                                          RicMswValve*               valve,
                                          RifTextDataTableFormatter& formatter,
                                          RicMswExportInfo&          exportInfo,
                                          double                     maxSegmentLength,
                                          int*                       segmentNumber );
    static void writeCompletionWelsegsSegments( gsl::not_null<const RicMswSegment*>    outletSegment,
                                                gsl::not_null<const RicMswCompletion*> completion,
                                                RifTextDataTableFormatter&             formatter,
                                                RicMswExportInfo&                      exportInfo,
                                                double                                 maxSegmentLength,
                                                int*                                   segmentNumber );

    static void
                writeCompletionWelsegsSegmentsForBranch( RifTextDataTableFormatter&                         formatter,
                                                         RicMswExportInfo&                                  exportInfo,
                                                         gsl::not_null<RicMswBranch*>                       branch,
                                                         const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                                                         double                                             maxSegmentLength,
                                                         int*                                               segmentNumber );
    static void writeWelsegsCompletionCommentHeader( RifTextDataTableFormatter&        formatter,
                                                     RigCompletionData::CompletionType completionType );
    static void generateCompsegTables( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo );
    static void generateCompsegTable( RifTextDataTableFormatter&                         formatter,
                                      RicMswExportInfo&                                  exportInfo,
                                      gsl::not_null<const RicMswBranch*>                 branch,
                                      bool                                               exportSubGridIntersections,
                                      const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                                      gsl::not_null<bool*>                               headerGenerated );
    static void generateCompsegHeader( RifTextDataTableFormatter&        formatter,
                                       RicMswExportInfo&                 exportInfo,
                                       RigCompletionData::CompletionType completionType,
                                       bool                              exportSubGridIntersections );
    static void generateWsegvalvTable( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo );
    static void generateWsegAicdTable( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo );

    static std::pair<double, double>
        calculateOverlapWithActiveCells( double                                           startMD,
                                         double                                           endMD,
                                         const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                         const RigActiveCellInfo*                         activeCellInfo );

private:
    static std::vector<std::pair<double, double>>
        createSubSegmentMDPairs( double startMD, double endMD, double maxSegmentLength );

    static void createWellPathSegments( gsl::not_null<RicMswBranch*>                      branch,
                                        const std::vector<WellPathCellIntersectionInfo>&  cellSegmentIntersections,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        const RimWellPath*                                wellPath,
                                        int                                               timeStep,
                                        RimEclipseCase*                                   eclipseCase,
                                        bool*                                             foundSubGridIntersections );

    static void createValveCompletions( gsl::not_null<RicMswBranch*>                      branch,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        const RimWellPath*                                wellPath,
                                        RiaDefines::EclipseUnitSystem                     unitSystem );

    static void
        assignValveContributionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*>                      branch,
                                                    const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                    const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                                    const RigActiveCellInfo*                         activeCellInfo,
                                                    RiaDefines::EclipseUnitSystem                    unitSystem );

    static void moveIntersectionsToICVs( gsl::not_null<RicMswBranch*>                      branch,
                                         const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                         RiaDefines::EclipseUnitSystem                     unitSystem );

    static void moveIntersectionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*> branch );

    static void assignFishbonesLateralIntersections( const RimEclipseCase*         caseToApply,
                                                     const RimWellPath*            wellPath,
                                                     const RimFishbones*           fishbonesSubs,
                                                     gsl::not_null<RicMswSegment*> segment,
                                                     bool*                         foundSubGridIntersections,
                                                     double                        maxSegmentLength );

    static void assignFractureCompletionsToCellSegment( const RimEclipseCase*                 caseToApply,
                                                        const RimWellPathFracture*            fracture,
                                                        const std::vector<RigCompletionData>& completionData,
                                                        gsl::not_null<RicMswSegment*>         segment,
                                                        bool* foundSubGridIntersections );

    static std::vector<RigCompletionData>
        generatePerforationIntersections( gsl::not_null<const RimWellPath*>            wellPath,
                                          gsl::not_null<const RimPerforationInterval*> perforationInterval,
                                          int                                          timeStep,
                                          gsl::not_null<RimEclipseCase*>               eclipseCase );

    static void assignPerforationIntersections( const std::vector<RigCompletionData>& completionData,
                                                gsl::not_null<RicMswCompletion*>      perforationCompletion,
                                                const WellPathCellIntersectionInfo&   cellIntInfo,
                                                double                                overlapStart,
                                                double                                overlapEnd,
                                                bool*                                 foundSubGridIntersections );

    static void assignBranchNumbersToCompletions( const RimEclipseCase*         caseToApply,
                                                  gsl::not_null<RicMswSegment*> segment,
                                                  gsl::not_null<int*>           branchNumber );
    static void assignBranchNumbersToBranch( const RimEclipseCase*        caseToApply,
                                             RicMswExportInfo*            exportInfo,
                                             gsl::not_null<RicMswBranch*> branch,
                                             gsl::not_null<int*>          branchNumber );

    static double tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath, double measuredDepth );
};
