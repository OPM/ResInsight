#include "gtest/gtest.h"

#include "RimSummaryTableTools.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, SortVectorDataOnDateIsIndependentOfInputOrder )
{
    using namespace RimSummaryTableTools;

    auto vector = []( const QString& category, double value, time_t first, time_t last )
    { return VectorData{ .category = category, .name = "WOPR", .values = { value }, .firstTimeStep = first, .lastTimeStep = last }; };

    const std::vector<VectorData> input = { vector( "E-2H", 1.0, 20, 30 ),
                                            vector( "Z-1H", 0.0, 0, 0 ),
                                            vector( "B-1H", 1.0, 20, 30 ),
                                            vector( "C-1H", 1.0, 10, 40 ),
                                            vector( "A-1H", 0.0, 0, 0 ),
                                            vector( "D-1H", 1.0, 20, 25 ) };

    const std::vector<QString> expected = { "C-1H", "D-1H", "B-1H", "E-2H", "A-1H", "Z-1H" };

    for ( bool reverse : { false, true } )
    {
        TableData tableData;
        tableData.thresholdValue       = 0.0;
        tableData.vectorDataCollection = input;
        if ( reverse ) std::reverse( tableData.vectorDataCollection.begin(), tableData.vectorDataCollection.end() );

        sortVectorDataOnDate( tableData );

        EXPECT_EQ( expected, categoryNames( tableData.vectorDataCollection ) );
    }
}
