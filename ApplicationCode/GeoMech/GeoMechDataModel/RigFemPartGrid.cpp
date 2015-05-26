/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemPartGrid.h"


#include "RigFemPart.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartGrid::RigFemPartGrid(RigFemPart* femPart)
{
    m_femPart = femPart;
    generateStructGridData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartGrid::~RigFemPartGrid()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::generateStructGridData()
{
    // 1. Calculate neighbors for each element
    //    record the ones with 3 or fewer neighbors as possible grid corners
    // 2. Loop over the possible corner cells, 
    //    find the one that corresponds to IJK = 000 
    //    by Determining what surfs correspond to NEG IJK surfaces in that element,
    //    and that none of those faces have a neighbor
    // 4. Assign IJK = 000 to that element 
    //    Store IJK in elm idx array
    // 5. Loop along POS I surfaces increment I for each element and assign IJK
    //    when at end, go to POS J neighbor, increment J, repeat above.
    //    etc for POS Z
    //    Find max IJK as you go, 
    //    also assert that there are no NEG I/NEG J/NEG Z neighbors when starting on a new row
    //    (Need to find min, and offset IJK values if there exists such)
    // 6. If IJK to elm idx is needed, allocate "grid" with maxI,maxJ,maxZ values
    //    Loop over elms, assign elmIdx to IJK address in grid
    

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountI() const
{
    CVF_ASSERT(false);
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountJ() const
{
    CVF_ASSERT(false);
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountK() const
{
    CVF_ASSERT(false);
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::isCellValid(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::minCoordinate() const
{
    CVF_ASSERT(false);
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::maxCoordinate() const
{
    CVF_ASSERT(false);
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex) const
{
    CVF_ASSERT(false);
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::cellIndexFromIJK(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::ijkFromCellIndex(size_t cellIndex, size_t* i, size_t* j, size_t* k) const
{
    CVF_ASSERT(false);
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::cellIJKFromCoordinate(const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k) const
{
    CVF_ASSERT(false);
    return false;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::cellCornerVertices(size_t cellIndex, cvf::Vec3d vertices[8]) const
{
    CVF_ASSERT(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::cellCentroid(size_t cellIndex) const
{
    CVF_ASSERT(false);
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::cellMinMaxCordinates(size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate) const
{
    CVF_ASSERT(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointIndexFromIJK(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::gridPointCoordinate(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return cvf::Vec3d::ZERO;
}


