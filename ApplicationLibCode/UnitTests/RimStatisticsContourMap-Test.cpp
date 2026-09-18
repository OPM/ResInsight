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

#include "RiaDefines.h"
#include "RiaTestDataDirectory.h"

#include "ContourMap/RigContourMapCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "ContourMap/RimStatisticsContourMap.h"
#include "RimCellRangeFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimReservoirGridEnsemble.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QDir>
#include <QFile>

#include <cmath>
#include <memory>

namespace
{
using GridModeType   = RimReservoirGridEnsembleBase::GridModeType;
using GridImportMode = RimStatisticsContourMap::GridImportMode;

RimEclipseResultCase* openBruggeRealization( const QString& realizationFolder, const QString& fileName )
{
    QDir baseFolder( TEST_MODEL_DIR );
    if ( !baseFolder.cd( QString( "Case_with_10_timesteps/%1" ).arg( realizationFolder ) ) ) return nullptr;

    QString filePath = baseFolder.absoluteFilePath( fileName );
    if ( !QFile::exists( filePath ) ) return nullptr;

    auto eclipseCase = std::make_unique<RimEclipseResultCase>();
    eclipseCase->setCaseInfo( realizationFolder, filePath );
    if ( !eclipseCase->openEclipseGridFile() ) return nullptr;

    return eclipseCase.release();
}

std::unique_ptr<RimReservoirGridEnsemble> createBruggeEnsemble( GridModeType gridMode )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    for ( const auto& [folder, fileName] : std::vector<std::pair<QString, QString>>{ { "Real0", "BRUGGE_0000.EGRID" },
                                                                                     { "Real10", "BRUGGE_0010.EGRID" },
                                                                                     { "Real30", "BRUGGE_0030.EGRID" },
                                                                                     { "Real40", "BRUGGE_0040.EGRID" } } )
    {
        auto* eclipseCase = openBruggeRealization( folder, fileName );
        if ( !eclipseCase ) return nullptr;
        ensemble->addCase( eclipseCase );
    }

    auto* autoDetectField = dynamic_cast<caf::PdmField<bool>*>( ensemble->findField( "AutoDetectGridType" ) );
    auto* gridModeField   = dynamic_cast<caf::PdmField<caf::AppEnum<GridModeType>>*>( ensemble->findField( "GridMode" ) );
    if ( !autoDetectField || !gridModeField ) return nullptr;

    autoDetectField->setValue( false );
    gridModeField->setValue( gridMode );

    return ensemble;
}

RimStatisticsContourMap* addPoroContourMap( RimReservoirGridEnsemble* ensemble, GridImportMode importMode )
{
    auto* map = new RimStatisticsContourMap;
    ensemble->addStatisticsContourMap( map );
    map->setEclipseCase( ensemble->cases().front() );
    map->setGridImportMode( importMode );

    auto* aggregationField =
        dynamic_cast<caf::PdmField<caf::AppEnum<RigContourMapCalculator::ResultAggregationType>>*>( map->findField( "ResultAggregation" ) );
    auto* resultDefinitionField = dynamic_cast<caf::PdmChildField<RimEclipseResultDefinition*>*>( map->findField( "ResultDefinition" ) );
    if ( !aggregationField || !resultDefinitionField ) return nullptr;

    aggregationField->setValue( RigContourMapCalculator::MEAN );
    ( *resultDefinitionField )()->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    ( *resultDefinitionField )()->setResultVariable( "PORO" );

    return map;
}

void setDataFilter( RimStatisticsContourMap* map, RimCellFilter* filter )
{
    auto* dataFilterField = dynamic_cast<caf::PdmPtrField<RimCellFilter*>*>( map->findField( "DataFilter" ) );
    ASSERT_TRUE( dataFilterField != nullptr );
    dataFilterField->setValue( filter );
}

// Filter including the top K layer of the grid only
RimCellRangeFilter* addTopLayerFilter( RimReservoirGridEnsemble* ensemble )
{
    const RigMainGrid* mainGrid = ensemble->cases().front()->eclipseCaseData()->mainGrid();

    auto* rangeFilter        = ensemble->dataFilterCollection()->addNewRangeFilter();
    rangeFilter->startIndexI = 1;
    rangeFilter->startIndexJ = 1;
    rangeFilter->startIndexK = 1;
    rangeFilter->cellCountI  = static_cast<int>( mainGrid->cellCountI() );
    rangeFilter->cellCountJ  = static_cast<int>( mainGrid->cellCountJ() );
    rangeFilter->cellCountK  = 1;

    return rangeFilter;
}

