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

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseResultAddress.h"
#include "RimEclipseResultCase.h"
#include "RimGridCalculation.h"
#include "RimGridCalculationCollection.h"
#include "RimGridCalculationVariable.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"

#include "cafPdmPtrField.h"

#include <QDir>
#include <QFile>

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, ReplaceInvalidValuesWithDefaultValue )
{
    const double        defaultValue = 0.0;
    std::vector<double> values =
        { 1.0, HUGE_VAL, 2.0, -HUGE_VAL, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -3.0, 0.0 };

    size_t replacedCount = RimGridCalculation::replaceInvalidValuesWithDefaultValue( defaultValue, values );

    EXPECT_EQ( 4u, replacedCount );

    const std::vector<double> expected = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0, -3.0, 0.0 };
    ASSERT_EQ( expected.size(), values.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( expected[i], values[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, ReplaceInvalidValuesWithDefaultValueNoInvalidValues )
{
    const std::vector<double> original = { 1.0, 2.0, -3.0, 0.0 };

    std::vector<double> values        = original;
    size_t              replacedCount = RimGridCalculation::replaceInvalidValuesWithDefaultValue( 42.0, values );

    EXPECT_EQ( 0u, replacedCount );
    EXPECT_EQ( original, values );

    std::vector<double> emptyValues;
    EXPECT_EQ( 0u, RimGridCalculation::replaceInvalidValuesWithDefaultValue( 42.0, emptyValues ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, DestinationEnsembleCasesAreOutputs )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    firstCase  = new RimEclipseResultCase;
    auto*                    secondCase = new RimEclipseResultCase;
    ensemble.addCase( firstCase );
    ensemble.addCase( secondCase );

    RimGridCalculation calculation;
    auto*              destinationEnsembleField =
        dynamic_cast<caf::PdmPtrField<RimReservoirGridEnsemble*>*>( calculation.findField( "DestinationEnsemble" ) );
    ASSERT_TRUE( destinationEnsembleField != nullptr );
    destinationEnsembleField->setValue( &ensemble );

    const auto outputCases = calculation.outputEclipseCases();
    ASSERT_EQ( 2u, outputCases.size() );
    EXPECT_EQ( firstCase, outputCases[0] );
    EXPECT_EQ( secondCase, outputCases[1] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, AdditionalEnsembleCasesAreOutputs )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    firstCase  = new RimEclipseResultCase;
    auto*                    secondCase = new RimEclipseResultCase;
    ensemble.addCase( firstCase );
    ensemble.addCase( secondCase );

    RimGridCalculation calculation;
    auto*              additionalCasesTypeField = dynamic_cast<caf::PdmField<caf::AppEnum<RimGridCalculation::AdditionalCasesType>>*>(
        calculation.findField( "AdditionalCasesType" ) );
    auto* additionalEnsembleField =
        dynamic_cast<caf::PdmPtrField<RimReservoirGridEnsemble*>*>( calculation.findField( "AdditionalEnsemble" ) );
    ASSERT_TRUE( additionalCasesTypeField != nullptr );
    ASSERT_TRUE( additionalEnsembleField != nullptr );

    additionalCasesTypeField->setValue( RimGridCalculation::AdditionalCasesType::ENSEMBLE );
    additionalEnsembleField->setValue( &ensemble );

    const auto outputCases = calculation.outputEclipseCases();
    ASSERT_EQ( 2u, outputCases.size() );
    EXPECT_EQ( firstCase, outputCases[0] );
    EXPECT_EQ( secondCase, outputCases[1] );
}

//--------------------------------------------------------------------------------------------------
/// The results of a case that is not opened are not available, and must not be accessed when the calculation is removed
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, RemoveDependentObjectsForCaseNotOpened )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    notOpenedCase = new RimEclipseResultCase;
    ensemble.addCase( notOpenedCase );

    RimGridCalculation calculation;
    calculation.setExpression( "MY_CALCULATION := 1" );

    auto* destinationEnsembleField =
        dynamic_cast<caf::PdmPtrField<RimReservoirGridEnsemble*>*>( calculation.findField( "DestinationEnsemble" ) );
    ASSERT_TRUE( destinationEnsembleField != nullptr );
    destinationEnsembleField->setValue( &ensemble );

    ASSERT_TRUE( notOpenedCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) == nullptr );

    calculation.removeDependentObjects();
}

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* openBruggeRealizationForCalculation( const QString& realizationFolder, const QString& fileName )
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
} // namespace

