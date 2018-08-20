/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigCompletionData.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicMultiSegmentWellExportInfo.h"
#include "RicWellPathFractureReportItem.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

class RigCell;
class RigEclipseCaseData;
class RigMainGrid;
class RimEclipseCase;
class RimFishbonesMultipleSubs;
class RimSimWellInView;
class RimWellPath;
class RimWellPathFracture;
class RifEclipseDataTableFormatter;
class RigVirtualPerforationTransmissibilities;

//==================================================================================================
/// 
//==================================================================================================
class RicWellPathExportCompletionDataFeatureImpl
{

public:
    static RicMultiSegmentWellExportInfo  generateFishbonesMswExportInfo(const RimEclipseCase* caseToApply,
                                                                         const RimWellPath*    wellPath);

    static RicMultiSegmentWellExportInfo  generateFishbonesMswExportInfo(const RimEclipseCase*                         caseToApply,
                                                                         const RimWellPath*                            wellPath,
                                                                         const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs);

    static RicMultiSegmentWellExportInfo  generateFracturesMswExportInfo(RimEclipseCase*    caseToApply,
                                                                         const RimWellPath* wellPath);

    static RicMultiSegmentWellExportInfo  generateFracturesMswExportInfo(RimEclipseCase*                          caseToApply,
                                                                         const RimWellPath*                       wellPath,
                                                                         const std::vector<RimWellPathFracture*>& fractures);

    static CellDirection                  calculateDirectionInCell(RimEclipseCase* eclipseCase, 
                                                                   size_t globalCellIndex, 
                                                                   const cvf::Vec3d& lengthsInCell);
    
    static double                         calculateTransmissibility(RimEclipseCase* eclipseCase, 
                                                                    const RimWellPath* wellPath, 
                                                                    const cvf::Vec3d& internalCellLengths, 
                                                                    double skinFactor, 
                                                                    double wellRadius, 
                                                                    size_t globalCellIndex, 
                                                                    bool useLateralNTG, 
                                                                    size_t volumeScaleConstant = 1, 
                                                                    CellDirection directionForVolumeScaling = CellDirection::DIR_I);


    static void                           exportCompletions(const std::vector<RimWellPath*>& wellPaths, 
                                                            const std::vector<RimSimWellInView*>& simWells, 
                                                            const RicExportCompletionDataSettingsUi& exportSettings);

    static std::vector<RigCompletionData> computeStaticCompletionsForWellPath(RimWellPath* wellPath,
                                                                              RimEclipseCase* eclipseCase);

    static std::vector<RigCompletionData> computeDynamicCompletionsForWellPath(RimWellPath* wellPath,
                                                                               RimEclipseCase* eclipseCase,
                                                                               size_t timeStepIndex);

    static void                           generateWelsegsTable(RifEclipseDataTableFormatter& formatter,
                                                               const RicMultiSegmentWellExportInfo& exportInfo);
    static void                           generateCompsegTables(RifEclipseDataTableFormatter& formatter,
                                                                const RicMultiSegmentWellExportInfo& exportInfo);
    static void                           generateCompsegTable(RifEclipseDataTableFormatter& formatter,
                                                               const RicMultiSegmentWellExportInfo& exportInfo,
                                                               bool exportSubGridIntersections);
    static void                           generateWsegvalvTable(RifEclipseDataTableFormatter& formatter,
                                                                const RicMultiSegmentWellExportInfo& exportInfo);

private:
    static double                         calculateTransmissibilityAsEclipseDoes(RimEclipseCase* eclipseCase,
                                                                                 double skinFactor,
                                                                                 double wellRadius,
                                                                                 size_t globalCellIndex,
                                                                                 CellDirection direction);
    
    static RigCompletionData              combineEclipseCellCompletions(const std::vector<RigCompletionData>& completions, 
                                                                        const RicExportCompletionDataSettingsUi& settings);

    static void                           sortAndExportCompletionsToFile(RimEclipseCase* eclipseCase,
                                                                         const QString& exportFolder, 
                                                                         const QString& fileName, 
                                                                         std::vector<RigCompletionData>& completions, 
                                                                         const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems,
                                                                         RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static void                           exportCompdatAndWpimultTables(RimEclipseCase* sourceCase,
                                                                        const QString& folderName, 
                                                                        const QString& fileName, 
                                                                        const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid, 
                                                                        const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems,
                                                                        RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static void                           exportCompdatTableUsingFormatter(RifEclipseDataTableFormatter& formatter, 
                                                                           const QString& gridName, 
                                                                           const std::vector<RigCompletionData>& completionData);

    static void                           exportWpimultTableUsingFormatter(RifEclipseDataTableFormatter& formatter,
                                                                           const QString& gridName,
                                                                           const std::vector<RigCompletionData>& completionData);

    static std::vector<RigCompletionData> generatePerforationsCompdatValues(const RimWellPath* wellPath,
                                                                                   const RicExportCompletionDataSettingsUi& settings);

    static void                           assignFishbonesLateralIntersections(const RimEclipseCase*           caseToApply,
                                                                              const RimFishbonesMultipleSubs* fishbonesSubs,
                                                                              RicMswWellSegment*              location,
                                                                              bool*                           foundSubGridIntersections);

    static void                           assignFractureIntersections(const RimEclipseCase*                 caseToApply,
                                                                      const RimWellPathFracture*            fracture,
                                                                      const std::vector<RigCompletionData>& completionData,
                                                                      RicMswWellSegment*                    location,
                                                                      bool*                                 foundSubGridIntersections);

    static void                           assignBranchAndSegmentNumbers(const RimEclipseCase*   caseToApply,
                                                                        RicMswWellSegment*      location,
                                                                        int*                    branchNum,
                                                                        int*                    segmentNum);
    static void                           assignBranchAndSegmentNumbers(const RimEclipseCase* caseToApply,
                                                                        RicMultiSegmentWellExportInfo* exportInfo);

    static void                           appendCompletionData(std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>* completionData,
                                                               const std::vector<RigCompletionData>& data);
};
