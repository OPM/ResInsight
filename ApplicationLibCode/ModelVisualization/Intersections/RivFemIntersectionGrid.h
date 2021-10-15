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

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cvfStructGrid.h"

#include <vector>

class RigFemPart;
class RigFemPartCollection;
class RigFault;

class RivFemIntersectionGrid : public RivIntersectionHexGridInterface
{
public:
    explicit RivFemIntersectionGrid( const RigFemPartCollection* femParts );

    cvf::Vec3d       displayOffset() const override;
    cvf::BoundingBox boundingBox() const override;
    void             findIntersectingCells( const cvf::BoundingBox& intersectingBB,
                                            std::vector<size_t>*    intersectedCells ) const override;
    bool             useCell( size_t cellIndex ) const override;
    void             cellCornerVertices( size_t cellIndex, cvf::Vec3d cellCorners[8] ) const override;
    void             cellCornerIndices( size_t cellIndex, size_t cornerIndices[8] ) const override;
    const RigFault*  findFaultFromCellIndexAndCellFace( size_t                             reservoirCellIndex,
                                                        cvf::StructGridInterface::FaceType face ) const override;

private:
    cvf::cref<RigFemPartCollection> m_femParts;
};
