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
#include "RiaResultNames.h"
#include "RiaTestDataDirectory.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimCellFilterTools.h"
#include "RimCellRangeFilter.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"

#include "cafPdmField.h"

#include <QDir>
#include <QFile>

#include <cmath>
#include <memory>

static std::unique_ptr<RimEclipseResultCase> openBruggeCase( const QString& realizationFolder, const QString& fileName )
{
    QDir baseFolder( TEST_MODEL_DIR );
    if ( !baseFolder.cd( QString( "Case_with_10_timesteps/%1" ).arg( realizationFolder ) ) ) return nullptr;

    QString filePath = baseFolder.absoluteFilePath( fileName );
    if ( !QFile::exists( filePath ) ) return nullptr;

    auto eclipseCase = std::make_unique<RimEclipseResultCase>();
    eclipseCase->setCaseInfo( realizationFolder, filePath );
    if ( !eclipseCase->openEclipseGridFile() ) return nullptr;

    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCellFilterToolsTest, RangeFilterVisibility )
{
    auto eclipseCase = openBruggeCase( "Real0", "BRUGGE_0000.EGRID" );
    ASSERT_TRUE( eclipseCase != nullptr );

    RimCellRangeFilter rangeFilter;
    rangeFilter.setCase( eclipseCase.get() );
    rangeFilter.startIndexI = 10;
    rangeFilter.startIndexJ = 20;
    rangeFilter.startIndexK = 1;
    rangeFilter.cellCountI  = 5;
    rangeFilter.cellCountJ  = 4;
    rangeFilter.cellCountK  = 3;

    auto visibility = RimCellFilterTools::computeReservoirCellVisibility( &rangeFilter, eclipseCase.get(), 0 );
    ASSERT_TRUE( visibility.notNull() );

    const RigMainGrid* mainGrid = eclipseCase->eclipseCaseData()->mainGrid();
    ASSERT_EQ( mainGrid->totalCellCount(), visibility->size() );

    size_t visibleCount = 0;
    for ( size_t i = 0; i < visibility->size(); i++ )
    {
        if ( visibility->val( i ) ) visibleCount++;
    }

    // The Brugge grid has no LGRs, so the geometric mask is exactly the IJK box
    EXPECT_EQ( size_t( 5 * 4 * 3 ), visibleCount );

    // An EXCLUDE filter must select the complement
    rangeFilter.setFilterMode( RimCellFilter::EXCLUDE );

    auto excludeVisibility = RimCellFilterTools::computeReservoirCellVisibility( &rangeFilter, eclipseCase.get(), 0 );
    ASSERT_TRUE( excludeVisibility.notNull() );

    size_t excludeVisibleCount = 0;
    for ( size_t i = 0; i < excludeVisibility->size(); i++ )
    {
        if ( excludeVisibility->val( i ) ) excludeVisibleCount++;
    }

    EXPECT_EQ( visibility->size() - visibleCount, excludeVisibleCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCellFilterToolsTest, PropertyFilterVisibilityPerCase )
{
    auto caseA = openBruggeCase( "Real0", "BRUGGE_0000.EGRID" );
    auto caseB = openBruggeCase( "Real40", "BRUGGE_0040.EGRID" );
    ASSERT_TRUE( caseA != nullptr );
    ASSERT_TRUE( caseB != nullptr );

    const double lowerBound = 0.3;
    const double upperBound = 0.6;

    // Property filter bound to case A
    RimEclipsePropertyFilter propertyFilter;
    propertyFilter.setCase( caseA.get() );
    propertyFilter.resultDefinition()->setEclipseCase( caseA.get() );
    propertyFilter.resultDefinition()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    propertyFilter.resultDefinition()->setResultVariable( RiaResultNames::swat() );

    auto* lowerField = dynamic_cast<caf::PdmField<double>*>( propertyFilter.findField( "LowerBound" ) );
    auto* upperField = dynamic_cast<caf::PdmField<double>*>( propertyFilter.findField( "UpperBound" ) );
    ASSERT_TRUE( lowerField && upperField );
    lowerField->setValue( lowerBound );
    upperField->setValue( upperBound );

    const size_t timeStepIndex = 5;

    auto countVisible = []( const cvf::UByteArray* visibility )
    {
        size_t count = 0;
        for ( size_t i = 0; i < visibility->size(); i++ )
        {
            if ( visibility->val( i ) ) count++;
        }
        return count;
    };

    // Reference: count active cells with SWAT inside the bounds, using the case data directly
    auto expectedVisibleCount = [&]( RimEclipseCase* eclipseCase )
    {
        auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
        auto results       = eclipseCase->results( porosityModel );

        RigEclipseResultAddress swatAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() );
        results->ensureKnownResultLoaded( swatAddress );
        const auto& values = results->cellScalarResults( swatAddress, timeStepIndex );

        size_t count = 0;
        for ( double value : values )
        {
            if ( lowerBound <= value && value <= upperBound ) count++;
        }
        return count;
    };

    auto visibilityA = RimCellFilterTools::computeReservoirCellVisibility( &propertyFilter, caseA.get(), timeStepIndex );
    auto visibilityB = RimCellFilterTools::computeReservoirCellVisibility( &propertyFilter, caseB.get(), timeStepIndex );
    ASSERT_TRUE( visibilityA.notNull() );
    ASSERT_TRUE( visibilityB.notNull() );

    // The same filter definition must be evaluated against each case's own result values
    EXPECT_EQ( expectedVisibleCount( caseA.get() ), countVisible( visibilityA.p() ) );
    EXPECT_EQ( expectedVisibleCount( caseB.get() ), countVisible( visibilityB.p() ) );

    // The realizations have different SWAT fields, so the masks must differ
    ASSERT_EQ( visibilityA->size(), visibilityB->size() );
    size_t differingCells = 0;
    for ( size_t i = 0; i < visibilityA->size(); i++ )
    {
        if ( visibilityA->val( i ) != visibilityB->val( i ) ) differingCells++;
    }
    EXPECT_GT( differingCells, 0u );
}
