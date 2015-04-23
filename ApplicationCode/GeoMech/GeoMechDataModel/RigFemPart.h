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

#pragma once

#include "RigFemTypes.h"
#include "cvfObject.h"
#include "cvfAssert.h"

#include "cvfVector3.h"
#include <vector>

class RigFemPartNodes
{
public:
     std::vector<int>           nodeIds;
     std::vector<cvf::Vec3f>    coordinates;
};

class RigFemPart : public cvf::Object
{
public:
    RigFemPart();
    virtual ~RigFemPart();
    void                        preAllocateElementStorage(int elementCount);
    void                        appendElement(RigElementType elmType, int id, const int* connectivities);

    size_t                      elementCount() const                { return m_elementId.size(); }
    
    int                         elmId(size_t index) const           { return m_elementId[index]; }
    RigElementType              elementType(size_t index) const      { return m_elementTypes[index]; }
    const int*                  connectivities(size_t index) const  { return &m_allAlementConnectivities[m_elementConnectivityStartIndices[index]];}

    RigFemPartNodes&            nodes() {return m_nodes;}
    
private:
    std::vector<int>            m_elementId;
    std::vector<RigElementType> m_elementTypes;
    std::vector<size_t>         m_elementConnectivityStartIndices;
    std::vector<int>            m_allAlementConnectivities;

    RigFemPartNodes             m_nodes;

};
