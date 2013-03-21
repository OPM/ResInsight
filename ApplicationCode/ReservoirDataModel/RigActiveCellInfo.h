/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <vector>



class RigActiveCellInfo : public cvf::Object
{
public:
    RigActiveCellInfo();

    void                setGlobalCellCount(size_t globalCellCount);

    bool                isActiveInMatrixModel(size_t globalCellIndex) const;
    size_t              activeIndexInMatrixModel(size_t globalCellIndex) const;
    void                setActiveIndexInMatrixModel(size_t globalCellIndex, size_t globalActiveCellIndex);

    void                setGridCount(size_t gridCount);
    void                setGridActiveCellCounts(size_t gridIndex, size_t matrixActiveCellCount);
    void                gridActiveCellCounts(size_t gridIndex, size_t& matrixActiveCellCount);
    void                computeDerivedData();

    size_t              globalMatrixModelActiveCellCount() const;

    void                setMatrixModelActiveCellsBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max);
    void                matrixModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const;

    cvf::BoundingBox    matrixActiveCellsGeometryBoundingBox() const;
    void                setMatrixActiveCellsGeometryBoundingBox(cvf::BoundingBox bb);

private:
    class GridActiveCellCounts
    {
    public:
        size_t          matrixModelActiveCellCount() const;
        void            setMatrixModelActiveCellCount(size_t activeMatrixModelCellCount);

    private:
        size_t          m_matrixModelActiveCellCount;
    };


private:
    std::vector<GridActiveCellCounts>   m_perGridActiveCellInfo;

    std::vector<size_t>                 m_activeInMatrixModel;

    size_t                              m_globalMatrixModelActiveCellCount;

    cvf::Vec3st                         m_activeCellPositionMin;
    cvf::Vec3st                         m_activeCellPositionMax;

    cvf::BoundingBox                    m_matrixActiveCellsBoundingBox;
};
