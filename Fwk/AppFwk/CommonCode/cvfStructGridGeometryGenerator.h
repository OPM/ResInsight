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

#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfStructGrid.h"
#include "cvfCollection.h"

namespace cvf {

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

    void addCellIncludeRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas);
    void addCellInclude(size_t i, size_t j, size_t k, bool applyToSubGridAreas);

    void addCellExcludeRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas);

    bool isCellVisible(size_t i, size_t j, size_t k, bool isInSubGridArea) const;
    bool isCellExcluded(size_t i, size_t j, size_t k, bool isInSubGridArea) const;

private:
    class CellRange
    {
    public:
        CellRange()
            : m_min(cvf::Vec3st::ZERO),
            m_max(UNDEFINED_SIZE_T, UNDEFINED_SIZE_T, UNDEFINED_SIZE_T),
            m_applyToSubGridAreas(true)
        {
        }

        CellRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK, bool applyToSubGridAreas)
            : m_min(minI, minJ, minK),
            m_max(maxI, maxJ, maxK),
            m_applyToSubGridAreas(applyToSubGridAreas)
        {
        }

        bool isInRange(size_t i, size_t j, size_t k, bool isInSubGridArea) const
        {
            if (isInSubGridArea && !m_applyToSubGridAreas) return false;
            cvf::Vec3st test(i, j, k);

            int idx;
            for (idx = 0; idx < 3; idx++)
            {
                if (test[idx] < m_min[idx] || m_max[idx] <= test[idx])
                {
                    return false;
                }
            }

            return true;
        }

    public:
        cvf::Vec3st m_min;
        cvf::Vec3st m_max;
        bool m_applyToSubGridAreas;
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
    virtual bool isFaceVisible(size_t i, size_t j, size_t k, StructGridInterface::FaceType face, const UByteArray* cellVisibility) const = 0;
};



//==================================================================================================
//
// 
//
//==================================================================================================
class StructGridGeometryGenerator : public Object
{
public:
    StructGridGeometryGenerator(const StructGridInterface* grid);
    ~StructGridGeometryGenerator();

    // Setup methods

    void                setCellVisibility(const UByteArray* cellVisibility);
    void                addFaceVisibilityFilter(const CellFaceVisibilityFilter* cellVisibilityFilter);

    // Access, valid after generation is done

    const StructGridInterface* activeGrid() { return m_grid.p(); }

    void                textureCoordinates(Vec2fArray* textureCoords, const StructGridScalarDataAccess* dataAccessObject, const ScalarMapper* mapper) const;

    // Mapping between cells and geometry
    ref<cvf::Array<size_t> >    
                        triangleToSourceGridCellMap() const;

    const std::vector<size_t>&                       
                        quadToGridCellIndices() const;
    const std::vector<StructGridInterface::FaceType>&    
                        quadToFace() const;

    // Generated geometry
    ref<DrawableGeo>    generateSurface();
    ref<DrawableGeo>    createMeshDrawable();
    ref<DrawableGeo>    createOutlineMeshDrawable(double creaseAngle);

private:
    static ref<UIntArray> 
                        lineIndicesFromQuadVertexArray(const Vec3fArray* vertexArray);
    bool                isCellFaceVisible(size_t i, size_t j, size_t k, StructGridInterface::FaceType face) const;
    
    void                computeArrays();

private:
    // Input
    cref<StructGridInterface>                    m_grid;                     // The grid being processed
    std::vector<const CellFaceVisibilityFilter*> m_cellVisibilityFilters;
    cref<UByteArray>                             m_cellVisibility;

    // Created arrays
    cvf::ref<cvf::Vec3fArray>                    m_vertices;
    // Mappings
    std::vector<size_t>                          m_triangleIndexToGridCellIndex;
    std::vector<size_t>                          m_quadsToGridCells;
    std::vector<StructGridInterface::FaceType>   m_quadsToFace;
    
};

}
