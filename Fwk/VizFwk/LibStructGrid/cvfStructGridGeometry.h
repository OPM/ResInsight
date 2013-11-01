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

namespace cvf {

class RectilinearGrid;
class DrawableGeo;
class ScalarMapper;


//==================================================================================================
//
// 
//
//==================================================================================================
class StructGridGeometry : public Object
{
public:
    StructGridGeometry(const RectilinearGrid* grid);
    ~StructGridGeometry();

    void                setCellRegion(uint minI, uint minJ, uint minK, uint maxI, uint maxJ, uint maxK);
    void                setCellRegionFullGrid();
    void                setCellRegionSlabI(uint i);
    void                setCellRegionSlabJ(uint j);
    void                setCellRegionSlabK(uint k);
    bool                isCellRegionValid() const;
    void                resetCellRegion();

    void                setMapScalar(uint scalarSetIndex, const ScalarMapper* mapper, bool nodeAveragedScalars);

    ref<DrawableGeo>    generateSurface() const;
    ref<DrawableGeo>    generateSimplifiedMeshLines() const;
    ref<DrawableGeo>    generateOutline() const;

private:
    cref<RectilinearGrid>   m_grid;                     // The grid being processed
    uint                    m_cellMinI;                 // Minimum cell in region
    uint                    m_cellMinJ;                 //
    uint                    m_cellMinK;                 //
    uint                    m_cellMaxI;                 // Max cell in region
    uint                    m_cellMaxJ;                 //
    uint                    m_cellMaxK;                 //

    uint                    m_scalarSetIndex;           // Index of scalar set that should be mapped onto the cut plane. UNDEFINED_UINT for no mapping
    cref<ScalarMapper>      m_scalarMapper;             // Scalar mapper to use when mapping. Both scalar set index and mapper must be set in order to get scalar mapping
    bool                    m_mapNodeAveragedScalars;   // If true we'll compute node averaged scalars before mapping them on the surface. If false per cell scalars will be mapped. 
};

}
