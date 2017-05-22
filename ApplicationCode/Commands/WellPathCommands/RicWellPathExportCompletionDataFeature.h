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

#include "RifEclipseOutputTableFormatter.h"

#include "RigWellLogExtractionTools.h"

#include "RimExportCompletionDataSettings.h"

#include "cafCmdFeature.h"

#include "cvfBoundingBox.h"


class RimWellPath;
class RimEclipseCase;
class RigEclipseCaseData;
class RigMainGrid;
class RigCell;
class RimFishbonesMultipleSubs;

//==================================================================================================
/// 
//==================================================================================================
enum WellSegmentCellDirection {
    POS_I,
    NEG_I,
    POS_J,
    NEG_J,
    POS_K,
    NEG_K
};

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLateralIntersection {
    WellSegmentLateralIntersection(int segmentNumber, int attachedSegmentNumber, size_t cellIndex, double length, double depth)
        : segmentNumber(segmentNumber),
          attachedSegmentNumber(attachedSegmentNumber),
          cellIndex(cellIndex),
          length(length),
          depth(depth),
          direction(POS_I),
          directionLength(-1.0),
          mainBoreCell(false)
    {}

    int                      segmentNumber;
    int                      attachedSegmentNumber;
    size_t                   cellIndex;
    bool                     mainBoreCell;
    double                   length;
    double                   depth;
    WellSegmentCellDirection direction;
    double                   directionLength;
};

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLateral {
    WellSegmentLateral(size_t lateralIndex) : lateralIndex(lateralIndex) {}

    size_t                                      lateralIndex;
    int                                         branchNumber;
    std::vector<WellSegmentLateralIntersection> intersections;
};

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLocation {
    WellSegmentLocation(const RimFishbonesMultipleSubs* subs, double measuredDepth, double trueVerticalDepth, size_t subIndex, int segmentNumber = -1)
        : fishbonesSubs(subs),
          measuredDepth(measuredDepth),
          trueVerticalDepth(trueVerticalDepth),
          subIndex(subIndex),
          segmentNumber(segmentNumber)
    {
    }

    const RimFishbonesMultipleSubs*       fishbonesSubs;
    double                                measuredDepth;
    double                                trueVerticalDepth;
    size_t                                subIndex;
    int                                   segmentNumber;
    std::vector<WellSegmentLateral>       laterals;
};

//==================================================================================================
/// 
//==================================================================================================
struct EclipseCellIndexRange {
    size_t i;
    size_t j;
    size_t k1;
    size_t k2;
};

//==================================================================================================
/// 
//==================================================================================================
typedef std::tuple<size_t, size_t, size_t> EclipseCellIndex;

//==================================================================================================
/// 
//==================================================================================================
class RicWellPathExportCompletionDataFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;
protected:

    // Overrides
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;

private:
    static void                                  exportToFolder(RimWellPath* wellPath, const RimExportCompletionDataSettings& exportSettings);

    static void                                  generateCompdatTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings, const std::vector<WellSegmentLocation>& locations);
    static void                                  generateWpimultTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings, const std::map<size_t, double>& lateralsPerCell);
    static void                                  generateWelsegsTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings, const std::vector<WellSegmentLocation>& locations);
    static void                                  generateCompsegsTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings, const std::vector<WellSegmentLocation>& locations);

    static std::map<size_t, double>              computeLateralsPerCell(const std::vector<WellSegmentLocation>& segmentLocations, bool removeMainBoreCells);

    static std::vector<size_t>                   findCloseCells(const RigEclipseCaseData* caseData, const cvf::BoundingBox& bb);
    static size_t                                findCellFromCoords(const RigEclipseCaseData* caseData, const cvf::Vec3d& coords);

    static std::vector<EclipseCellIndexRange>    getCellIndexRange(const RigMainGrid* grid, const std::vector<size_t>& cellIndices);
    static bool                                  cellOrdering(const EclipseCellIndex& cell1, const EclipseCellIndex& cell2);
    static std::vector<size_t>                   findIntersectingCells(const RigEclipseCaseData* grid, const std::vector<cvf::Vec3d>& coords);
    static void                                  setHexCorners(const RigCell& cell, const std::vector<cvf::Vec3d>& nodeCoords, cvf::Vec3d* hexCorners);
    static void                                  markWellPathCells(const std::vector<size_t>& wellPathCells, std::vector<WellSegmentLocation>* locations);
    static bool                                  wellSegmentLocationOrdering(const WellSegmentLocation& first, const WellSegmentLocation& second);
    static std::vector<HexIntersectionInfo>      findIntersections(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords);
    static bool                                  isPointBetween(const cvf::Vec3d& pointA, const cvf::Vec3d& pointB, const cvf::Vec3d& needle);
    static void                                  filterIntersections(std::vector<HexIntersectionInfo>* intersections);
    static std::vector<WellSegmentLocation>      findWellSegmentLocations(const RimEclipseCase* caseToApply, RimWellPath* wellPath);
    static void                                  calculateLateralIntersections(const RimEclipseCase* caseToApply, WellSegmentLocation* location, int* branchNum, int* segmentNum);
    static void                                  assignBranchAndSegmentNumbers(const RimEclipseCase* caseToApply, std::vector<WellSegmentLocation>* locations);

    // Calculate direction
    static void                                        calculateCellMainAxisDirections(const RigMainGrid* grid, size_t cellIndex, cvf::Vec3d* iAxisDirection, cvf::Vec3d* jAxisDirection, cvf::Vec3d* kAxisDirection);
    static cvf::Vec3d                                  calculateCellMainAxisDirection(const cvf::Vec3d* hexCorners, cvf::StructGridInterface::FaceType startFace, cvf::StructGridInterface::FaceType endFace);
    static std::pair<WellSegmentCellDirection, double> calculateDirectionAndDistanceInCell(const RigMainGrid* grid, size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint);
};
