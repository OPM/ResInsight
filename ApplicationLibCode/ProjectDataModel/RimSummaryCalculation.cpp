/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryCalculation.h"

#include "expressionparser/ExpressionParser.h"

#include "RiaCurveMerger.h"
#include "RiaLogging.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimSummaryCalculation, "RimSummaryCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation::RimSummaryCalculation()
{
    CAF_PDM_InitObject( "RimSummaryCalculation", ":/octave.png", "Calculation", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable* RimSummaryCalculation::createVariable()
{
    return new RimSummaryCalculationVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::calculate()
{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( m_expression );

    RiaTimeHistoryCurveMerger timeHistoryCurveMerger;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->summaryCase() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary case defined for variable : %1" ).arg( v->name() ) );

            return false;
        }

        if ( !v->summaryAddress() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary address defined for variable : %1" ).arg( v->name() ) );

            return false;
        }

        RiaSummaryCurveDefinition curveDef( v->summaryCase(), v->summaryAddress()->address(), false );

        std::vector<double> curveValues;
        RiaSummaryCurveDefinition::resultValues( curveDef, &curveValues );

        std::vector<time_t> curveTimeSteps = RiaSummaryCurveDefinition::timeSteps( curveDef );

        if ( !curveTimeSteps.empty() && !curveValues.empty() )
        {
            timeHistoryCurveMerger.addCurveData( curveTimeSteps, curveValues );
        }
    }

    timeHistoryCurveMerger.computeInterpolatedValues();

    ExpressionParser parser;
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        parser.assignVector( v->name(), timeHistoryCurveMerger.interpolatedYValuesForAllXValues( i ) );
    }

    std::vector<double> resultValues;
    resultValues.resize( timeHistoryCurveMerger.allXValues().size() );

    parser.assignVector( leftHandSideVariableName, resultValues );

    QString errorText;
    bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

    if ( evaluatedOk )
    {
        m_timesteps.v().clear();
        m_calculatedValues.v().clear();

        if ( timeHistoryCurveMerger.validIntervalsForAllXValues().size() > 0 )
        {
            size_t firstValidTimeStep = timeHistoryCurveMerger.validIntervalsForAllXValues().front().first;
            size_t lastValidTimeStep  = timeHistoryCurveMerger.validIntervalsForAllXValues().back().second + 1;

            if ( lastValidTimeStep > firstValidTimeStep && lastValidTimeStep <= timeHistoryCurveMerger.allXValues().size() )
            {
                std::vector<time_t> validTimeSteps( timeHistoryCurveMerger.allXValues().begin() + firstValidTimeStep,
                                                    timeHistoryCurveMerger.allXValues().begin() + lastValidTimeStep );

                std::vector<double> validValues( resultValues.begin() + firstValidTimeStep,
                                                 resultValues.begin() + lastValidTimeStep );

                m_timesteps        = validTimeSteps;
                m_calculatedValues = validValues;
            }
        }

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
void RimSummaryCalculation::updateDependentObjects()
{
    RimSummaryCalculationCollection* calcColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( calcColl );
    calcColl->rebuildCaseMetaData();

    RimSummaryMultiPlotCollection* summaryPlotCollection = RiaSummaryTools::summaryMultiPlotCollection();
    for ( auto multiPlot : summaryPlotCollection->multiPlots() )
    {
        for ( RimSummaryPlot* sumPlot : multiPlot->summaryPlots() )
        {
            bool plotContainsCalculatedCurves = false;

            for ( RimSummaryCurve* sumCurve : sumPlot->summaryCurves() )
            {
                if ( sumCurve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
                {
                    sumCurve->updateConnectedEditors();

                    plotContainsCalculatedCurves = true;
                }
            }

            if ( plotContainsCalculatedCurves )
            {
                sumPlot->loadDataAndUpdate();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::removeDependentObjects()
{
}
