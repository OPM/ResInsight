//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfStructGrid.h"
#include "cvfCollection.h"

namespace cvf {

class DrawableGeo;
class ScalarMapper;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class CellRangeFilter
{
public:
    CellRangeFilter();

    void addCellIncludeRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);
    void addCellInclude(size_t i, size_t j, size_t k);

    void addCellExcludeRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);

    bool isCellRejected(size_t i, size_t j, size_t k) const;

    enum CellStateType
    {
        INCLUDED, /// Cell is among the included cells
        NOT_INCLUDED, /// Cell is not among the included cells
        EXCLUDED /// Filter actively states that cell is to be hidden
    };

    CellStateType cellState(size_t i, size_t j, size_t k) const;
    static CellStateType combine(CellStateType a, CellStateType b) ;

private:
    class CellRange
    {
    public:
        CellRange()
            : m_min(cvf::Vec3st::ZERO),
            m_max(UNDEFINED_SIZE_T, UNDEFINED_SIZE_T, UNDEFINED_SIZE_T)
        {
        }

        CellRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK)
            : m_min(minI, minJ, minK),
            m_max(maxI, maxJ, maxK)
        {
        }

        bool isInRange(size_t i, size_t j, size_t k) const
        {
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

    void                textureCoordinates(Vec2fArray* textureCoords, size_t timeStepIndex, size_t scalarSetIndex, const ScalarMapper* mapper) const;

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
