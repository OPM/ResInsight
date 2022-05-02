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

#include "RimEclipseCase.h"

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

    RimEclipseCase*                  eclipseCase = nullptr;
    std::vector<std::vector<double>> values;
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

        // Use the first defined eclipse case from for output
        if ( !eclipseCase ) eclipseCase = v->eclipseCase();

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

        RigEclipseResultAddress resAddr( v->resultCategoryType(), v->resultVariable() );
        if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "Unable to load result for variable : %1" ).arg( v->name() ) );
            return false;
        }

        std::vector<std::vector<double>> inputValues = eclipseCase->results( porosityModel )->cellScalarResults( resAddr );

        values.push_back( inputValues[0] );
    }

    ExpressionParser parser;
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

        parser.assignVector( v->name(), values[i] );
    }

    std::vector<double> resultValues;
    resultValues.resize( values[0].size() );
    parser.assignVector( leftHandSideVariableName, resultValues );

    QString errorText;
    bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

    if ( evaluatedOk )
    {
        m_timesteps.v().clear();
        m_calculatedValues.v().clear();

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

        if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            eclipseCase->results( porosityModel )->createResultEntry( resAddr, true );
        }
        std::vector<std::vector<double>>* scalarResultFrames =
            eclipseCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
        size_t timeStepCount = eclipseCase->results( porosityModel )->maxTimeStepCount();
        scalarResultFrames->resize( timeStepCount );

        size_t tsId                    = 0;
        scalarResultFrames->at( tsId ) = resultValues;

        m_isDirty = false;
    }
    else
    {
        QString s = "The following error message was received from the parser library : \n\n";
        s += errorText;

        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", s );
    }

    return evaluatedOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::updateDependentObjects()
{
    // RimGridCalculationCollection* calcColl = nullptr;
    // this->firstAncestorOrThisOfTypeAsserted( calcColl );
    // calcColl->rebuildCaseMetaData();

    // RimGridMultiPlotCollection* summaryPlotCollection = RiaGridTools::summaryMultiPlotCollection();
    // for ( auto multiPlot : summaryPlotCollection->multiPlots() )
    // {
    //     for ( RimGridPlot* sumPlot : multiPlot->summaryPlots() )
    //     {
    //         bool plotContainsCalculatedCurves = false;

    //         for ( RimGridCurve* sumCurve : sumPlot->summaryCurves() )
    //         {
    //             if ( sumCurve->summaryAddressY().category() == RifEclipseGridAddress::SUMMARY_CALCULATED )
    //             {
    //                 sumCurve->updateConnectedEditors();

    //                 plotContainsCalculatedCurves = true;
    //             }
    //         }

    //         if ( plotContainsCalculatedCurves )
    //         {
    //             sumPlot->loadDataAndUpdate();
    //         }
    //     }
    // }
}
