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



class RigActiveCellInfo
{
public:
    RigActiveCellInfo();

    void                setGlobalCellCount(size_t globalCellCount);

    bool                isActiveInMatrixModel(size_t globalCellIndex) const;
    size_t              activeIndexInMatrixModel(size_t globalCellIndex) const;
    void                setActiveIndexInMatrixModel(size_t globalCellIndex, size_t globalActiveCellIndex);

    bool                isActiveInFractureModel(size_t globalCellIndex) const;
    size_t              activeIndexInFractureModel(size_t globalCellIndex) const;
    void                setActiveIndexInFractureModel(size_t globalCellIndex, size_t globalActiveCellIndex);

    // From RigBase
    void                setGridCount(size_t gridCount);
    void                setGridActiveCellCounts(size_t gridIndex, size_t matrixActiveCellCount, size_t fractureActiveCellCount);
    void                gridActiveCellCounts(size_t gridIndex, size_t& matrixActiveCellCount, size_t& fractureActiveCellCount);
    void                computeDerivedData();


    // From RigMainGrid
    size_t              globalMatrixModelActiveCellCount() const;
    size_t              globalFractureModelActiveCellCount() const;

    void                setMatrixModelActiveCellsBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max);
    void                matrixModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const;
    void                setFractureModelActiveCellsBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max);
    void                fractureModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const;

    cvf::BoundingBox    matrixActiveCellsGeometryBoundingBox() const;
    void                setMatrixActiveCellsGeometryBoundingBox(cvf::BoundingBox bb);


private:
    class GridActiveCellCounts
    {
    public:
        size_t          matrixModelActiveCellCount() const;
        void            setMatrixModelActiveCellCount(size_t activeMatrixModelCellCount);
        size_t          fractureModelActiveCellCount() const;
        void            setFractureModelActiveCellCount(size_t activeFractureModelCellCount);

    private:
        size_t          m_matrixModelActiveCellCount;
        size_t          m_fractureModelActiveCellCount;
    };


private:
    std::vector<size_t>     m_activeInMatrixModel;
    std::vector<size_t>     m_activeInFractureModel;

    std::vector<GridActiveCellCounts>     m_perGridActiveCellInfo;

    // From RigMainGrid
    size_t                                  m_globalMatrixModelActiveCellCount;
    size_t                                  m_globalFractureModelActiveCellCount;

    cvf::Vec3st                             m_activeCellPositionMin;
    cvf::Vec3st                             m_activeCellPositionMax;
    cvf::Vec3st                             m_fractureModelActiveCellPositionMin;
    cvf::Vec3st                             m_fractureModelActiveCellPositionMax;

    // NOT USED
//    cvf::Vec3st                             m_validCellPositionMin;
//    cvf::Vec3st                             m_validCellPositionMax;
    cvf::BoundingBox                        m_matrixActiveCellsBoundingBox;
};
