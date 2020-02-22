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

class RicExportCompletionDataSettingsUi;
class RifTextDataTableFormatter;
class RimEclipseCase;
class RimFishbonesMultipleSubs;
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

    static void exportWellSegmentsForFractures( RimEclipseCase*                          eclipseCase,
                                                std::shared_ptr<QFile>                   exportFile,
                                                const RimWellPath*                       wellPath,
                                                const std::vector<RimWellPathFracture*>& fractures );

    static void exportWellSegmentsForFishbones( RimEclipseCase*                               eclipseCase,
                                                std::shared_ptr<QFile>                        exportFile,
                                                const RimWellPath*                            wellPath,
                                                const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs );

    static void exportWellSegmentsForPerforations( RimEclipseCase*                                   eclipseCase,
                                                   std::shared_ptr<QFile>                            exportFile,
                                                   const RimWellPath*                                wellPath,
                                                   int                                               timeStep,
                                                   const std::vector<const RimPerforationInterval*>& perforationIntervals );

    static RicMswExportInfo generateFishbonesMswExportInfo( const RimEclipseCase* caseToApply,
                                                            const RimWellPath*    wellPath,
                                                            bool                  enableSegmentSplitting );

private:
    static RicMswExportInfo generateFishbonesMswExportInfo( const RimEclipseCase*                         caseToApply,
                                                            const RimWellPath*                            wellPath,
                                                            const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs,
                                                            bool enableSegmentSplitting );

    static RicMswExportInfo generateFracturesMswExportInfo( RimEclipseCase* caseToApply, const RimWellPath* wellPath );

    static RicMswExportInfo generateFracturesMswExportInfo( RimEclipseCase*                          caseToApply,
                                                            const RimWellPath*                       wellPath,
                                                            const std::vector<RimWellPathFracture*>& fractures );

    static RicMswExportInfo
        generatePerforationsMswExportInfo( RimEclipseCase*                                   eclipseCase,
                                           const RimWellPath*                                wellPath,
                                           int                                               timeStep,
                                           const std::vector<const RimPerforationInterval*>& perforationIntervals );

    static std::vector<SubSegmentIntersectionInfo>
        generateSubSegments( const RimEclipseCase* eclipseCase, const RimWellPath* wellPath, double& initialMD );

    static std::vector<WellPathCellIntersectionInfo>
        filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                             double                                           initialMD,
                             const RigWellPath*                               wellPathGeometry,
                             const RimEclipseCase*                            eclipseCase );

    static void generateWelsegsTable( RifTextDataTableFormatter& formatter, const RicMswExportInfo& exportInfo );

    static void generateWelsegsSegments( RifTextDataTableFormatter&                         formatter,
                                         const RicMswExportInfo&                            exportInfo,
                                         const std::set<RigCompletionData::CompletionType>& exportCompletionTypes );
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

private:
    typedef std::vector<std::shared_ptr<RicMswSegment>> MainBoreSegments;
    typedef std::map<std::shared_ptr<RicMswCompletion>, std::set<std::pair<const RimWellPathValve*, size_t>>> ValveContributionMap;

    static MainBoreSegments
        createMainBoreSegmentsForPerforations( const std::vector<SubSegmentIntersectionInfo>&    subSegIntersections,
                                               const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                               const RimWellPath*                                wellPath,
                                               int                                               timeStep,
                                               RimEclipseCase*                                   eclipseCase,
                                               bool* foundSubGridIntersections );

    static void createValveCompletions( std::vector<std::shared_ptr<RicMswSegment>>&      mainBoreSegments,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        RiaEclipseUnitTools::UnitSystem                   unitSystem );

    static void
        assignValveContributionsToSuperICDsOrAICDs( const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
                                                    const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                    RiaEclipseUnitTools::UnitSystem                   unitSystem );

    static void moveIntersectionsToICVs( const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
                                         const std::vector<const RimPerforationInterval*>&  perforationIntervals,
                                         RiaEclipseUnitTools::UnitSystem                    unitSystem );

    static void moveIntersectionsToSuperICDsOrAICDs( MainBoreSegments mainBoreSegments );

    static void assignFishbonesLateralIntersections( const RimEclipseCase*           caseToApply,
                                                     const RimFishbonesMultipleSubs* fishbonesSubs,
                                                     std::shared_ptr<RicMswSegment>  location,
                                                     bool*                           foundSubGridIntersections,
                                                     double                          maxSegmentLength );

    static void assignFractureIntersections( const RimEclipseCase*                 caseToApply,
                                             const RimWellPathFracture*            fracture,
                                             const std::vector<RigCompletionData>& completionData,
                                             std::shared_ptr<RicMswSegment>        location,
                                             bool*                                 foundSubGridIntersections );

    static std::vector<RigCompletionData> generatePerforationIntersections( const RimWellPath*            wellPath,
                                                                            const RimPerforationInterval* perforationInterval,
                                                                            int                           timeStep,
                                                                            RimEclipseCase*               eclipseCase );

    static void assignPerforationIntersections( const std::vector<RigCompletionData>& completionData,
                                                std::shared_ptr<RicMswCompletion>     perforationCompletion,
                                                const SubSegmentIntersectionInfo&     cellIntInfo,
                                                double                                overlapStart,
                                                double                                overlapEnd,
                                                bool*                                 foundSubGridIntersections );

    static void assignBranchAndSegmentNumbers( const RimEclipseCase*          caseToApply,
                                               std::shared_ptr<RicMswSegment> location,
                                               int*                           branchNum,
                                               int*                           segmentNum );
    static void assignBranchAndSegmentNumbers( const RimEclipseCase* caseToApply, RicMswExportInfo* exportInfo );
};
