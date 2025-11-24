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

#pragma once

#include "RiaDefines.h"

#include <gsl/gsl>

#include <expected>

class RicMswCompletion;
class RigCompletionData;
class RicMswSegment;
class RicMswExportInfo;
class RicMswBranch;
class RigActiveCellInfo;
class RimEclipseCase;
class RimFishbones;
class RimPerforationInterval;
class RimWellPath;
class RigMswTableData;
class RimWellPathFracture;
class RimMswCompletionParameters;
class RigWellPath;
class RimModeledWellPath;

struct WellPathCellIntersectionInfo;

//--------------------------------------------------------------------------------------------------
/// This class is responsible for exporting well path MSW table data
/// This class is based on RicWellPathExportMswCompletionsImpl
//--------------------------------------------------------------------------------------------------
class RicWellPathExportMswTableData
{
public:
    // The intention is to extract MSW data from a single well. Any handling of multiple wells is supposed to be managed in a different class
    static std::expected<RigMswTableData, std::string> extractSingleWellMswData( RimEclipseCase* eclipseCase,
                                                                                 RimWellPath*    wellPath,
                                                                                 int             timeStep,
                                                                                 bool exportCompletionsAfterMainBoreSegments = true );

    static void generateFishbonesMswExportInfoForWell( const RimEclipseCase* eclipseCase,
                                                       const RimWellPath*    wellPath,
                                                       RicMswExportInfo*     exportInfo,
                                                       RicMswBranch*         branch );

private:
    static void updateDataForMultipleItemsInSameGridCell( gsl::not_null<RicMswBranch*> branch );

    static bool generatePerforationsMswExportInfo( const RimEclipseCase*                            eclipseCase,
                                                   const RimWellPath*                               wellPath,
                                                   int                                              timeStep,
                                                   double                                           initialMD,
                                                   const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                                   gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                                   gsl::not_null<RicMswBranch*>                     branch );

    static void appendFishbonesMswExportInfo( const RimEclipseCase*                            eclipseCase,
                                              const RimWellPath*                               wellPath,
                                              double                                           initialMD,
                                              const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                              gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                              gsl::not_null<RicMswBranch*>                     branch );

    static bool appendFracturesMswExportInfo( RimEclipseCase*                                  eclipseCase,
                                              const RimWellPath*                               wellPath,
                                              double                                           initialMD,
                                              const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                              gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                              gsl::not_null<RicMswBranch*>                     branch );

    static std::vector<WellPathCellIntersectionInfo> generateCellSegments( const RimEclipseCase* eclipseCase, const RimWellPath* wellPath );

    static double computeIntitialMeasuredDepth( const RimEclipseCase*                            eclipseCase,
                                                const RimWellPath*                               wellPath,
                                                const RimMswCompletionParameters*                mswParameters,
                                                const std::vector<WellPathCellIntersectionInfo>& allIntersections );

    static std::vector<WellPathCellIntersectionInfo> filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                                          double                                           initialMD,
                                                                          gsl::not_null<const RigWellPath*>                wellPathGeometry,
                                                                          gsl::not_null<const RimEclipseCase*>             eclipseCase );

    static std::pair<double, double> calculateOverlapWithActiveCells( double startMD,
                                                                      double endMD,
                                                                      const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                                                      const RigActiveCellInfo* activeCellInfo );

    static void createWellPathSegments( gsl::not_null<RicMswBranch*>                      branch,
                                        const std::vector<WellPathCellIntersectionInfo>&  cellSegmentIntersections,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        const RimWellPath*                                wellPath,
                                        int                                               timeStep,
                                        const RimEclipseCase*                             eclipseCase,
                                        bool*                                             foundSubGridIntersections );

    static void createValveCompletions( gsl::not_null<RicMswBranch*>                      branch,
                                        const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                        const RimWellPath*                                wellPath,
                                        RiaDefines::EclipseUnitSystem                     unitSystem );

    static void assignValveContributionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*>                      branch,
                                                            const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                            const std::vector<WellPathCellIntersectionInfo>&  wellPathIntersections,
                                                            const RigActiveCellInfo*                          activeCellInfo,
                                                            RiaDefines::EclipseUnitSystem                     unitSystem );

    static void moveIntersectionsToICVs( gsl::not_null<RicMswBranch*>                      branch,
                                         const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                         RiaDefines::EclipseUnitSystem                     unitSystem );

    static void moveIntersectionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*> branch );

    static void assignFishbonesLateralIntersections( const RimEclipseCase*         eclipseCase,
                                                     const RimWellPath*            wellPath,
                                                     const RimFishbones*           fishbonesSubs,
                                                     gsl::not_null<RicMswSegment*> segment,
                                                     bool*                         foundSubGridIntersections,
                                                     double                        maxSegmentLength,
                                                     RiaDefines::EclipseUnitSystem unitSystem );

    static void assignFractureCompletionsToCellSegment( const RimEclipseCase*                 eclipseCase,
                                                        const RimWellPath*                    wellPath,
                                                        const RimWellPathFracture*            fracture,
                                                        const std::vector<RigCompletionData>& completionData,
                                                        gsl::not_null<RicMswSegment*>         segment,
                                                        bool*                                 foundSubGridIntersections );

    static std::vector<RigCompletionData> generatePerforationIntersections( gsl::not_null<const RimWellPath*> wellPath,
                                                                            gsl::not_null<const RimPerforationInterval*> perforationInterval,
                                                                            int                                  timeStep,
                                                                            gsl::not_null<const RimEclipseCase*> eclipseCase );

    static void assignPerforationIntersections( const std::vector<RigCompletionData>& completionData,
                                                gsl::not_null<RicMswCompletion*>      perforationCompletion,
                                                const WellPathCellIntersectionInfo&   cellIntInfo,
                                                double                                overlapStart,
                                                double                                overlapEnd,
                                                bool*                                 foundSubGridIntersections );

    static void assignBranchNumbersToPerforations( const RimEclipseCase* eclipseCase, gsl::not_null<RicMswSegment*> segment, int branchNumber );

    static void assignBranchNumbersToOtherCompletions( const RimEclipseCase*         eclipseCase,
                                                       gsl::not_null<RicMswSegment*> segment,
                                                       gsl::not_null<int*>           branchNumber );

    static void assignBranchNumbersToBranch( const RimEclipseCase*        eclipseCase,
                                             RicMswExportInfo*            exportInfo,
                                             gsl::not_null<RicMswBranch*> branch,
                                             gsl::not_null<int*>          branchNumber );

    static std::unique_ptr<RicMswBranch> createChildMswBranch( const RimWellPath* childWellPath );

    static std::vector<RimWellPath*> wellPathsWithTieIn( const RimWellPath* wellPath );
};
