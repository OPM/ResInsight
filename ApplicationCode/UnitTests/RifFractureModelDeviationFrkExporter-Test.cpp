#include "gtest/gtest.h"

#include "RifFractureModelDeviationFrkExporter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifFractureModelDeviationFrkExporterTest, TvdFixup )
{
    std::vector<double> tvd =
        { 475.722, 492.126, 508.53, 524.934, 541.338, 557.743, 574.147, 590.551, 606.955, 623.359, 639.764, 656.168, 672.572 };
    std::vector<double> md =
        { 475.722, 492.126, 508.53, 524.934, 541.339, 557.743, 574.147, 590.551, 606.955, 623.36, 639.764, 656.168, 672.572 };

    std::vector<double> exportTvd;
    std::vector<double> exportMd;

    RifFractureModelDeviationFrkExporter::fixupDepthValuesForExport( tvd, md, exportTvd, exportMd );

    EXPECT_EQ( tvd.size(), exportTvd.size() );
    EXPECT_EQ( md.size(), exportMd.size() );

    for ( size_t i = 1; i < exportMd.size(); i++ )
    {
        double changeMd  = exportMd[i] - exportMd[i - 1];
        double changeTvd = exportTvd[i] - exportTvd[i - 1];
        ASSERT_GE( changeMd, changeTvd );
    }
}
