//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cvfArray.h"
#include "cvfCollection.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"

namespace cvf
{
class DrawableGeo;
class ScalarMapper;
class StructGridScalarDataAccess;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class CellRangeFilter
{
public:
    CellRangeFilter();

    void addCellIncludeRange( size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas );
    void addCellInclude( size_t i, size_t j, size_t k, bool applyToSubGridAreas );

    void addCellExcludeRange( size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas );
    void addCellExclude( size_t i, size_t j, size_t k, bool applyToSubGridAreas );

    bool isCellVisible( size_t i, size_t j, size_t k, bool isInSubGridArea ) const;
    bool isCellExcluded( size_t i, size_t j, size_t k, bool isInSubGridArea ) const;

    bool hasIncludeRanges() const;

private:
    class CellRange
    {
    public:
        CellRange()
            : m_min( cvf::Vec3st::ZERO )
            , m_max( UNDEFINED_SIZE_T, UNDEFINED_SIZE_T, UNDEFINED_SIZE_T )
            , m_applyToSubGridAreas( true )
        {
        }

        CellRange( size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas )
            : m_min( minI, minJ, minK )
            , m_max( maxI, maxJ, maxK )
            , m_applyToSubGridAreas( applyToSubGridAreas )
        {
        }

        bool isInRange( size_t i, size_t j, size_t k, bool isInSubGridArea ) const
        {
            if ( isInSubGridArea && !m_applyToSubGridAreas ) return false;
            cvf::Vec3st test( i, j, k );

            int idx;
            for ( idx = 0; idx < 3; idx++ )
            {
                if ( test[idx] < m_min[idx] || m_max[idx] <= test[idx] )
                {
                    return false;
                }
            }

            return true;
        }

    public:
        cvf::Vec3st m_min;
        cvf::Vec3st m_max;
        bool        m_applyToSubGridAreas;
    };

private:
    std::vector<CellRange> m_includeRanges;
    std::vector<CellRange> m_excludeRanges;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class CellFaceVisibilityFilter
{
public:
    virtual bool isFaceVisible( size_t                        i,
                                size_t                        j,
                                size_t                        k,
                                StructGridInterface::FaceType face,
                                const UByteArray*             cellVisibility ) const = 0;
};

class StructGridQuadToCellFaceMapper : public Object
{
public:
    size_t quadCount() const { return m_quadsToCells.size(); }

    size_t                        cellIndex( size_t quadIdx ) const { return m_quadsToCells[quadIdx]; }
    StructGridInterface::FaceType cellFace( size_t quadIdx ) const { return m_quadsToFace[quadIdx]; }

    // Interface for building the mappings
    std::vector<size_t>&                        quadToCellIndexMap() { return m_quadsToCells; }
    std::vector<StructGridInterface::FaceType>& quadToCellFaceMap() { return m_quadsToFace; }

private:
    std::vector<size_t>                        m_quadsToCells;
    std::vector<StructGridInterface::FaceType> m_quadsToFace;
};

class StuctGridTriangleToCellFaceMapper : public Object
{
public:
    explicit StuctGridTriangleToCellFaceMapper( const StructGridQuadToCellFaceMapper* quadMapper )
    {
        m_quadMapper = quadMapper;
    }
    size_t triangleCount() const { return 2 * m_quadMapper->quadCount(); }

    size_t cellIndex( size_t triangleIdx ) const { return m_quadMapper->cellIndex( triangleIdx / 2 ); }
    StructGridInterface::FaceType cellFace( size_t triangleIdx ) const
    {
        return m_quadMapper->cellFace( triangleIdx / 2 );
    }

private:
    cref<StructGridQuadToCellFaceMapper> m_quadMapper;
};

//==================================================================================================
//
//
//
//==================================================================================================
class StructGridGeometryGenerator : public Object
{
public:
    explicit StructGridGeometryGenerator( const StructGridInterface* grid, bool useOpenMP );
    ~StructGridGeometryGenerator() override;

    // Setup methods

    void setCellVisibility( const UByteArray* cellVisibility );
    void addFaceVisibilityFilter( const CellFaceVisibilityFilter* cellVisibilityFilter );

    // Access, valid after generation is done

    const StructGridInterface* activeGrid() { return m_grid.p(); }

    void textureCoordinates( Vec2fArray*                       textureCoords,
                             const StructGridScalarDataAccess* resultAccessor,
                             const ScalarMapper*               mapper ) const;

    // Mapping between cells and geometry

    const StructGridQuadToCellFaceMapper*    quadToCellFaceMapper() { return m_quadMapper.p(); }
    const StuctGridTriangleToCellFaceMapper* triangleToCellFaceMapper() { return m_triangleMapper.p(); }

    // Generated geometry
    ref<DrawableGeo> generateSurface();
    ref<DrawableGeo> createMeshDrawable();
    ref<DrawableGeo> createOutlineMeshDrawable( double creaseAngle );

    static ref<DrawableGeo> createMeshDrawableFromSingleCell( const StructGridInterface* grid, size_t cellIndex );

    static ref<DrawableGeo> createMeshDrawableFromSingleCell( const StructGridInterface* grid,
                                                              size_t                     cellIndex,
                                                              const cvf::Vec3d&          displayModelOffset );

private:
    static ref<UIntArray> lineIndicesFromQuadVertexArray( const Vec3fArray* vertexArray );
    bool                  isCellFaceVisible( size_t i, size_t j, size_t k, StructGridInterface::FaceType face ) const;

    void computeArrays();

private:
    // Input
    cref<StructGridInterface>                    m_grid; // The grid being processed
    std::vector<const CellFaceVisibilityFilter*> m_cellVisibilityFilters;
    cref<UByteArray>                             m_cellVisibility;

    // Created arrays
    cvf::ref<cvf::Vec3fArray> m_vertices;

    // Mappings
    ref<StructGridQuadToCellFaceMapper>    m_quadMapper;
    ref<StuctGridTriangleToCellFaceMapper> m_triangleMapper;

    // Multiple treads can be used when building geometry data structures.
    // This causes visual artifacts due to transparency algorithm, and a stable visual image
    // can be produced if OpenMP is disabled. Currently used by regression test comparing images
    bool m_useOpenMP;
};

} // namespace cvf
