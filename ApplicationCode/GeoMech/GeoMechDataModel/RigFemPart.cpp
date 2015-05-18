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