size_t countValidValues( const std::vector<double>& values )
{
    return std::count_if( values.begin(), values.end(), []( double v ) { return std::isfinite( v ); } );
}

size_t countDifferentValues( const std::vector<double>& lhs, const std::vector<double>& rhs )
{
    size_t differentCount = 0;
    for ( size_t i = 0; i < lhs.size(); i++ )
    {
        const bool lhsValid = std::isfinite( lhs[i] );
        const bool rhsValid = std::isfinite( rhs[i] );
        if ( lhsValid != rhsValid || ( lhsValid && std::abs( lhs[i] - rhs[i] ) > 1.0e-9 ) ) differentCount++;
    }
    return differentCount;
}

//--------------------------------------------------------------------------------------------------
/// Compute the mean PORO contour map with and without a top layer data filter, and verify that the
/// filter changes the result
//--------------------------------------------------------------------------------------------------
std::vector<double> verifyDataFilterIsApplied( GridModeType gridMode, GridImportMode importMode )
{
    auto ensemble = createBruggeEnsemble( gridMode );
    if ( !ensemble ) ADD_FAILURE() << "Failed to create Brugge ensemble";
    if ( !ensemble ) return {};

    auto* map = addPoroContourMap( ensemble.get(), importMode );
    if ( !map ) ADD_FAILURE() << "Failed to create contour map";
    if ( !map ) return {};

    RimStatisticsContourMap::computeStatisticsForMaps( { map } );
    const auto unfiltered = map->result( 0, RimStatisticsContourMap::StatisticsType::MEAN );
    EXPECT_GT( countValidValues( unfiltered ), 0u );

    setDataFilter( map, addTopLayerFilter( ensemble.get() ) );
    RimStatisticsContourMap::computeStatisticsForMaps( { map } );
    const auto filtered = map->result( 0, RimStatisticsContourMap::StatisticsType::MEAN );

    EXPECT_EQ( unfiltered.size(), filtered.size() );
    if ( unfiltered.size() != filtered.size() ) return {};

    EXPECT_GT( countValidValues( filtered ), 0u );
    EXPECT_GT( countDifferentValues( unfiltered, filtered ), 0u );

    // An EXCLUDE filter of the same layer must give yet another result
    auto* excludeFilter = addTopLayerFilter( ensemble.get() );
    excludeFilter->setFilterMode( RimCellFilter::EXCLUDE );
    setDataFilter( map, excludeFilter );
    RimStatisticsContourMap::computeStatisticsForMaps( { map } );
    const auto excluded = map->result( 0, RimStatisticsContourMap::StatisticsType::MEAN );

    EXPECT_GT( countDifferentValues( filtered, excluded ), 0u );

    return filtered;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimStatisticsContourMapTest, DataFilterSharedGrid )
{
    verifyDataFilterIsApplied( GridModeType::SHARED_GRID, GridImportMode::SHARED_GRID );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimStatisticsContourMapTest, DataFilterIndividualGridImport )
{
    verifyDataFilterIsApplied( GridModeType::SHARED_GRID, GridImportMode::INDIVIDUAL_GRIDS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimStatisticsContourMapTest, DataFilterIndividualGridEnsemble )
{
    verifyDataFilterIsApplied( GridModeType::INDIVIDUAL_GRIDS, GridImportMode::SHARED_GRID );
}

//--------------------------------------------------------------------------------------------------
/// The Brugge realizations have identical grids, so the shared and individual grid code paths must
/// give the same filtered result
//--------------------------------------------------------------------------------------------------
TEST( RimStatisticsContourMapTest, DataFilterSharedAndIndividualGridsAgree )
{
    const auto shared     = verifyDataFilterIsApplied( GridModeType::SHARED_GRID, GridImportMode::SHARED_GRID );
    const auto individual = verifyDataFilterIsApplied( GridModeType::INDIVIDUAL_GRIDS, GridImportMode::INDIVIDUAL_GRIDS );

    ASSERT_FALSE( shared.empty() );
    ASSERT_EQ( shared.size(), individual.size() );
    EXPECT_EQ( 0u, countDifferentValues( shared, individual ) );
}
