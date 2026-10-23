#include "gtest/gtest.h"

#include "RigFemPart.h"

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
