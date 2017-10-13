/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C)  Statoil ASA
//  Copyright (C)  Ceetron Solutions AS
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

#include "RigSingleWellResultsData.h"
#include "cvfVector3.h"
#include <vector>

class RigEclipseCaseData;
class RimSimWellInView;

class RigSimulationWellCenterLineCalculator
{
public:
    static void calculateWellPipeStaticCenterline(RimSimWellInView* rimWell, 
                                                  std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                  std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds) ;

    static void calculateWellPipeDynamicCenterline(const RimSimWellInView* rimWell, 
                                                   size_t timeStepIndex,
                                                   std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                   std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds) ;


    static void calculateWellPipeCenterlineFromWellFrame(const RigEclipseCaseData* eclipseCaseData, 
                                                         const RigSingleWellResultsData* wellResults,
                                                         int timeStepIndex,
                                                         bool isAutoDetectBranches,
                                                         bool useAllCellCenters,
                                                         std::vector<std::vector<cvf::Vec3d>> &pipeBranchesCLCoords, 
                                                         std::vector<std::vector<RigWellResultPoint>> &pipeBranchesCellIds);
private:

    static bool hasAnyResultCells(const std::vector<RigWellResultBranch> &resBranches);
    static bool hasAnyValidDataCells(const RigWellResultBranch& branch);
    static void finishPipeCenterLine( std::vector< std::vector<cvf::Vec3d> > &pipeBranchesCLCoords, const cvf::Vec3d& lastCellCenter ) ;

    static void addCellCenterPoints(const RigEclipseCaseData* eclipseCaseData, std::vector<std::vector<cvf::Vec3d>> &pipeBranchesCLCoords, std::vector<std::vector<RigWellResultPoint>> &pipeBranchesCellIds);
};

