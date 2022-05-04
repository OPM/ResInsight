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
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimReloadCaseTools.h"

#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"

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

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RimEclipseCase* eclipseCase = findEclipseCaseFromVariables();
    if ( !eclipseCase )
    {
        RiaLogging::errorInMessageBox( nullptr,
                                       "Expression Parser",
                                       QString( "No case found for calculation : %1" ).arg( leftHandSideVariableName ) );
        return false;
    }

    const size_t timeStepCount = eclipseCase->results( porosityModel )->maxTimeStepCount();

    std::vector<std::vector<std::vector<double>>> values;
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

        if ( !v->eclipseCase() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No case defined for variable : %1" ).arg( v->name() ) );
            return false;
        }

        if ( v->resultVariable().isEmpty() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No result variable defined for variable : %1" ).arg( v->name() ) );
            return false;
        }

        auto                    resultCategoryType = v->resultCategoryType();
        RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );
        if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "Unable to load result for variable : %1" ).arg( v->name() ) );
            return false;
        }

        int timeStep = v->timeStep();

        std::vector<std::vector<double>> inputValues = eclipseCase->results( porosityModel )->cellScalarResults( resAddr );
        if ( resultCategoryType == RiaDefines::ResultCatType::STATIC_NATIVE )
        {
            // Use static data for all time steps
            inputValues.resize( timeStepCount );
            for ( size_t tsId = 1; tsId < timeStepCount; tsId++ )
            {
                inputValues[tsId] = inputValues[0];
            }
        }
        else if ( timeStep != RimGridCalculationVariable::allTimeStepsValue() )
        {
            // Use data from a specific time step for this variable for all result time steps
            for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
            {
                if ( static_cast<int>( tsId ) != timeStep )
                {
                    inputValues[tsId] = inputValues[timeStep];
                }
            }
        }

        values.push_back( inputValues );
    }

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
    {
        eclipseCase->results( porosityModel )->createResultEntry( resAddr, true );
    }

    eclipseCase->results( porosityModel )->clearScalarResult( resAddr );

    std::vector<std::vector<double>>* scalarResultFrames =
        eclipseCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
    scalarResultFrames->resize( timeStepCount );

    for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
    {
        ExpressionParser parser;
        for ( size_t i = 0; i < m_variables.size(); i++ )
        {
            RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
            parser.assignVector( v->name(), values[i][tsId] );
        }

        std::vector<double> resultValues;
        resultValues.resize( values[0][tsId].size() );
        parser.assignVector( leftHandSideVariableName, resultValues );

        QString errorText;
        bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

        if ( evaluatedOk )
        {
            scalarResultFrames->at( tsId ) = resultValues;

            m_isDirty = false;
        }
        else
        {
            QString s = "The following error message was received from the parser library : \n\n";
            s += errorText;

            RiaLogging::errorInMessageBox( nullptr, "Expression Parser", s );
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridCalculation::findEclipseCaseFromVariables()
{
    for ( auto variable : m_variables )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        if ( v->eclipseCase() ) return v->eclipseCase();
    }

    return nullptr;
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
        // Select default result if
        for ( auto v : eclipseCase->reservoirViews() )
        {
            if ( v->cellResult()->resultType() == resAddr.resultCatType() &&
                 v->cellResult()->resultVariable() == resAddr.resultName() )
            {
                v->cellResult()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
                v->cellResult()->setResultVariable( "SOIL" );
            }
        }

        eclipseCase->results( porosityModel )->clearScalarResult( resAddr );
        eclipseCase->results( porosityModel )->eraseGeneratedResult( resAddr );

        RimReloadCaseTools::updateAll3dViews( eclipseCase );
    }
}
