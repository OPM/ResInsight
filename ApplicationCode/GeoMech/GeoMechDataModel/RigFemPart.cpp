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
#include "cvfBoundingBox.h"
#include "cvfBoundingBoxTree.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPart::RigFemPart()
    : m_elementPartId( -1 )
    , m_characteristicElementSize( std::numeric_limits<float>::infinity() )
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
void RigFemPart::preAllocateElementStorage( int elementCount )
{
    m_elementId.reserve( elementCount );
    m_elementTypes.reserve( elementCount );
    m_elementConnectivityStartIndices.reserve( elementCount );

    m_allElementConnectivities.reserve( elementCount * 8 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::appendElement( RigElementType elmType, int id, const int* connectivities )
{
    m_elementId.push_back( id );
    m_elementTypes.push_back( elmType );
    m_elementConnectivityStartIndices.push_back( m_allElementConnectivities.size() );

    int nodeCount = RigFemTypes::elementNodeCount( elmType );
    for ( int lnIdx = 0; lnIdx < nodeCount; ++lnIdx )
    {
        m_allElementConnectivities.push_back( connectivities[lnIdx] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPartGrid* RigFemPart::getOrCreateStructGrid() const
{
    if ( m_structGrid.isNull() )
    {
        m_structGrid = new RigFemPartGrid();
        m_structGrid->setFemPart( this );
    }

    return m_structGrid.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::assertNodeToElmIndicesIsCalculated()
{
    if ( m_nodeToElmRefs.size() != nodes().nodeIds.size() )
    {
        this->calculateNodeToElmRefs();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::calculateNodeToElmRefs()
{
    m_nodeToElmRefs.resize( nodes().nodeIds.size() );
    m_nodeGlobalToLocalIndices.resize( nodes().nodeIds.size() );

    for ( int eIdx = 0; eIdx < static_cast<int>( m_elementId.size() ); ++eIdx )
    {
        int        elmNodeCount = RigFemTypes::elementNodeCount( elementType( eIdx ) );
        const int* elmNodes     = connectivities( eIdx );
        for ( int localIdx = 0; localIdx < elmNodeCount; ++localIdx )
        {
            m_nodeToElmRefs[elmNodes[localIdx]].push_back( eIdx );
            m_nodeGlobalToLocalIndices[elmNodes[localIdx]].push_back( localIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFemPart::elementsUsingNode( int nodeIndex ) const
{
    return m_nodeToElmRefs[nodeIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned char>& RigFemPart::elementLocalIndicesForNode( int nodeIndex ) const
{
    return m_nodeGlobalToLocalIndices[nodeIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::assertElmNeighborsIsCalculated()
{
    if ( m_elmNeighbors.size() != m_elementId.size() )
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

    RigFemFaceComparator fComp; // Outside loop to avoid memory alloc/dealloc. Rember to set as private in opm
                                // parallelization
    std::vector<int> candidates; //

    m_elmNeighbors.resize( this->elementCount() );

    for ( int eIdx = 0; eIdx < this->elementCount(); ++eIdx )
    {
        RigElementType elmType  = this->elementType( eIdx );
        const int*     elmNodes = this->connectivities( eIdx );

        int faceCount     = RigFemTypes::elementFaceCount( elmType );
        int neighborCount = 0;
        for ( int faceIdx = 0; faceIdx < faceCount; ++faceIdx )
        {
            m_elmNeighbors[eIdx].indicesToNeighborElms[faceIdx] = -1;
            m_elmNeighbors[eIdx].faceInNeighborElm[faceIdx]     = -1;
            int        faceNodeCount                            = 0;
            const int* localFaceIndices = RigFemTypes::localElmNodeIndicesForFace( elmType, faceIdx, &faceNodeCount );

            // Get neighbor candidates
            candidates.clear();
            {
                int                     firstNodeIdxOfFace = elmNodes[localFaceIndices[0]];
                const std::vector<int>& candidates1        = this->elementsUsingNode( firstNodeIdxOfFace );

                if ( !candidates1.empty() )
                {
                    // Get neighbor candidates from the diagonal node

                    int thirdNodeIdxOfFace = elmNodes[localFaceIndices[3]];

                    const std::vector<int>& candidates2 = this->elementsUsingNode( thirdNodeIdxOfFace );

                    // The candidates are sorted from smallest to largest, so we do a linear search to find the
                    // (two) common cells in the two arrays, and leaving this element out, we have one candidate left

                    size_t idx1 = 0;
                    size_t idx2 = 0;

                    while ( idx1 < candidates1.size() && idx2 < candidates2.size() )
                    {
                        if ( candidates1[idx1] < candidates2[idx2] )
                        {
                            ++idx1;
                            continue;
                        }
                        if ( candidates1[idx1] > candidates2[idx2] )
                        {
                            ++idx2;
                            continue;
                        }
                        if ( candidates1[idx1] == candidates2[idx2] )
                        {
                            if ( candidates1[idx1] != eIdx )
                            {
                                candidates.push_back( candidates1[idx1] );
                            }
                            ++idx1;
                            ++idx2;
                        }
                    }
                }
            }

            if ( candidates.size() )
            {
                fComp.setMainFace( elmNodes, localFaceIndices, faceNodeCount );
            }

            // Check if any of the neighbor candidates faces matches
            for ( int nbcIdx = 0; nbcIdx < static_cast<int>( candidates.size() ); ++nbcIdx )
            {
                int nbcElmIdx = candidates[nbcIdx];

                RigElementType nbcElmType  = this->elementType( nbcElmIdx );
                const int*     nbcElmNodes = this->connectivities( nbcElmIdx );

                int  nbcFaceCount    = RigFemTypes::elementFaceCount( nbcElmType );
                bool isNeighborFound = false;
                for ( int nbcFaceIdx = 0; nbcFaceIdx < nbcFaceCount; ++nbcFaceIdx )
                {
                    int        nbcFaceNodeCount = 0;
                    const int* nbcLocalFaceIndices =
                        RigFemTypes::localElmNodeIndicesForFace( nbcElmType, nbcFaceIdx, &nbcFaceNodeCount );

                    // Compare faces
                    if ( fComp.isSameButOposite( nbcElmNodes, nbcLocalFaceIndices, nbcFaceNodeCount ) )
                    {
                        m_elmNeighbors[eIdx].indicesToNeighborElms[faceIdx] = nbcElmIdx;
                        m_elmNeighbors[eIdx].faceInNeighborElm[faceIdx]     = nbcFaceIdx;
                        isNeighborFound                                     = true;

                        break;
                    }
                }

                if ( isNeighborFound )
                {
                    ++neighborCount;
                    break;
                }
            }
        }

        if ( ( faceCount - neighborCount ) >= 3 )
        {
            m_possibleGridCornerElements.push_back( eIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RigFemPart::faceNormal( int elmIdx, int faceIdx ) const
{
    const std::vector<cvf::Vec3f>& nodeCoordinates = this->nodes().coordinates;

    RigElementType eType          = this->elementType( elmIdx );
    const int*     elmNodeIndices = this->connectivities( elmIdx );

    int        faceNodeCount              = 0;
    const int* localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace( eType, faceIdx, &faceNodeCount );

    if ( faceNodeCount == 4 )
    {
        const cvf::Vec3f* quadVxs[4];

        quadVxs[0] = &( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] );
        quadVxs[1] = &( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] );
        quadVxs[2] = &( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] );
        quadVxs[3] = &( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] );

        cvf::Vec3f normal = ( *( quadVxs[2] ) - *( quadVxs[0] ) ) ^ ( *( quadVxs[3] ) - *( quadVxs[1] ) );
        return normal;
    }
    else if ( faceNodeCount != 4 )
    {
        CVF_ASSERT( false );
    }

    return cvf::Vec3f::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigFemPart::characteristicElementSize() const
{
    if ( m_characteristicElementSize != std::numeric_limits<float>::infinity() ) return m_characteristicElementSize;

    int   elmsToAverageCount = 0;
    float sumMaxEdgeLength   = 0;
    for ( int elmIdx = 0; elmIdx < elementCount(); elmIdx++ )
    {
        RigElementType eType = this->elementType( elmIdx );

        if ( eType == HEX8P )
        {
            const int* elementConn = this->connectivities( elmIdx );
            cvf::Vec3f nodePos0   = this->nodes().coordinates[elementConn[0]];
            cvf::Vec3f nodePos1   = this->nodes().coordinates[elementConn[1]];
            cvf::Vec3f nodePos3   = this->nodes().coordinates[elementConn[3]];
            cvf::Vec3f nodePos4   = this->nodes().coordinates[elementConn[4]];

            float l1 = ( nodePos1 - nodePos0 ).length();
            float l3 = ( nodePos3 - nodePos0 ).length();
            float l4 = ( nodePos4 - nodePos0 ).length();

            float maxLength = l1 > l3 ? l1 : l3;
            maxLength       = maxLength > l4 ? maxLength : l4;

            sumMaxEdgeLength += maxLength;
            ++elmsToAverageCount;
        }
    }

    CVF_ASSERT( elmsToAverageCount );

    m_characteristicElementSize = sumMaxEdgeLength / elmsToAverageCount;

    return m_characteristicElementSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigFemPart::boundingBox() const
{
    if ( m_boundingBox.isValid() ) return m_boundingBox;

    size_t nodeCount = nodes().coordinates.size();
    for ( size_t nIdx = 0; nIdx < nodeCount; ++nIdx )
    {
        m_boundingBox.add( nodes().coordinates[nIdx] );
    }

    return m_boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::findIntersectingCells( const cvf::BoundingBox& inputBB, std::vector<size_t>* elementIndices ) const
{
    ensureIntersectionSearchTreeIsBuilt();
    findIntersectingCellsWithExistingSearchTree( inputBB, elementIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::findIntersectingCellsWithExistingSearchTree( const cvf::BoundingBox& inputBB,
                                                              std::vector<size_t>*    elementIndices ) const
{
    CVF_ASSERT( m_elementSearchTree.notNull() );
    m_elementSearchTree->findIntersections( inputBB, elementIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::ensureIntersectionSearchTreeIsBuilt() const
{
    // build tree
    if ( m_elementSearchTree.isNull() )
    {
        size_t elmCount = elementCount();

        std::vector<cvf::BoundingBox> cellBoundingBoxes;
        cellBoundingBoxes.resize( elmCount );

        for ( size_t elmIdx = 0; elmIdx < elmCount; ++elmIdx )
        {
            const int*        cellIndices = connectivities( elmIdx );
            cvf::BoundingBox& cellBB      = cellBoundingBoxes[elmIdx];
            cellBB.add( m_nodes.coordinates[cellIndices[0]] );
            cellBB.add( m_nodes.coordinates[cellIndices[1]] );
            cellBB.add( m_nodes.coordinates[cellIndices[2]] );
            cellBB.add( m_nodes.coordinates[cellIndices[3]] );
            cellBB.add( m_nodes.coordinates[cellIndices[4]] );
            cellBB.add( m_nodes.coordinates[cellIndices[5]] );
            cellBB.add( m_nodes.coordinates[cellIndices[6]] );
            cellBB.add( m_nodes.coordinates[cellIndices[7]] );
        }

        m_elementSearchTree = new cvf::BoundingBoxTree;
        m_elementSearchTree->buildTreeFromBoundingBoxes( cellBoundingBoxes, nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPart::elementNodeResultCount() const
{
    int lastElmIdx = this->elementCount() - 1;
    if ( lastElmIdx < 0 ) return 0;
    RigElementType elmType          = this->elementType( lastElmIdx );
    int            elmNodeCount     = RigFemTypes::elementNodeCount( elmType );
    size_t         lastElmResultIdx = this->elementNodeResultIdx( lastElmIdx, elmNodeCount - 1 );

    return lastElmResultIdx + 1;
}

//--------------------------------------------------------------------------------------------------
/// Generate a sensible index into the result vector based on which result position type is used.
//--------------------------------------------------------------------------------------------------
size_t RigFemPart::resultValueIdxFromResultPosType( RigFemResultPosEnum resultPosType, int elementIdx, int elmLocalNodeIdx ) const
{
    if ( resultPosType == RIG_ELEMENT || resultPosType == RIG_FORMATION_NAMES )
    {
        CVF_ASSERT( elementIdx < static_cast<int>( m_elementId.size() ) );
        return elementIdx;
    }

    size_t elementNodeResultIdx = this->elementNodeResultIdx( static_cast<int>( elementIdx ), elmLocalNodeIdx );
    CVF_ASSERT( elementNodeResultIdx < elementNodeResultCount() );

    if ( resultPosType == RIG_ELEMENT_NODAL || resultPosType == RIG_INTEGRATION_POINT )
    {
        return elementNodeResultIdx;
    }
    else if ( resultPosType == RIG_NODAL )
    {
        size_t nodeIdx = nodeIdxFromElementNodeResultIdx( elementNodeResultIdx );
        CVF_ASSERT( nodeIdx < m_nodes.nodeIds.size() );
        return nodeIdx;
    }

    CVF_ASSERT( false );
    return 0u;
}
