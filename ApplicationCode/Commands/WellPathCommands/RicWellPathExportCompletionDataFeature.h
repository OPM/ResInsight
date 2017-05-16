/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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


#include "cafCmdFeature.h"

#include "cvfBoundingBox.h"

class RimWellPath;
class RimEclipseCase;
class RigEclipseCaseData;
class RigMainGrid;
class RigCell;

namespace caf 
{

struct EclipseCellIndexRange {
    size_t i;
    size_t j;
    size_t k1;
    size_t k2;
};

typedef std::tuple<size_t, size_t, size_t> EclipseCellIndex;

//==================================================================================================
/// 
//==================================================================================================
class RicWellPathExportCompletionDataFeature : public CmdFeature
{
    CAF_CMD_HEADER_INIT;
protected:

    // Overrides
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;

private:
    static void                                  exportToFolder(RimWellPath* wellPath, const QString& fileName, const RimEclipseCase* caseToApply, bool includeWpimult);
    static std::vector<size_t>                   findCloseCells(const RigEclipseCaseData* caseData, const cvf::BoundingBox& bb);
    static std::vector<EclipseCellIndexRange>    getCellIndexRange(const RigMainGrid* grid, const std::vector<size_t>& cellIndices);
    static bool                                  cellOrdering(const EclipseCellIndex& cell1, const EclipseCellIndex& cell2);
    static std::vector<size_t>                   findIntersectingCells(const RigEclipseCaseData* grid, const std::vector<cvf::Vec3d>& coords);
    static void                                  setHexCorners(const RigCell& cell, const std::vector<cvf::Vec3d>& nodeCoords, cvf::Vec3d* hexCorners);
    static std::vector<size_t>                   filterWellPathCells(const std::vector<size_t>& completionCells, const std::vector<size_t>& wellPathCells);
    static void                                  addLateralToCells(std::map<size_t, double>* lateralsPerCell, const std::vector<size_t>& lateralCells);
};



} // end namespace caf
