/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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
#include "cvfVector2.h"

using namespace cvf;

//==================================================================================================
///
///
//==================================================================================================
class RigContourMapGrid
{
public:
    RigContourMapGrid( const cvf::BoundingBox& originalBoundingBox, double sampleSpacing );
    RigContourMapGrid( const cvf::BoundingBox& originalBoundingBox, const cvf::BoundingBox& expandedBoundingBox, double sampleSpacing );

    // Copy constructor
    RigContourMapGrid( const RigContourMapGrid& other )
        : m_sampleSpacing( other.m_sampleSpacing )
        , m_mapSize( other.m_mapSize )
        , m_expandedBoundingBox( other.m_expandedBoundingBox )
        , m_originalBoundingBox( other.m_originalBoundingBox )
    {
    }

    double sampleSpacing() const;

    cvf::Vec2ui numberOfElementsIJ() const;
    cvf::Vec2ui numberOfVerticesIJ() const;

    cvf::uint numberOfCells() const;
    size_t    numberOfVertices() const;

    cvf::Vec3d origin3d() const;

    std::vector<double> xVertexPositions() const;
    std::vector<double> yVertexPositions() const;

    std::vector<cvf::Vec3d> generateVertices() const;

    // Cell index and position conversion
    size_t      cellIndexFromIJ( cvf::uint i, cvf::uint j ) const;
    size_t      vertexIndexFromIJ( cvf::uint i, cvf::uint j ) const;
    cvf::Vec2ui ijFromVertexIndex( size_t gridIndex ) const;
    cvf::Vec2ui ijFromCellIndex( size_t mapIndex ) const;
    cvf::Vec2ui ijFromLocalPos( const cvf::Vec2d& localPos2d ) const;
    cvf::Vec2d  cellCenterPosition( cvf::uint i, cvf::uint j ) const;
    cvf::Vec2d  origin2d() const;

    double gridEdgeOffset() const;

    const cvf::BoundingBox& expandedBoundingBox() const;
    const cvf::BoundingBox& originalBoundingBox() const;
    const cvf::Vec2ui&      mapSize() const;

private:
    static cvf::Vec2ui calculateMapSize( const cvf::Vec3d& extent, double sampleSpacing );

    static cvf::BoundingBox
        makeMaxPointMultipleOfCellSize( const cvf::BoundingBox& boundingBox, const cvf::Vec2ui& mapSize, double sampleSpacing );

    double           m_sampleSpacing;
    cvf::Vec2ui      m_mapSize;
    cvf::BoundingBox m_expandedBoundingBox;
    cvf::BoundingBox m_originalBoundingBox;
};
