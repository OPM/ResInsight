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

#include "RigFemResultPosEnum.h"
#include "RigFemTypes.h"
#include "cvfAssert.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include <vector>

class RigFemPartGrid;

namespace cvf
{
class BoundingBoxTree;
}

class RigFemPartNodes
{
public:
    std::vector<int>        nodeIds;
    std::vector<cvf::Vec3f> coordinates;
};

class RigFemPart : public cvf::Object
{
public:
    RigFemPart();
    ~RigFemPart() override;

    int  elementPartId() const { return m_elementPartId; }
    void setElementPartId( int partId ) { m_elementPartId = partId; }

    void preAllocateElementStorage( int elementCount );
    void appendElement( RigElementType elmType, int elementId, const int* connectivities );

    int elementCount() const { return static_cast<int>( m_elementId.size() ); }

    int            elmId( size_t elementIdx ) const { return m_elementId[elementIdx]; }
    RigElementType elementType( size_t elementIdx ) const { return m_elementTypes[elementIdx]; }
    const int*     connectivities( size_t elementIdx ) const
    {
        return &m_allElementConnectivities[m_elementConnectivityStartIndices[elementIdx]];
    }

    size_t elementNodeResultIdx( int elementIdx, int elmLocalNodeIdx ) const
    {
        return m_elementConnectivityStartIndices[elementIdx] + elmLocalNodeIdx;
    }
    size_t elementNodeResultCount() const;
    int    nodeIdxFromElementNodeResultIdx( size_t elmNodeResultIdx ) const
    {
        return m_allElementConnectivities[elmNodeResultIdx];
    }
    size_t resultValueIdxFromResultPosType( RigFemResultPosEnum resultPosType, int elementIdx, int elmLocalNodeIdx ) const;
    RigFemPartNodes&       nodes() { return m_nodes; }
    const RigFemPartNodes& nodes() const { return m_nodes; }

    void                              assertNodeToElmIndicesIsCalculated();
    const std::vector<int>&           elementsUsingNode( int nodeIndex ) const;
    const std::vector<unsigned char>& elementLocalIndicesForNode( int nodeIndex ) const;

    void assertElmNeighborsIsCalculated();
    int  elementNeighbor( int elementIndex, int faceIndex ) const
    {
        return m_elmNeighbors[elementIndex].indicesToNeighborElms[faceIndex];
    }
    int neighborFace( int elementIndex, int faceIndex ) const
    {
        return m_elmNeighbors[elementIndex].faceInNeighborElm[faceIndex];
    }

    cvf::BoundingBox        boundingBox() const;
    float                   characteristicElementSize() const;
    const std::vector<int>& possibleGridCornerElements() const { return m_possibleGridCornerElements; }
    void findIntersectingCells( const cvf::BoundingBox& inputBB, std::vector<size_t>* elementIndices ) const;
    void findIntersectingCellsWithExistingSearchTree( const cvf::BoundingBox& inputBB,
                                                      std::vector<size_t>*    elementIndices ) const;

    void ensureIntersectionSearchTreeIsBuilt() const;

    cvf::Vec3f faceNormal( int elementIndex, int faceIndex ) const;

    const RigFemPartGrid*   getOrCreateStructGrid() const;
    const std::vector<int>& elementIdxToId() const { return m_elementId; }

private:
    int m_elementPartId;

    std::vector<int>            m_elementId;
    std::vector<RigElementType> m_elementTypes;
    std::vector<size_t>         m_elementConnectivityStartIndices;
    std::vector<int>            m_allElementConnectivities;

    RigFemPartNodes m_nodes;

    mutable cvf::ref<RigFemPartGrid> m_structGrid;

    void                                    calculateNodeToElmRefs();
    std::vector<std::vector<int>>           m_nodeToElmRefs; // Needs a more memory friendly structure
    std::vector<std::vector<unsigned char>> m_nodeGlobalToLocalIndices;

    void calculateElmNeighbors();
    struct Neighbors
    {
        int  indicesToNeighborElms[6];
        char faceInNeighborElm[6];
    };
    std::vector<Neighbors> m_elmNeighbors;
    std::vector<int>       m_possibleGridCornerElements;

    mutable float            m_characteristicElementSize;
    mutable cvf::BoundingBox m_boundingBox;

    mutable cvf::ref<cvf::BoundingBoxTree> m_elementSearchTree;
};
