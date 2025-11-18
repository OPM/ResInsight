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
    , m_enabled( true )
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
int RigFemPart::elementPartId() const
{
    return m_elementPartId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::setElementPartId( int partId )
{
    m_elementPartId = partId;
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
int RigFemPart::elementCount() const
{
    return static_cast<int>( m_elementId.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::nodeCount() const
{
    return static_cast<int>( m_nodes.nodeIds.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::allConnectivitiesCount() const
{
    return static_cast<int>( m_allElementConnectivities.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::elmId( size_t elementIdx ) const
{
    return m_elementId[elementIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigElementType RigFemPart::elementType( size_t elementIdx ) const
{
    return m_elementTypes[elementIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const int* RigFemPart::connectivities( size_t elementIdx ) const
{
    return &m_allElementConnectivities[m_elementConnectivityStartIndices[elementIdx]];
}

//--------------------------------------------------------------------------------------------------
/// Returns state of success for fill element coordinates
//--------------------------------------------------------------------------------------------------
bool RigFemPart::fillElementCoordinates( size_t elementIdx, std::array<cvf::Vec3d, 8>& coordinates ) const
{
    const auto elemType         = elementType( elementIdx );
    const int* elemConnectivity = connectivities( elementIdx );
    const auto nodeCount        = RigFemTypes::elementNodeCount( elemType );

    // Only handle element of hex for now - false success
    if ( nodeCount != 8 ) return false;

    // Retrieve the node indices
    std::vector<int> nodeIndices;
    for ( int i = 0; i < nodeCount; ++i )
    {
        const int nodeIdx = elemConnectivity[i];
        nodeIndices.push_back( nodeIdx );
    }

    // Fill coordinates for each node
    const auto& partNodes = nodes();
    for ( int i = 0; i < (int)nodeIndices.size(); ++i )
    {
        coordinates[i].set( partNodes.coordinates[nodeIndices[i]] );
    }

    // Return true success
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPart::elementNodeResultIdx( int elementIdx, int elmLocalNodeIdx ) const
{
    return m_elementConnectivityStartIndices[elementIdx] + elmLocalNodeIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::nodeIdxFromElementNodeResultIdx( size_t elmNodeResultIdx ) const
{
    return m_allElementConnectivities[elmNodeResultIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::elementNeighbor( int elementIndex, int faceIndex ) const
{
    return m_elmNeighbors[elementIndex].indicesToNeighborElms[faceIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPart::neighborFace( int elementIndex, int faceIndex ) const
{
    return m_elmNeighbors[elementIndex].faceInNeighborElm[faceIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFemPart::elementIdxToId() const
{
    return m_elementId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartNodes& RigFemPart::nodes()
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPartNodes& RigFemPart::nodes() const
{
    return m_nodes;
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
        calculateNodeToElmRefs();
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
        calculateElmNeighbors();
    }
}

#include "RigFemFaceComparator.h"
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::calculateElmNeighbors()
{
    // Calculate elm neighbors: elmIdxs matching each face of the element

    RigFemFaceComparator fComp; // Outside loop to avoid memory alloc/dealloc. Remember to set as private in opm
                                // parallelization
    std::vector<int> candidates; //

    m_elmNeighbors.resize( elementCount() );

    for ( int eIdx = 0; eIdx < elementCount(); ++eIdx )
    {
        RigElementType elmType  = elementType( eIdx );
        const int*     elmNodes = connectivities( eIdx );

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
                const std::vector<int>& candidates1        = elementsUsingNode( firstNodeIdxOfFace );

                if ( !candidates1.empty() )
                {
                    // Get neighbor candidates from the diagonal node

                    int thirdNodeIdxOfFace = elmNodes[localFaceIndices[3]];

                    const std::vector<int>& candidates2 = elementsUsingNode( thirdNodeIdxOfFace );

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

            if ( !candidates.empty() )
            {
                fComp.setMainFace( elmNodes, localFaceIndices, faceNodeCount );
            }

            // Check if any of the neighbor candidates faces matches
            for ( int nbcIdx = 0; nbcIdx < static_cast<int>( candidates.size() ); ++nbcIdx )
            {
                int nbcElmIdx = candidates[nbcIdx];

                RigElementType nbcElmType  = elementType( nbcElmIdx );
                const int*     nbcElmNodes = connectivities( nbcElmIdx );

                int  nbcFaceCount    = RigFemTypes::elementFaceCount( nbcElmType );
                bool isNeighborFound = false;
                for ( int nbcFaceIdx = 0; nbcFaceIdx < nbcFaceCount; ++nbcFaceIdx )
                {
                    int        nbcFaceNodeCount    = 0;
                    const int* nbcLocalFaceIndices = RigFemTypes::localElmNodeIndicesForFace( nbcElmType, nbcFaceIdx, &nbcFaceNodeCount );

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
    const std::vector<cvf::Vec3f>& nodeCoordinates = nodes().coordinates;

    RigElementType eType          = elementType( elmIdx );
    const int*     elmNodeIndices = connectivities( elmIdx );

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

    std::vector<RigElementType> elementPriority = { RigElementType::HEX8P, RigElementType::HEX8 };

    for ( auto elmType : elementPriority )
    {
        int   elmsToAverageCount = 0;
        float sumMaxEdgeLength   = 0;
        for ( int elmIdx = 0; elmIdx < elementCount(); elmIdx++ )
        {
            RigElementType eType = elementType( elmIdx );

            if ( eType == elmType )
            {
                const int* elementConn = connectivities( elmIdx );
                cvf::Vec3f nodePos0    = nodes().coordinates[elementConn[0]];
                cvf::Vec3f nodePos1    = nodes().coordinates[elementConn[1]];
                cvf::Vec3f nodePos3    = nodes().coordinates[elementConn[3]];
                cvf::Vec3f nodePos4    = nodes().coordinates[elementConn[4]];

                float l1 = ( nodePos1 - nodePos0 ).length();
                float l3 = ( nodePos3 - nodePos0 ).length();
                float l4 = ( nodePos4 - nodePos0 ).length();

                float maxLength = l1 > l3 ? l1 : l3;
                maxLength       = maxLength > l4 ? maxLength : l4;

                sumMaxEdgeLength += maxLength;
                ++elmsToAverageCount;
            }
        }
        if ( elmsToAverageCount > 0 )
        {
            m_characteristicElementSize = sumMaxEdgeLength / elmsToAverageCount;
            break;
        }
    }

    return m_characteristicElementSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFemPart::possibleGridCornerElements() const
{
    return m_possibleGridCornerElements;
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
std::vector<size_t> RigFemPart::findIntersectingElementIndices( const cvf::BoundingBox& inputBB ) const
{
    ensureIntersectionSearchTreeIsBuilt();
    return findIntersectingElementsWithExistingSearchTree( inputBB );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigFemPart::findIntersectingElementsWithExistingSearchTree( const cvf::BoundingBox& inputBB ) const
{
    CVF_ASSERT( m_elementSearchTree.notNull() );
    std::vector<size_t> elementIndices;
    m_elementSearchTree->findIntersections( inputBB, &elementIndices );
    return elementIndices;
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
    int lastElmIdx = elementCount() - 1;
    if ( lastElmIdx < 0 ) return 0;
    RigElementType elmType          = elementType( lastElmIdx );
    int            elmNodeCount     = RigFemTypes::elementNodeCount( elmType );
    size_t         lastElmResultIdx = elementNodeResultIdx( lastElmIdx, elmNodeCount - 1 );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPart::isHexahedron( size_t elementIdx ) const
{
    RigElementType elType = elementType( elementIdx );
    return RigFemTypes::is8NodeElement( elType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::setName( std::string name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RigFemPart::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::setEnabled( bool enable )
{
    m_enabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPart::enabled() const
{
    return m_enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPart::addElementSet( std::string name, const std::vector<size_t>& elementIds )
{
    m_elementSetNames.push_back( name );

    std::map<size_t, size_t> idToIndex;

    for ( size_t i = 0; i < m_elementId.size(); i++ )
    {
        idToIndex[m_elementId[i]] = i;
    }

    std::vector<size_t> elementIndexes;
    elementIndexes.resize( elementIds.size() );

    for ( size_t i = 0; i < elementIds.size(); i++ )
    {
        elementIndexes[i] = idToIndex[elementIds[i] + 1];
    }

    m_elementIndexSets.push_back( elementIndexes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPart::elementSetNames() const
{
    return m_elementSetNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigFemPart::elementSet( int setIndex ) const
{
    if ( ( setIndex < 0 ) || ( setIndex >= (int)m_elementIndexSets.size() ) ) return {};
    return m_elementIndexSets[setIndex];
}
