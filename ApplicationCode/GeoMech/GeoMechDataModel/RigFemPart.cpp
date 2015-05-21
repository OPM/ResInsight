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

#include "RigFemPart.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPart::RigFemPart()
    :m_elementPartId(-1)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPart::~RigFemPart()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPart::preAllocateElementStorage(int elementCount)
{
    m_elementId.reserve(elementCount);
    m_elementTypes.reserve(elementCount);
    m_elementConnectivityStartIndices.reserve(elementCount);

    m_allAlementConnectivities.reserve(elementCount*8);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPart::appendElement(RigElementType elmType, int id, const int* connectivities)
{
    m_elementId.push_back(id);
    m_elementTypes.push_back(elmType);
    m_elementConnectivityStartIndices.push_back(m_allAlementConnectivities.size());

    int nodeCount = RigFemTypes::elmentNodeCount(elmType);
    for (int lnIdx = 0; lnIdx < nodeCount; ++lnIdx)
    {
        m_allAlementConnectivities.push_back(connectivities[lnIdx]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFemPartGrid* RigFemPart::structGrid()
{
    if (m_structGrid.isNull())
    {
        m_structGrid = new RigFemPartGrid(this);
    }

    return m_structGrid.p();
}


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
