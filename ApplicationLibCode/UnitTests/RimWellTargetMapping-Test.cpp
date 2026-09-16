/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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

#include "RiaPorosityModel.h"
#include "RiaResultNames.h"
#include "RiaTestDataDirectory.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include "RimCellFilterTools.h"
#include "RimCellRangeFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimRegularGridCase.h"
#include "RimReservoirGridEnsemble.h"
#include "RimWellTargetMapping.h"

#include <QDir>

#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
class TestableWellTargetMapping : public RimWellTargetMapping
{
public:
    using RimWellTargetMapping::calculateValueOptions;
    using RimWellTargetMapping::initAfterRead;
};

std::unique_ptr<RimEclipseResultCase> openCase( const QString& realization, const QString& fileName )
{
    auto eclipseCase = std::make_unique<RimEclipseResultCase>();
    auto filePath    = QDir( TEST_MODEL_DIR ).absoluteFilePath( "Case_with_10_timesteps/" + realization + "/" + fileName );
    eclipseCase->setCaseInfo( realization, filePath );
    if ( !eclipseCase->openEclipseGridFile() ) return nullptr;
    return eclipseCase;
}

template <typename T>
void setField( caf::PdmObject& object, const QString& keyword, const T& value )
{
    auto field = dynamic_cast<caf::PdmField<T>*>( object.findField( keyword ) );
    ASSERT_NE( nullptr, field );
    field->setValue( value );
}

void expectFilteredClusters( RimEclipseCase* eclipseCase, RimCellFilter* filter, size_t timeStep )
{
    auto visibility = RimCellFilterTools::computeReservoirCellVisibility( filter, eclipseCase, timeStep );
    ASSERT_TRUE( visibility.notNull() );
    auto                    results = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigEclipseResultAddress address( RiaDefines::ResultCatType::GENERATED, RigWellTargetMapping::wellTargetResultName() );
    const auto&             clusters      = results->cellScalarResults( address, timeStep );
    auto                    activeIndices = results->activeCellInfo()->activeReservoirCellIndices();
    ASSERT_EQ( activeIndices.size(), clusters.size() );

    size_t clusteredCells = 0;
    size_t excludedCells  = 0;
    for ( size_t i = 0; i < clusters.size(); ++i )
    {
        const bool isClustered = std::isfinite( clusters[i] ) && clusters[i] > 0.0;
        if ( isClustered ) ++clusteredCells;
        if ( !visibility->val( activeIndices[i].value() ) )
        {
            ++excludedCells;
            EXPECT_FALSE( isClustered );
        }
    }
    EXPECT_GT( clusteredCells, 0u );
    EXPECT_GT( excludedCells, 0u );
}
} // namespace

TEST( RimWellTargetMappingTest, FilterOptionsUseOwningCase )
{
    RimEclipseResultCase eclipseCase;
    auto                 mapping = new TestableWellTargetMapping;
    eclipseCase.addWellTargetMapping( mapping );
    auto filter = eclipseCase.dataFilterCollection()->addNewRangeFilter();
    filter->setName( "Case filter" );

    auto options = mapping->calculateValueOptions( mapping->findField( "DataFilter" ) );
    ASSERT_EQ( 2, options.size() );
    EXPECT_EQ( "None", options[0].optionUiText() );
    EXPECT_EQ( caf::PdmOptionItemInfo( "Case filter", filter ).value(), options[1].value() );
}

TEST( RimWellTargetMappingTest, FilterOptionsUseOwningEnsemble )
{
    RimReservoirGridEnsemble ensemble;
    auto                     eclipseCase = new RimEclipseResultCase;
    ensemble.addCase( eclipseCase );
    eclipseCase->dataFilterCollection()->addNewRangeFilter();
    auto mapping = new TestableWellTargetMapping;
    ensemble.addWellTargetMapping( mapping );
    auto filter = ensemble.dataFilterCollection()->addNewRangeFilter();
    filter->setName( "Ensemble filter" );

    auto options = mapping->calculateValueOptions( mapping->findField( "DataFilter" ) );
    ASSERT_EQ( 2, options.size() );
    EXPECT_EQ( "None", options[0].optionUiText() );
    EXPECT_EQ( caf::PdmOptionItemInfo( "Ensemble filter", filter ).value(), options[1].value() );
}

TEST( RimWellTargetMappingTest, ClearingFilterRestoresCandidates )
{
    auto eclipseCase = openCase( "Real0", "BRUGGE_0000.EGRID" );
    ASSERT_NE( nullptr, eclipseCase );
    auto filter = eclipseCase->dataFilterCollection()->addNewPropertyFilter();
    filter->resultDefinition()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    filter->resultDefinition()->setResultVariable( RiaResultNames::swat() );
    setField( *filter, "LowerBound", 0.0 );
    setField( *filter, "UpperBound", 1.0 );
    filter->setFilterMode( RimCellFilter::EXCLUDE );

    auto mapping = new TestableWellTargetMapping;
    eclipseCase->addWellTargetMapping( mapping );
    setField( *mapping, "TimeStep", 5 );
    setField( *mapping, "Iterations", 10 );
    setField( *mapping, "MaxNumTargets", 1 );
    auto filterField = dynamic_cast<caf::PdmPtrField<RimCellFilter*>*>( mapping->findField( "DataFilter" ) );
    ASSERT_NE( nullptr, filterField );
    *filterField = filter;
    mapping->initAfterRead();

    auto                    results = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigEclipseResultAddress address( RiaDefines::ResultCatType::GENERATED, RigWellTargetMapping::wellTargetResultName() );
    const auto&             clusters = results->cellScalarResults( address, 5 );
    ASSERT_FALSE( clusters.empty() );
    auto isClustered = []( double value ) { return std::isfinite( value ) && value > 0.0; };
    EXPECT_EQ( 0, std::count_if( clusters.begin(), clusters.end(), isClustered ) );

    *filterField = nullptr;
    mapping->initAfterRead();
    const auto& unfilteredClusters = results->cellScalarResults( address, 5 );
    EXPECT_GT( std::count_if( unfilteredClusters.begin(), unfilteredClusters.end(), isClustered ), 0 );
}

