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

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cvfStructGrid.h"

#include <string>
#include <vector>

class RigFault;

//--------------------------------------------------------------------------------------------------
/// Interface definition used to compute the geometry for planes intersecting a grid
//--------------------------------------------------------------------------------------------------
class RivIntersectionHexGridInterface : public cvf::Object
{
public:
    virtual cvf::Vec3d          displayOffset() const                                                                = 0;
    virtual cvf::BoundingBox    boundingBox() const                                                                  = 0;
    virtual std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& intersectingBB ) const                = 0;
    virtual bool                useCell( size_t cellIndex ) const                                                    = 0;
    virtual void                cellCornerVertices( size_t cellIndex, std::array<cvf::Vec3d, 8>& cellCorners ) const = 0;
    virtual void                cellCornerIndices( size_t cellIndex, std::array<size_t, 8>& cornerIndices ) const    = 0;
    virtual const RigFault* findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face ) const = 0;
    virtual void setKIntervalFilter( bool enabled, std::string kIntervalStr ) = 0;
};
