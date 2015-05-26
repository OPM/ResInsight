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

#include <stdlib.h>

#include "RigFemTypes.h"
#include "cvfObject.h"
#include "cvfAssert.h"

#include "cvfVector3.h"
#include <vector>

class RigFemPartGrid;

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

    int                         elementPartId() const                      { return m_elementPartId; }
    void                        setElementPartId(int partId)               { m_elementPartId = partId; }

    void                        preAllocateElementStorage(int elementCount);
    void                        appendElement(RigElementType elmType, int elementId, const int* connectivities);

    size_t                      elementCount() const                       { return m_elementId.size(); }
    
    int                         elmId(size_t elementIdx) const             { return m_elementId[elementIdx]; }
    RigElementType              elementType(size_t elementIdx) const       { return m_elementTypes[elementIdx]; }
    const int*                  connectivities(size_t elementIdx) const    { return &m_allAlementConnectivities[m_elementConnectivityStartIndices[elementIdx]];}
    size_t                      elementNodeResultIdx(int elementIdx, int elmLocalNodeIdx) const { return m_elementConnectivityStartIndices[elementIdx] + elmLocalNodeIdx;}

    RigFemPartNodes&            nodes()                                    {return m_nodes;}
    const RigFemPartNodes&      nodes() const                              {return m_nodes;}

    void                        assertNodeToElmIndicesIsCalculated();
    const size_t*               elementsUsingNode(int nodeIndex);
    int                         numElementsUsingNode(int nodeIndex);
    
    void                        assertElmNeighborsIsCalculated();
    int                         elementNeighbor(int elementIndex, int faceIndex) const
                                { return m_elmNeighbors[elementIndex].idxToNeighborElmPrFace[faceIndex]; }
    const RigFemPartGrid*       structGrid();   

private:
    int                         m_elementPartId;

    std::vector<int>            m_elementId;
    std::vector<RigElementType> m_elementTypes;
    std::vector<size_t>         m_elementConnectivityStartIndices;
    std::vector<int>            m_allAlementConnectivities;

    RigFemPartNodes             m_nodes;

    cvf::ref<RigFemPartGrid>    m_structGrid;

    void calculateNodeToElmRefs();
    std::vector<std::vector<size_t>> m_nodeToElmRefs; // Needs a more memory friendly structure
  
    void calculateElmNeighbors();
    struct Neighbors { int idxToNeighborElmPrFace[6]; };
    std::vector<  Neighbors > m_elmNeighbors;
};
