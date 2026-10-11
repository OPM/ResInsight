#include "gtest/gtest.h"

#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigEclipseContourMapProjection.h"
#include "RifReaderMockModel.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigFloodingSettings.h"
#include "RigMainGrid.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// A result with a single time step (e.g. an input property) must be usable for any time step.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseContourMapProjection, SingleTimeStepResultForLaterTimeStep )
{
    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
    mockReader->enableWellData( false );
    mockReader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();

    RigCaseCellResultsData* resultData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const double            propertyValue = 42.0;
    RigEclipseResultAddress address( RiaDefines::ResultCatType::INPUT_PROPERTY, "MYPROP" );
    resultData->createResultEntry( address, false );
    resultData->setTimeStepInfos( address, { RigEclipseTimeStepInfo( QDateTime(), 0, 0.0 ) } );
    auto* timeSteps = resultData->modifiableCellScalarResultTimesteps( address );
    timeSteps->resize( 1 );
    ( *timeSteps )[0].assign( caseData->mainGrid()->totalCellCount(), propertyValue );

    RigContourMapGrid              contourMapGrid( caseData->mainGrid()->boundingBox(), 10.0 );
    RigEclipseContourMapProjection projection( &contourMapGrid, caseData.p(), resultData );
    projection.generateGridMapping( RigContourMapCalculator::MEAN, {}, {} );

    RigFloodingSettings floodingSettings( RigFloodingSettings::FloodingType::WATER_FLOODING,
                                          0.0,
                                          RigFloodingSettings::FloodingType::GAS_FLOODING,
                                          0.0 );

    const int timeStep               = 3;
    auto [useActiveCellInfo, values] = RigEclipseContourMapProjection::generateResults( projection,
                                                                                        contourMapGrid,
                                                                                        *resultData,
                                                                                        address,
                                                                                        RigContourMapCalculator::MEAN,
                                                                                        timeStep,
                                                                                        floodingSettings );

    size_t definedCount = 0;
    for ( double value : values )
    {
        if ( std::isinf( value ) ) continue;
        EXPECT_DOUBLE_EQ( propertyValue, value );
        definedCount++;
    }
    EXPECT_GT( definedCount, 0u );
}
