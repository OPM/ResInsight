/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimGridCalculation.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGridCalculationVariable.h"
#include "RimReloadCaseTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "expressionparser/ExpressionParser.h"

CAF_PDM_SOURCE_INIT( RimGridCalculation, "RimGridCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation::RimGridCalculation()
{
    CAF_PDM_InitObject( "RimGridCalculation", ":/octave.png", "Calculation", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable* RimGridCalculation::createVariable() const
{
    return new RimGridCalculationVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::calculate()
{
    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    RimEclipseCase* eclipseCase = findEclipseCaseFromVariables();
    if ( !eclipseCase )
    {
        RiaLogging::errorInMessageBox( nullptr,
                                       "Grid Property Calculator",
                                       QString( "No case found for calculation : %1" ).arg( leftHandSideVariableName ) );
        return false;
    }

    auto [isOk, errorMessage] = validateVariables();
    if ( !isOk )
    {
        RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        return false;
    }

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
    {
        eclipseCase->results( porosityModel )->createResultEntry( resAddr, true );
    }

    eclipseCase->results( porosityModel )->clearScalarResult( resAddr );

    const size_t timeStepCount = eclipseCase->results( porosityModel )->maxTimeStepCount();

    std::vector<std::vector<double>>* scalarResultFrames =
        eclipseCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
    scalarResultFrames->resize( timeStepCount );

    for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
    {
        std::vector<std::vector<double>> values;
        for ( size_t i = 0; i < m_variables.size(); i++ )
        {
            RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
            CAF_ASSERT( v != nullptr );
            values.push_back( getInputVectorForVariable( v, tsId, porosityModel ) );
        }

        ExpressionParser parser;
        for ( size_t i = 0; i < m_variables.size(); i++ )
        {
            RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
            CAF_ASSERT( v != nullptr );
            parser.assignVector( v->name(), values[i] );
        }

        std::vector<double> resultValues;
        resultValues.resize( values[0].size() );
        parser.assignVector( leftHandSideVariableName, resultValues );

        QString errorText;
        bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

        if ( evaluatedOk )
        {
            auto [cellFilterView, defaultValueConfig] = findFilterValuesFromVariables();

            if ( cellFilterView )
            {
                auto [defaultValueType, defaultValue] = defaultValueConfig;
                filterResults( cellFilterView, values, defaultValueType, defaultValue, resultValues );
            }

            scalarResultFrames->at( tsId ) = resultValues;

            m_isDirty = false;
        }
        else
        {
            QString s = "The following error message was received from the parser library : \n\n";
            s += errorText;

            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", s );
            return false;
        }
    }

    eclipseCase->updateResultAddressCollection();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridCalculation::findEclipseCaseFromVariables() const
{
    for ( auto variable : m_variables )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        CAF_ASSERT( v != nullptr );

        if ( v->eclipseCase() ) return v->eclipseCase();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimGridView*, RimGridCalculationVariable::DefaultValueConfig>
    RimGridCalculation::findFilterValuesFromVariables() const
{
    for ( auto variable : m_variables )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        CAF_ASSERT( v != nullptr );

        if ( v->cellFilterView() ) return std::make_pair( v->cellFilterView(), v->defaultValueConfiguration() );
    }

    return std::pair( nullptr, std::make_pair( RimGridCalculationVariable::DefaultValueType::POSITIVE_INFINITY, HUGE_VAL ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCalculation::findFilterVariableIndex() const
{
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        auto                        variable = m_variables[i];
        RimGridCalculationVariable* v        = dynamic_cast<RimGridCalculationVariable*>( variable );
        CAF_ASSERT( v != nullptr );

        if ( v->cellFilterView() ) return static_cast<int>( i );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridCalculation::getInputVectorForVariable( RimGridCalculationVariable*   v,
                                                                   size_t                        tsId,
                                                                   RiaDefines::PorosityModelType porosityModel ) const
{
    int timeStep = v->timeStep();

    auto resultCategoryType = v->resultCategoryType();

    // General case is to use the data from the given time step
    size_t timeStepToUse = tsId;

    if ( resultCategoryType == RiaDefines::ResultCatType::STATIC_NATIVE )
    {
        // Use the first time step for static data for all time steps
        timeStepToUse = 0;
    }
    else if ( timeStep != RimGridCalculationVariable::allTimeStepsValue() )
    {
        // Use data from a specific time step for this variable for all result time steps
        timeStepToUse = timeStep;
    }

    RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );

    auto   mainGrid     = v->eclipseCase()->mainGrid();
    size_t maxGridCount = mainGrid->gridCount();

    size_t              cellCount = mainGrid->globalCellArray().size();
    std::vector<double> inputValues( cellCount );
    for ( size_t gridIdx = 0; gridIdx < maxGridCount; ++gridIdx )
    {
        auto grid = mainGrid->gridByIndex( gridIdx );

        cvf::ref<RigResultAccessor> sourceResultAccessor =
            RigResultAccessorFactory::createFromResultAddress( v->eclipseCase()->eclipseCaseData(),
                                                               gridIdx,
                                                               porosityModel,
                                                               timeStepToUse,
                                                               resAddr );

#pragma omp parallel for
        for ( int localGridCellIdx = 0; localGridCellIdx < static_cast<int>( grid->cellCount() ); localGridCellIdx++ )
        {
            const size_t reservoirCellIndex = grid->reservoirCellIndex( localGridCellIdx );
            inputValues[reservoirCellIndex] = sourceResultAccessor->cellScalar( localGridCellIdx );
        }
    }

    return inputValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithVector( const std::vector<double>& inputValues,
                                                          cvf::ref<cvf::UByteArray>  visibility,
                                                          std::vector<double>&       resultValues )
{
#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( resultValues.size() ); i++ )
    {
        if ( !visibility->val( i ) )
        {
            resultValues[i] = inputValues[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithDefaultValue( double                    defaultValue,
                                                                cvf::ref<cvf::UByteArray> visibility,
                                                                std::vector<double>&      resultValues )
{
#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( resultValues.size() ); i++ )
    {
        if ( !visibility->val( i ) )
        {
            resultValues[i] = defaultValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::filterResults( RimGridView*                                 cellFilterView,
                                        const std::vector<std::vector<double>>&      values,
                                        RimGridCalculationVariable::DefaultValueType defaultValueType,
                                        double                                       defaultValue,
                                        std::vector<double>&                         resultValues ) const
{
    auto visibility = cellFilterView->currentTotalCellVisibility();

    if ( defaultValueType == RimGridCalculationVariable::DefaultValueType::FROM_PROPERTY )
    {
        int filterVariableIndex = findFilterVariableIndex();
        replaceFilteredValuesWithVector( values[filterVariableIndex], visibility, resultValues );
    }
    else
    {
        replaceFilteredValuesWithDefaultValue( defaultValue, visibility, resultValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::updateDependentObjects()
{
    RimEclipseCase* eclipseCase = findEclipseCaseFromVariables();
    if ( eclipseCase )
    {
        RimReloadCaseTools::updateAll3dViews( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::removeDependentObjects()
{
    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    RimEclipseCase* eclipseCase = findEclipseCaseFromVariables();
    if ( eclipseCase )
    {
        // Select "None" result if the result that is being removed were displayed in a view.
        for ( auto v : eclipseCase->reservoirViews() )
        {
            if ( v->cellResult()->resultType() == resAddr.resultCatType() &&
                 v->cellResult()->resultVariable() == resAddr.resultName() )
            {
                v->cellResult()->setResultType( RiaDefines::ResultCatType::GENERATED );
                v->cellResult()->setResultVariable( "None" );
            }
        }

        eclipseCase->results( porosityModel )->clearScalarResult( resAddr );
        eclipseCase->results( porosityModel )->setRemovedTagOnGeneratedResult( resAddr );

        RimReloadCaseTools::updateAll3dViews( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RimGridCalculation::validateVariables()
{
    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->eclipseCase() )
        {
            QString errorMessage = QString( "No case defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        if ( v->resultVariable().isEmpty() )
        {
            QString errorMessage = QString( "No result variable defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        auto                    resultCategoryType = v->resultCategoryType();
        RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );
        if ( !v->eclipseCase()->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            QString errorMessage = QString( "Unable to load result for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }
    }

    return std::make_pair( true, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                                            std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_variables )
    {
        // Update the editors of all the variables if a variable changes.
        // This makes the read-only state of the filter parameters consistent:
        // only one filter is allowed at a time.
        for ( auto v : m_variables )
        {
            v->updateConnectedEditors();
        }
    }
}
