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

#include "RigFemPartGrid.h"

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
void RigFemPart::assertNodeToElmIndicesIsCalculated()
{
    if (m_nodeToElmRefs.size() != nodes().nodeIds.size())
    {
        this->calculateNodeToElmRefs();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPart::calculateNodeToElmRefs()
{
    m_nodeToElmRefs.resize(nodes().nodeIds.size());

    for (size_t eIdx = 0; eIdx < m_elementId.size(); ++eIdx)
    {
        int elmNodeCount = RigFemTypes::elmentNodeCount(elementType(eIdx));
        const int* elmNodes = connectivities(eIdx);
        for (int enIdx = 0; enIdx < elmNodeCount; enIdx)
        {
            m_nodeToElmRefs[elmNodes[enIdx]].push_back(eIdx);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const size_t* RigFemPart::elementsUsingNode(int nodeIndex)
{
   return &(m_nodeToElmRefs[nodeIndex][0]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigFemPart::numElementsUsingNode(int nodeIndex)
{
    return static_cast<int>(m_nodeToElmRefs[nodeIndex].size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPart::assertElmNeighborsIsCalculated()
{
    if (m_elmNeighbors.size() != m_elementId.size())
    {
        this->calculateElmNeighbors();
    }
}

#include "RigFemFaceComparator.h"
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPart::calculateElmNeighbors()
{
    // Calculate elm neighbors: elmIdxs matching each face of the element

    RigFemFaceComparator fComp; // Outside loop to avoid memory alloc/dealloc. Rember to set as private in opm parallelization
    
    m_elmNeighbors.resize(this->elementCount());

    for (cvf::uint eIdx = 0; eIdx < this->elementCount(); ++eIdx)
    {
        RigElementType elmType = this->elementType(eIdx);
        const int* elmNodes = this->connectivities(eIdx);

        int faceCount = RigFemTypes::elmentFaceCount(elmType);

        for (int faceIdx = 0; faceIdx < faceCount; ++faceIdx)
        {
            m_elmNeighbors[eIdx].idxToNeighborElmPrFace[faceIdx] = -1;

            int faceNodeCount = 0;
            const int* localFaceIndices = RigFemTypes::localElmNodeIndicesForFace(elmType, faceIdx, &faceNodeCount);

            // Get neighbor candidates
            int firstNodeIdxOfFace = elmNodes[localFaceIndices[0]];
            int neigborCandidateCount = this->numElementsUsingNode(firstNodeIdxOfFace);
            const size_t* candidates = this->elementsUsingNode(firstNodeIdxOfFace);
            if (neigborCandidateCount)
            {
                fComp.setMainFace(elmNodes, localFaceIndices, faceNodeCount);
            }

            // Check if any of the neighbor candidates faces matches 
            for (int nbcIdx = 0; nbcIdx < neigborCandidateCount; ++nbcIdx)
            {
                int nbcElmIdx = candidates[neigborCandidateCount];

                RigElementType nbcElmType = this->elementType(nbcElmIdx);
                const int* nbcElmNodes = this->connectivities(nbcElmIdx);

                int nbcFaceCount = RigFemTypes::elmentFaceCount(nbcElmType);
                bool isNeighborFound = false;
                for (int nbcFaceIdx = 0; nbcFaceIdx < nbcFaceCount; ++nbcFaceIdx)
                {
                    int nbcFaceNodeCount = 0;
                    const int* nbcLocalFaceIndices = RigFemTypes::localElmNodeIndicesForFace(nbcElmType, nbcFaceIdx, &nbcFaceNodeCount);

                    // Compare faces
                    if (fComp.isSameButOposite(nbcElmNodes, nbcLocalFaceIndices, nbcFaceNodeCount))
                    {
                        m_elmNeighbors[eIdx].idxToNeighborElmPrFace[faceIdx] = nbcElmIdx;
                        isNeighborFound = true;
                        break;
                    }
                }

                if (isNeighborFound) break;
            }
        }
    }
}

