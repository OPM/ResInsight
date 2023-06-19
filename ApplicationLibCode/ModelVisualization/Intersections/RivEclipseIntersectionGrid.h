/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 - Equinor ASA
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

#include "RivIntersectionHexGridInterface.h"

#include "RimCellFilterIntervalTool.h"

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cvfStructGrid.h"

#include <array>
#include <vector>

class RigActiveCellInfo;
class RigMainGrid;
class RigFault;

class RivEclipseIntersectionGrid : public RivIntersectionHexGridInterface
{
public:
    RivEclipseIntersectionGrid( const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool showInactiveCells );

    cvf::Vec3d       displayOffset() const override;
    cvf::BoundingBox boundingBox() const override;
    void             findIntersectingCells( const cvf::BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells ) const override;
    bool             useCell( size_t cellIndex ) const override;
    void             cellCornerVertices( size_t cellIndex, cvf::Vec3d cellCorners[8] ) const override;
    void             cellCornerIndices( size_t cellIndex, size_t cornerIndices[8] ) const override;
    const RigFault*  findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face ) const override;
    void             setKIntervalFilter( bool enabled, std::string kIntervalStr ) override;

private:
    cvf::cref<RigMainGrid>       m_mainGrid;
    cvf::cref<RigActiveCellInfo> m_activeCellInfo;
    bool                         m_showInactiveCells;
    RimCellFilterIntervalTool    m_intervalTool;
};
