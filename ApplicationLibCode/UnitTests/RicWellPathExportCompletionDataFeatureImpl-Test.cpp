/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "Commands/CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "CompletionExportCommands/RicTransmissibilityCalculator.h"

#include "RiaFractureDefines.h"
#include "RifReaderMockModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCompletionData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigPerforationTransmissibilityEquations.h"

#include "Well/RigWellPath.h"

#include "RimEclipseResultCase.h"
#include "RimNonDarcyPerforationParameters.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmField.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace
{
void addStaticResult( RigCaseCellResultsData* cellResults, const QString& name, size_t cellCount, double value )
{
    RigEclipseResultAddress address( RiaDefines::ResultCatType::STATIC_NATIVE, name );
    cellResults->createResultEntry( address, false );
    cellResults->setTimeStepInfos( address, { RigEclipseTimeStepInfo( QDateTime(), 0, 0.0 ) } );

    auto* timesteps = cellResults->modifiableCellScalarResultTimesteps( address );
    timesteps->resize( 1 );
    ( *timesteps )[0].assign( cellCount, value );
}

// 2x2x2 grid with 50 unit cells, z = 0 at the top. PERMX = PERMY = 100 mD, PERMZ = 10 mD, NTG = 1, PORO = 0.2
RimEclipseResultCase* createMockEclipseCase( RiaDefines::EclipseUnitSystem unitSystem )
{
    auto* eclipseCase = new RimEclipseResultCase;

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( eclipseCase );
    caseData->setUnitsType( unitSystem );
    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, -100 ), cvf::Vec3d( 100, 100, 0 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
    mockReader->enableWellData( false );
    mockReader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();

    eclipseCase->setReservoirData( caseData.p() );

    const size_t            cellCount   = caseData->mainGrid()->totalCellCount();
    RigCaseCellResultsData* cellResults = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    addStaticResult( cellResults, "DX", cellCount, 50.0 );
    addStaticResult( cellResults, "DY", cellCount, 50.0 );
    addStaticResult( cellResults, "DZ", cellCount, 50.0 );
    addStaticResult( cellResults, "PERMX", cellCount, 100.0 );
    addStaticResult( cellResults, "PERMY", cellCount, 100.0 );
    addStaticResult( cellResults, "PERMZ", cellCount, 10.0 );
    addStaticResult( cellResults, "NTG", cellCount, 1.0 );
    addStaticResult( cellResults, "PORO", cellCount, 0.2 );

    return eclipseCase;
}

// Vertical well through the two upper-left cells, perforated over the full grid height
class PerforatedWellFixture
{
public:
    explicit PerforatedWellFixture( RiaDefines::EclipseUnitSystem unitSystem )
        : unitSystem( unitSystem )
        , wellRadius( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD ? 0.328 : 0.1 )
        , eclipseCase( createMockEclipseCase( unitSystem ) )
        , project( new RimProject )
        , wellPath( new RimWellPath )
    {
        wellPath->setName( "TestWell" );
        wellPath->setUnitSystem( unitSystem );

        cvf::ref<RigWellPath> geometry = new RigWellPath( { cvf::Vec3d( 25, 25, 10 ), cvf::Vec3d( 25, 25, -110 ) }, { 0.0, 120.0 } );
        wellPath->setWellPathGeometry( geometry.p() );
        project->oilFields[0]->wellPathCollection->addWellPath( wellPath );

        auto* interval = new RimPerforationInterval;
        interval->setStartAndEndMD( 10.0, 110.0 );
        interval->setDiameter( 2.0 * wellRadius );
        interval->setSkinFactor( 0.0 );
        wellPath->perforationIntervalCollection()->appendPerforation( interval );
    }

    ~PerforatedWellFixture()
    {
        delete eclipseCase;
        project->close();
        delete project;
    }

    void setNonDarcyType( RimNonDarcyPerforationParameters::NonDarcyFlowEnum type )
    {
        auto* params = const_cast<RimNonDarcyPerforationParameters*>( wellPath->perforationIntervalCollection()->nonDarcyParameters() );
        auto* field  = dynamic_cast<caf::PdmField<caf::AppEnum<RimNonDarcyPerforationParameters::NonDarcyFlowEnum>>*>(
            params->findField( "NonDarcyFlowType" ) );
        ASSERT_NE( nullptr, field );
        field->setValue( type );
    }

    void setUserDefinedDFactor( double dFactor )
    {
        auto* params = const_cast<RimNonDarcyPerforationParameters*>( wellPath->perforationIntervalCollection()->nonDarcyParameters() );
        auto* field  = dynamic_cast<caf::PdmField<double>*>( params->findField( "UserDefinedDFactor" ) );
        ASSERT_NE( nullptr, field );
        field->setValue( dFactor );
    }

    std::vector<RigCompletionData> completions()
    {
        return RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eclipseCase );
    }

    RiaDefines::EclipseUnitSystem unitSystem;
    double                        wellRadius;
    RimEclipseResultCase*         eclipseCase;
    RimProject*                   project;
    RimWellPath*                  wellPath;
};

