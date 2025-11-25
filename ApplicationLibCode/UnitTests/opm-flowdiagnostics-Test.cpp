#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RiaDefines.h"
#include "RifReaderEclipseOutput.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagDefines.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigMainGrid.h"
#include "RimEclipseResultCase.h"

#include <QFileInfo>
#include <QString>

#include <algorithm>
#include <iostream>

//--------------------------------------------------------------------------------------------------
/// Test calculateRelPermCurves for Oil relative permeability in Oil-Gas system
/// Based on Relperm_Oil_in_Oil_Gas test from OPM flowdiagnostics applications by SINTEF
//--------------------------------------------------------------------------------------------------
TEST( FlowDiagnosticsTest, calculateRelPermCurves_Oil_in_Oil_Gas )
{
    // Path to the test data
    QString   egridFilePath = QString( TEST_FLOW_DIAG_MODEL_DIR ) + "/NORNE_ATW2013.EGRID";
    QFileInfo fileInfo( egridFilePath );

    // Check if test file exists
    if ( !fileInfo.exists() )
    {
        GTEST_SKIP() << "Test file not found: " << egridFilePath.toStdString();
        return;
    }

    // Create an Eclipse result case
    auto eclipseCase = std::make_unique<RimEclipseResultCase>();
    eclipseCase->setGridFileName( egridFilePath );

    // Open the case
    bool success = eclipseCase->openEclipseGridFile();
    ASSERT_TRUE( success ) << "Failed to open Eclipse grid file";

    // Create flow diagnostics solver interface
    auto solver = std::make_unique<RigFlowDiagSolverInterface>( eclipseCase.get() );

    // Test with cell 16,31,10 (same as OPM test - Eclipse uses 1-based indexing)
    // Convert to 0-based indexing for ResInsight: (15, 30, 9)
    std::string gridName = ""; // Empty string for main grid

    // Get the active cell index for IJK coordinate (15, 30, 9) in 0-based indexing
    size_t gridLocalActiveCellIndex = 0;
    auto   eclipseCaseData          = eclipseCase->eclipseCaseData();
    if ( eclipseCaseData && eclipseCaseData->mainGrid() )
    {
        size_t i = 15; // 16 - 1 (convert from 1-based to 0-based)
        size_t j = 30; // 31 - 1
        size_t k = 9; // 10 - 1

        size_t gridIndex = eclipseCaseData->mainGrid()->cellIndexFromIJK( i, j, k );

        // Convert grid index to active cell index
        auto activeCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
        if ( activeCellInfo->isActive( gridIndex ) )
        {
            gridLocalActiveCellIndex = activeCellInfo->cellResultIndex( gridIndex );
        }
        else
        {
            GTEST_SKIP() << "Test cell (16,31,10) is not active in the grid";
            return;
        }
    }

    // Calculate relative permeability curves
    auto curves = solver->calculateRelPermCurves( gridName, gridLocalActiveCellIndex );

    // Basic verification - we should get some curves
    EXPECT_GT( curves.size(), 0 ) << "No relative permeability curves calculated";

    // Find KROG curve (Oil relative permeability in Oil-Gas system)
    const RigFlowDiagDefines::RelPermCurve* krogCurve = nullptr;

    for ( const auto& curve : curves )
    {
        if ( curve.ident == RigFlowDiagDefines::RelPermCurve::KROG && curve.curveSet == RigFlowDiagDefines::RelPermCurve::DRAINAGE &&
             curve.epsMode == RigFlowDiagDefines::RelPermCurve::EPS_OFF )
        {
            krogCurve = &curve;
            break;
        }
    }

    // Verify we found the KROG curve
    ASSERT_NE( krogCurve, nullptr ) << "KROG drainage curve without scaling not found";

    // Verify curve has data
    EXPECT_GT( krogCurve->saturationVals.size(), 0 ) << "KROG curve has no saturation values";
    EXPECT_GT( krogCurve->yVals.size(), 0 ) << "KROG curve has no y values";
    EXPECT_EQ( krogCurve->saturationVals.size(), krogCurve->yVals.size() ) << "Saturation and Y values size mismatch";

    // Expected values from OPM test (Gas saturations)
    std::vector<double> expectedSg = { 0.0,  1.0E-4, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4,    0.45, 0.5,
                                       0.55, 0.6,    0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.9999, 1.0 };

    // Expected relative permeability values for KROG
    std::vector<double> expectedKr = { 1.0,      0.999613776, 0.806888, 0.633562, 0.485506, 0.364043, 0.267589, 0.192992,
                                       0.136554, 0.094671,    0.064151, 0.042324, 0.027035, 0.016586, 0.009662, 0.005254,
                                       0.002597, 0.001117,    0.000384, 8.8E-05,  7.0E-06,  0.0,      0.0 };

    // Verify curve has expected number of points (allowing some tolerance)
    EXPECT_GE( krogCurve->saturationVals.size(), expectedSg.size() - 5 ) << "KROG curve has too few points";
    EXPECT_LE( krogCurve->saturationVals.size(), expectedSg.size() + 5 ) << "KROG curve has too many points";

    // Verify that saturation values are gas saturations between 0 and 1
    for ( size_t i = 0; i < krogCurve->saturationVals.size(); ++i )
    {
        EXPECT_GE( krogCurve->saturationVals[i], 0.0 ) << "Gas saturation should be >= 0";
        EXPECT_LE( krogCurve->saturationVals[i], 1.0 ) << "Gas saturation should be <= 1";
    }

    // Verify that y-values are relative permeabilities between 0 and 1
    for ( size_t i = 0; i < krogCurve->yVals.size(); ++i )
    {
        EXPECT_GE( krogCurve->yVals[i], 0.0 ) << "Relative permeability should be >= 0";
        EXPECT_LE( krogCurve->yVals[i], 1.0 ) << "Relative permeability should be <= 1";
    }

    // Verify oil relative permeability decreases with increasing gas saturation (monotonic)
    for ( size_t i = 1; i < krogCurve->yVals.size(); ++i )
    {
        EXPECT_LE( krogCurve->yVals[i], krogCurve->yVals[i - 1] + 1e-10 )
            << "Oil relative permeability should decrease with increasing gas saturation";
    }

    // Compare actual values with expected values from OMP test
    // Use a tolerance for floating point comparison
    const double tolerance = 1e-5;

    // Compare saturation values (gas saturations)
    size_t minSgSize = std::min( krogCurve->saturationVals.size(), expectedSg.size() );
    for ( size_t i = 0; i < minSgSize; ++i )
    {
        EXPECT_NEAR( krogCurve->saturationVals[i], expectedSg[i], tolerance )
            << "Gas saturation mismatch at index " << i << " - Expected: " << expectedSg[i] << ", Actual: " << krogCurve->saturationVals[i];
    }

    // Compare relative permeability values
    size_t minKrSize = std::min( krogCurve->yVals.size(), expectedKr.size() );
    for ( size_t i = 0; i < minKrSize; ++i )
    {
        EXPECT_NEAR( krogCurve->yVals[i], expectedKr[i], tolerance )
            << "Relative permeability mismatch at index " << i << " - Expected: " << expectedKr[i] << ", Actual: " << krogCurve->yVals[i];
    }
}
