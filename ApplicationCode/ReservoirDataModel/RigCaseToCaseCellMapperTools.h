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

#include "cvfMath.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigFemPart;

//==================================================================================================
///
//==================================================================================================

class RigCaseToCaseCellMapperTools
{
public:
    static void       estimatedFemCellFromEclCell( const RigMainGrid* eclGrid,
                                                   size_t             reservoirCellIndex,
                                                   cvf::Vec3d         estimatedElmCorners[8] );
    static void       rotateCellTopologicallyToMatchBaseCell( const cvf::Vec3d* baseCell,
                                                              bool              baseCellFaceNormalsIsOutwards,
                                                              cvf::Vec3d*       cell );
    static cvf::Vec3d calculateCellCenter( cvf::Vec3d elmCorners[8] );
    static bool       elementCorners( const RigFemPart* femPart, int elmIdx, cvf::Vec3d elmCorners[8] );
    static bool
        isEclFemCellsMatching( const cvf::Vec3d baseCell[8], cvf::Vec3d cell[8], double xyTolerance, double zTolerance );

private:
    static void rotateQuad( cvf::Vec3d quad[4], int idxToNewStart );
    static void flipQuadWinding( cvf::Vec3d quad[4] );
    static int  quadVxClosestToXYOfPoint( const cvf::Vec3d point, const cvf::Vec3d quad[4] );
    static int
        findMatchingPOSKFaceIdx( const cvf::Vec3d baseCell[8], bool isBaseCellNormalsOutwards, const cvf::Vec3d c2[8] );
};
