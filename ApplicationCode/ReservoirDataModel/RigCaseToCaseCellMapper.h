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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigFemPart;


//==================================================================================================
/// 
//==================================================================================================
class RigCaseToCaseCellMapper : public cvf::Object
{
public:
    RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigFemPart*  dependentFemPart);
    RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigMainGrid* dependentEclGrid);
    RigCaseToCaseCellMapper(RigFemPart*  masterFemPart, RigMainGrid* dependentEclGrid);
    RigCaseToCaseCellMapper(RigFemPart*  masterFemPart, RigFemPart*  dependentFemPart);

    const int * masterCaseCellIndices(int dependentCaseReservoirCellIndex, int* masterCaseCellIndexCount) const;

    const RigMainGrid* masterGrid() const           { return m_masterGrid;}
    const RigMainGrid* dependentGrid() const        { return m_dependentGrid;}
    const RigFemPart*  masterFemPart() const        { return m_masterFemPart;}
    const RigFemPart*  dependentFemPart() const     { return m_dependentFemPart;}

private:

    void storeMapping(int depCaseCellIdx, const std::vector<int>& masterCaseMatchingCells);
    void addMapping(int depCaseCellIdx, int masterCaseMatchingCell);
    void calculateEclToGeomCellMapping(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart, bool eclipseIsMaster);
    std::vector<int>    m_masterCellOrIntervalIndex;
    std::vector<std::vector<int> > m_masterCellIndexSeries;

    RigMainGrid* m_masterGrid;
    RigMainGrid* m_dependentGrid;
    RigFemPart*  m_masterFemPart;
    RigFemPart*  m_dependentFemPart;

};

//==================================================================================================
/// 
//==================================================================================================

class RigCaseToCaseMapperTools
{
public:

    static void estimatedFemCellFromEclCell(const RigMainGrid* eclGrid, size_t reservoirCellIndex, cvf::Vec3d estimatedElmCorners[8]);
    static void rotateQuad(cvf::Vec3d quad[4], int idxToNewStart);
    static void flipQuadWinding(cvf::Vec3d quad[4]);
    static int  quadVxClosestToXYOfPoint(const cvf::Vec3d point, const cvf::Vec3d quad[4]);
    static bool elementCorners(RigFemPart* femPart, int elmIdx, cvf::Vec3d elmCorners[8]);
    static int  findMatchingPOSKFaceIdx(const cvf::Vec3d baseCell[8], bool isBaseCellNormalsOutwards, const cvf::Vec3d c2[8]);
    static bool isEclFemCellsMatching(const cvf::Vec3d baseCell[8], cvf::Vec3d cell[8], double xyTolerance, double zTolerance);
    static void rotateCellTopologicallyToMatchBaseCell(const cvf::Vec3d * baseCell, bool baseCellFaceNormalsIsOutwards, cvf::Vec3d * cell);
};