//--------------------------------------------------------------------------------------------------
/// A plain per-cell expression is computed lazily for a single realization on demand, so clicking
/// "Calculate" for an ensemble destination must only compute the ensemble's main case.
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, EnsembleDestinationWithPlainExpressionOnlyCalculatesMainCase )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    firstCase  = openBruggeRealizationForCalculation( "Real0", "BRUGGE_0000.EGRID" );
    auto*                    secondCase = openBruggeRealizationForCalculation( "Real10", "BRUGGE_0010.EGRID" );
    ASSERT_TRUE( firstCase != nullptr );
    ASSERT_TRUE( secondCase != nullptr );
    ensemble.addCase( firstCase );
    ensemble.addCase( secondCase );

    RimGridCalculation calculation;
    calculation.setExpression( "MyCalc := x + 1" );
    auto* variable = dynamic_cast<RimGridCalculationVariable*>( calculation.addVariable( "x" ) );
    ASSERT_TRUE( variable != nullptr );

    RimEclipseResultAddress sourceAddress;
    sourceAddress.setEclipseCase( firstCase );
    sourceAddress.setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    sourceAddress.setResultName( "PORO" );
    variable->setEclipseResultAddress( sourceAddress );

    auto* destinationEnsembleField =
        dynamic_cast<caf::PdmPtrField<RimReservoirGridEnsemble*>*>( calculation.findField( "DestinationEnsemble" ) );
    ASSERT_TRUE( destinationEnsembleField != nullptr );
    destinationEnsembleField->setValue( &ensemble );

    ASSERT_TRUE( calculation.calculate() );

    const RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, "MyCalc" );
    EXPECT_TRUE( firstCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasResultEntry( resAddr ) );

    // The second realization's result is not computed, since only the ensemble's main case is needed
    ASSERT_TRUE( secondCase->eclipseCaseData() != nullptr );
    EXPECT_FALSE( secondCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasResultEntry( resAddr ) );
}

//--------------------------------------------------------------------------------------------------
/// An aggregation expression's result is the per-realization summary itself, so it cannot be produced
/// lazily and must be computed for every realization, even for an ensemble destination.
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, EnsembleDestinationWithAggregationExpressionCalculatesAllCases )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    firstCase  = openBruggeRealizationForCalculation( "Real0", "BRUGGE_0000.EGRID" );
    auto*                    secondCase = openBruggeRealizationForCalculation( "Real10", "BRUGGE_0010.EGRID" );
    ASSERT_TRUE( firstCase != nullptr );
    ASSERT_TRUE( secondCase != nullptr );
    ensemble.addCase( firstCase );
    ensemble.addCase( secondCase );

    RimGridCalculation calculation;
    calculation.setExpression( "MyCalc := sum(x)" );
    auto* variable = dynamic_cast<RimGridCalculationVariable*>( calculation.addVariable( "x" ) );
    ASSERT_TRUE( variable != nullptr );

    RimEclipseResultAddress sourceAddress;
    sourceAddress.setEclipseCase( firstCase );
    sourceAddress.setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    sourceAddress.setResultName( "PORO" );
    variable->setEclipseResultAddress( sourceAddress );

    auto* destinationEnsembleField =
        dynamic_cast<caf::PdmPtrField<RimReservoirGridEnsemble*>*>( calculation.findField( "DestinationEnsemble" ) );
    ASSERT_TRUE( destinationEnsembleField != nullptr );
    destinationEnsembleField->setValue( &ensemble );

    ASSERT_TRUE( calculation.calculate() );

    const RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, "MyCalc" );
    EXPECT_TRUE( firstCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasResultEntry( resAddr ) );

    // An aggregation expression's result cannot be produced lazily for a single realization, so every
    // realization must be computed eagerly
    ASSERT_TRUE( secondCase->eclipseCaseData() != nullptr );
    EXPECT_TRUE( secondCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasResultEntry( resAddr ) );
}

//--------------------------------------------------------------------------------------------------
/// A view in the generic view collection can be stepped to any case in the project, so all grid
/// calculations must be computed for that case, even when it is not an output case.
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, EnsureCalculationsAreComputedForCaseNotInOutputCases )
{
    std::unique_ptr<RimEclipseResultCase> destinationCase( openBruggeRealizationForCalculation( "Real0", "BRUGGE_0000.EGRID" ) );
    std::unique_ptr<RimEclipseResultCase> otherCase( openBruggeRealizationForCalculation( "Real10", "BRUGGE_0010.EGRID" ) );
    ASSERT_TRUE( destinationCase != nullptr );
    ASSERT_TRUE( otherCase != nullptr );

    RimProject* project = RimProject::current();
    ASSERT_TRUE( project != nullptr );

    auto* calculation = dynamic_cast<RimGridCalculation*>( project->gridCalculationCollection()->addCalculation( false ) );
    ASSERT_TRUE( calculation != nullptr );
    calculation->setExpression( "MyCalc := x + 1" );
    auto* variable = dynamic_cast<RimGridCalculationVariable*>( calculation->addVariable( "x" ) );
    ASSERT_TRUE( variable != nullptr );

    RimEclipseResultAddress sourceAddress;
    sourceAddress.setEclipseCase( destinationCase.get() );
    sourceAddress.setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    sourceAddress.setResultName( "PORO" );
    variable->setEclipseResultAddress( sourceAddress );

    auto* destinationCaseField = dynamic_cast<caf::PdmPtrField<RimEclipseCase*>*>( calculation->findField( "DestinationCase" ) );
    ASSERT_TRUE( destinationCaseField != nullptr );
    destinationCaseField->setValue( destinationCase.get() );

    const auto outputCases = calculation->outputEclipseCases();
    ASSERT_TRUE( std::find( outputCases.begin(), outputCases.end(), otherCase.get() ) == outputCases.end() );

    const RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, "MyCalc" );
    auto*                         otherResults = otherCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_FALSE( otherResults->hasResultEntry( resAddr ) );

    project->gridCalculationCollection()->ensureCalculationsAreComputed( otherCase.get() );

    EXPECT_TRUE( otherResults->hasResultEntry( resAddr ) );
    EXPECT_FALSE( otherResults->resultNames( RiaDefines::ResultCatType::GENERATED ).empty() );

    project->gridCalculationCollection()->deleteCalculation( calculation );
}
