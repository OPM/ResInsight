#include "gtest/gtest.h"

#include "RiuAbstractLegendFrame.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuAbstractLegendFrameTest, FirstAndLastLabelsAreAlwaysVisible )
{
    // Labels from bottom to top, the last label overlaps the second to last label
    std::vector<QRect> labelRects = { QRect( 0, 100, 10, 10 ),
                                      QRect( 0, 80, 10, 10 ),
                                      QRect( 0, 60, 10, 10 ),
                                      QRect( 0, 40, 10, 10 ),
                                      QRect( 0, 35, 10, 10 ) };

    EXPECT_EQ( std::vector<int>( { 0, 1, 2, 4 } ), RiuAbstractLegendFrame::visibleLabelIndices( labelRects ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuAbstractLegendFrameTest, OverlappingLabelsAreSkipped )
{
    std::vector<QRect> labelRects = { QRect( 0, 100, 10, 10 ), QRect( 0, 95, 10, 10 ), QRect( 0, 80, 10, 10 ), QRect( 0, 60, 10, 10 ) };

    EXPECT_EQ( std::vector<int>( { 0, 2, 3 } ), RiuAbstractLegendFrame::visibleLabelIndices( labelRects ) );

    EXPECT_TRUE( RiuAbstractLegendFrame::visibleLabelIndices( {} ).empty() );
    EXPECT_EQ( std::vector<int>( { 0 } ), RiuAbstractLegendFrame::visibleLabelIndices( { QRect( 0, 0, 10, 10 ) } ) );
}