class RimWellTargetMappingFilterTest : public testing::TestWithParam<std::tuple<bool, bool, bool>>
{
};

TEST_P( RimWellTargetMappingFilterTest, FiltersClustersWithoutViews )
{
    const auto [useEnsemble, useFormations, exclude] = GetParam();
    const size_t timeStep                            = 5;
    auto         firstCase                           = openCase( "Real0", "BRUGGE_0000.EGRID" );
    ASSERT_NE( nullptr, firstCase );
    RimReservoirGridEnsemble     ensemble;
    auto                         caseA = firstCase.get();
    std::vector<RimEclipseCase*> cases{ caseA };
    if ( useEnsemble )
    {
        ensemble.addCase( firstCase.release() );
        auto secondCase = openCase( "Real40", "BRUGGE_0040.EGRID" );
        ASSERT_NE( nullptr, secondCase );
        ensemble.addCase( secondCase.release() );
        cases = ensemble.cases();
    }

    auto collection = useEnsemble ? ensemble.dataFilterCollection() : caseA->dataFilterCollection();
    auto filter     = collection->addNewPropertyFilter();
    filter->setFilterMode( exclude ? RimCellFilter::EXCLUDE : RimCellFilter::INCLUDE );
    if ( useFormations )
    {
        int splitFraction = 1;
        for ( auto eclipseCase : cases )
        {
            const int         layerCount = static_cast<int>( eclipseCase->eclipseCaseData()->mainGrid()->cellCountK() );
            const int         splitLayer = layerCount * splitFraction / 3;
            RigFormationNames formations;
            formations.appendFormationRange( "Upper", 0, splitLayer );
            formations.appendFormationRange( "Lower", splitLayer + 1, layerCount - 1 );
            eclipseCase->eclipseCaseData()->setActiveFormationNames( &formations );
            ++splitFraction;
        }
        filter->resultDefinition()->setResultType( RiaDefines::ResultCatType::FORMATION_NAMES );
        filter->resultDefinition()->setResultVariable( RiaResultNames::activeFormationNamesResultName() );
        filter->setToDefaultValues();
        setField( *filter, "SelectedValues", std::vector<int>{ 0 } );
    }
    else
    {
        filter->resultDefinition()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        filter->resultDefinition()->setResultVariable( RiaResultNames::swat() );
        setField( *filter, "LowerBound", 0.3 );
        setField( *filter, "UpperBound", 0.6 );
    }

    if ( useEnsemble )
    {
        RigWellTargetMapping::ClusteringLimits limits{};
        limits.maxNumTargets = 1;
        limits.maxIterations = 10;
        limits.dataFilter    = filter;
        RigFloodingSettings                 floodingSettings( RigFloodingSettings::FloodingType::WATER_FLOODING,
                                              0.0,
                                              RigFloodingSettings::FloodingType::GAS_FLOODING,
                                              0.0 );
        std::unique_ptr<RimRegularGridCase> statistics(
            RigWellTargetMapping::generateEnsembleCandidates( ensemble.cases(),
                                                              timeStep,
                                                              cvf::Vec3st( 10, 10, 3 ),
                                                              RigWellTargetMapping::VolumeType::OIL,
                                                              RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED,
                                                              RigWellTargetMapping::VolumeResultType::TOTAL,
                                                              floodingSettings,
                                                              limits,
                                                              0.0,
                                                              0.0 ) );
        ASSERT_NE( nullptr, statistics );
    }
    else
    {
        auto mapping = new TestableWellTargetMapping;
        caseA->addWellTargetMapping( mapping );
        auto filterField = dynamic_cast<caf::PdmPtrField<RimCellFilter*>*>( mapping->findField( "DataFilter" ) );
        ASSERT_NE( nullptr, filterField );
        *filterField = filter;
        setField( *mapping, "TimeStep", static_cast<int>( timeStep ) );
        setField( *mapping, "Iterations", 10 );
        setField( *mapping, "MaxNumTargets", 1 );
        mapping->initAfterRead();
    }

    for ( auto eclipseCase : cases )
    {
        expectFilteredClusters( eclipseCase, filter, timeStep );
    }
}

INSTANTIATE_TEST_SUITE_P( SingleCaseAndEnsemble,
                          RimWellTargetMappingFilterTest,
                          testing::Combine( testing::Bool(), testing::Bool(), testing::Bool() ) );
