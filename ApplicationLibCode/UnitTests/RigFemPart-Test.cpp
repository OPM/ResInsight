#include "gtest/gtest.h"

#include "RigFemPart.h"
#include "RigFemTypes.h"

#include "cvfStructGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigFemTypes, Hex8FaceNodeIndicesMatchStructGridFaces )
{
    // HEX8 face order and node order are stored in result files and must stay unchanged
    const int expected[6][4] = { { 1, 2, 6, 5 }, { 0, 4, 7, 3 }, { 3, 7, 6, 2 }, { 0, 1, 5, 4 }, { 4, 5, 6, 7 }, { 0, 3, 2, 1 } };

    for ( auto elmType : { RigElementType::HEX8, RigElementType::HEX8P } )
    {
        ASSERT_EQ( 6, RigFemTypes::elementFaceCount( elmType ) );

        for ( int faceIdx = 0; faceIdx < 6; faceIdx++ )
        {
            int        faceNodeCount = 0;
            const int* nodes         = RigFemTypes::localElmNodeIndicesForFace( elmType, faceIdx, &faceNodeCount );
            ASSERT_EQ( 4, faceNodeCount );

            const auto& structGridNodes =
                cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( faceIdx ) );
            for ( int i = 0; i < 4; i++ )
            {
                EXPECT_EQ( expected[faceIdx][i], nodes[i] ) << "face " << faceIdx << " node " << i;
                EXPECT_EQ( structGridNodes[i], nodes[i] ) << "face " << faceIdx << " node " << i;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigFemTypes, Hex8OppositeFaces )
{
    const int expected[6] = { 1, 0, 3, 2, 5, 4 };

    for ( auto elmType : { RigElementType::HEX8, RigElementType::HEX8P } )
    {
        for ( int faceIdx = 0; faceIdx < 6; faceIdx++ )
        {
            EXPECT_EQ( expected[faceIdx], RigFemTypes::oppositeFace( elmType, faceIdx ) ) << "face " << faceIdx;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigFemTypes, Cax4FaceIsUnchanged )
{
    int        faceNodeCount = 0;
    const int* nodes         = RigFemTypes::localElmNodeIndicesForFace( RigElementType::CAX4, 0, &faceNodeCount );
    ASSERT_EQ( 4, faceNodeCount );
    for ( int i = 0; i < 4; i++ )
    {
        EXPECT_EQ( i, nodes[i] );
    }

    EXPECT_EQ( 0, RigFemTypes::oppositeFace( RigElementType::CAX4, 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// A part with elements but no nodes, as read from a malformed model, gives connectivities that are
/// outside the node list. Calculating neighbors must not read outside the node to element references.
//--------------------------------------------------------------------------------------------------
TEST( RigFemPart, ElmNeighbors_ElementsWithoutNodes )
{
    RigFemPart part;

    const int connectivities[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    part.preAllocateElementStorage( 1 );
    part.appendElement( RigElementType::HEX8, 1, connectivities );

    part.assertNodeToElmIndicesIsCalculated();
    part.assertElmNeighborsIsCalculated();

    EXPECT_TRUE( part.elementsUsingNode( 0 ).empty() );
    EXPECT_TRUE( part.elementsUsingNode( -1 ).empty() );
    EXPECT_TRUE( part.elementLocalIndicesForNode( 0 ).empty() );
    EXPECT_TRUE( part.elementLocalIndicesForNode( -1 ).empty() );
}