RigCompletionData makeCompletion( double startMD, double endMD, std::optional<int> completionNumber )
{
    RigCompletionData completion( "W", RigCompletionDataGridCell( 0, nullptr ), startMD );
    completion.setDepthRange( startMD, endMD );
    if ( completionNumber ) completion.setCompletionNumber( *completionNumber );
    return completion;
}

std::optional<int> numberForAllOrders( std::vector<RigCompletionData> completions )
{
    std::vector<size_t> order( completions.size() );
    for ( size_t i = 0; i < order.size(); ++i )
        order[i] = i;

    std::optional<int> first;
    bool               firstSet = false;
    do
    {
        std::vector<RigCompletionData> permuted;
        for ( size_t i : order )
            permuted.push_back( completions[i] );

        auto number = RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( permuted );
        if ( !firstSet )
        {
            first    = number;
            firstSet = true;
        }
        EXPECT_EQ( first, number );
    } while ( std::next_permutation( order.begin(), order.end() ) );

    return first;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A cell cut by two perforation intervals gets the number of the interval with the largest overlap,
/// regardless of the order the contributions arrive in.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberUsesLargestOverlap )
{
    EXPECT_EQ( 2, numberForAllOrders( { makeCompletion( 100.0, 101.0, 1 ), makeCompletion( 101.0, 105.0, 2 ) } ) );
    EXPECT_EQ( 1, numberForAllOrders( { makeCompletion( 100.0, 104.0, 1 ), makeCompletion( 104.0, 105.0, 2 ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberTieGoesToLowestNumber )
{
    EXPECT_EQ( 3, numberForAllOrders( { makeCompletion( 100.0, 102.0, 5 ), makeCompletion( 102.0, 104.0, 3 ) } ) );
}

//--------------------------------------------------------------------------------------------------
/// The same interval can cross a cell more than once; its lengths are summed.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberSumsOverlapPerNumber )
{
    EXPECT_EQ( 1,
               numberForAllOrders(
                   { makeCompletion( 100.0, 102.0, 1 ), makeCompletion( 102.0, 105.0, 2 ), makeCompletion( 110.0, 112.0, 1 ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberIgnoresContributionsWithoutNumber )
{
    EXPECT_EQ( 4, numberForAllOrders( { makeCompletion( 100.0, 110.0, std::nullopt ), makeCompletion( 110.0, 111.0, 4 ) } ) );
    EXPECT_EQ( std::nullopt, numberForAllOrders( { makeCompletion( 100.0, 110.0, std::nullopt ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberEmptyCompletionsGivesNoNumber )
{
    EXPECT_EQ( std::nullopt, RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( {} ) );
}

class CompdatNonDarcyTest : public ::testing::TestWithParam<RiaDefines::EclipseUnitSystem>
{
};

//--------------------------------------------------------------------------------------------------
/// Without non-Darcy flow, COMPDAT connections get a transmissibility but the D-factor is defaulted
//--------------------------------------------------------------------------------------------------
TEST_P( CompdatNonDarcyTest, WithoutNonDarcyHasDefaultDFactor )
{
    PerforatedWellFixture fixture( GetParam() );
    fixture.setNonDarcyType( RimNonDarcyPerforationParameters::NON_DARCY_NONE );

    auto completions = fixture.completions();
    ASSERT_EQ( 2u, completions.size() );

    for ( const auto& completion : completions )
    {
        EXPECT_GT( completion.transmissibility(), 0.0 );
        EXPECT_TRUE( RigCompletionData::isDefaultValue( completion.dFactor() ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// User-defined well D-factor is distributed as D_conn = D_well * sum(T) / T_conn and exported negative
//--------------------------------------------------------------------------------------------------
TEST_P( CompdatNonDarcyTest, WithUserDefinedDFactor )
{
    PerforatedWellFixture fixture( GetParam() );
    fixture.setNonDarcyType( RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED );
    fixture.setUserDefinedDFactor( 0.5 );

    auto completions = fixture.completions();
    ASSERT_EQ( 2u, completions.size() );

    double sumT = 0.0;
    for ( const auto& completion : completions )
        sumT += completion.transmissibility();

    for ( const auto& completion : completions )
    {
        EXPECT_LT( completion.dFactor(), 0.0 );
        EXPECT_NEAR( -0.5 * sumT / completion.transmissibility(), completion.dFactor(), 1.0e-9 );
    }

    // Identical cells share the well D-factor equally
    EXPECT_NEAR( completions[0].dFactor(), completions[1].dFactor(), 1.0e-9 );
    EXPECT_NEAR( -1.0, completions[0].dFactor(), 1.0e-9 );
}

//--------------------------------------------------------------------------------------------------
/// Computed D-factor must match the D-factor equation applied to the connection's effective h and K
//--------------------------------------------------------------------------------------------------
TEST_P( CompdatNonDarcyTest, WithComputedDFactor )
{
    PerforatedWellFixture fixture( GetParam() );
    fixture.setNonDarcyType( RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED );

    auto completions = fixture.completions();
    ASSERT_EQ( 2u, completions.size() );

    const auto*  params = fixture.wellPath->perforationIntervalCollection()->nonDarcyParameters();
    const double alpha  = RiaDefines::nonDarcyFlowAlpha( GetParam() );

    for ( const auto& completion : completions )
    {
        // Vertical well: 50 length units of well inside each cell, along K
        auto data = RicTransmissibilityCalculator::calculateTransmissibilityData( fixture.eclipseCase,
                                                                                  fixture.wellPath,
                                                                                  cvf::Vec3d( 0.0, 0.0, 50.0 ),
                                                                                  0.0,
                                                                                  fixture.wellRadius,
                                                                                  completion.completionDataGridCell().globalCellIndex(),
                                                                                  false );
        ASSERT_TRUE( data.isValid() );

        const double effectiveK = params->gridPermeabilityScalingFactor() * data.effectiveK();

        const double beta = RigPerforationTransmissibilityEquations::betaFactor( params->inertialCoefficientBeta0(),
                                                                                 effectiveK,
                                                                                 params->permeabilityScalingFactor(),
                                                                                 0.2,
                                                                                 params->porosityScalingFactor() );

        const double expected = RigPerforationTransmissibilityEquations::dFactor( alpha,
                                                                                  beta,
                                                                                  effectiveK,
                                                                                  data.effectiveH(),
                                                                                  params->wellRadius(),
                                                                                  params->relativeGasDensity(),
                                                                                  params->gasViscosity() );

        ASSERT_TRUE( std::isfinite( expected ) );
        EXPECT_GT( expected, 0.0 );
        EXPECT_FALSE( RigCompletionData::isDefaultValue( completion.dFactor() ) );
        EXPECT_NEAR( -expected, completion.dFactor(), 1.0e-9 * expected );
    }
}

//--------------------------------------------------------------------------------------------------
/// The connection factor itself must not depend on the non-Darcy setting
//--------------------------------------------------------------------------------------------------
TEST_P( CompdatNonDarcyTest, TransmissibilityIndependentOfNonDarcy )
{
    PerforatedWellFixture fixture( GetParam() );

    fixture.setNonDarcyType( RimNonDarcyPerforationParameters::NON_DARCY_NONE );
    auto without = fixture.completions();

    fixture.setNonDarcyType( RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED );
    auto with = fixture.completions();

    ASSERT_EQ( without.size(), with.size() );
    for ( size_t i = 0; i < with.size(); ++i )
    {
        EXPECT_DOUBLE_EQ( without[i].transmissibility(), with[i].transmissibility() );
    }
}

INSTANTIATE_TEST_SUITE_P( MetricAndField,
                          CompdatNonDarcyTest,
                          ::testing::Values( RiaDefines::EclipseUnitSystem::UNITS_METRIC, RiaDefines::EclipseUnitSystem::UNITS_FIELD ),
                          []( const ::testing::TestParamInfo<RiaDefines::EclipseUnitSystem>& info )
                          { return info.param == RiaDefines::EclipseUnitSystem::UNITS_FIELD ? "Field" : "Metric"; } );
