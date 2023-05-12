/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimSummaryRegressionAnalysisCurve.h"

#include "cafPdmUiLineEditor.h"

#include "LinearRegression.hpp"
#include "PolynominalRegression.hpp"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSummaryRegressionAnalysisCurve, "RegressionAnalysisCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::RegressionType>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LINEAR, "LINEAR", "Linear" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POLYNOMINAL, "POLYNOMINAL", "Polynominal" );
    setDefault( RimSummaryRegressionAnalysisCurve::RegressionType::LINEAR );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve::RimSummaryRegressionAnalysisCurve()
{
    CAF_PDM_InitObject( "Regression Analysis Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_regressionType, "RegressionType", "Type" );
    CAF_PDM_InitField( &m_polynominalDegree, "PolynominalDegree", 3, "Degree" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve::~RimSummaryRegressionAnalysisCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryRegressionAnalysisCurve::valuesY() const
{
    return computeRegressionCurve( RimSummaryCurve::timeStepsY(), RimSummaryCurve::valuesY() ).second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryRegressionAnalysisCurve::valuesX() const
{
    return computeRegressionCurve( RimSummaryCurve::timeStepsX(), RimSummaryCurve::valuesX() ).second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::timeStepsY() const
{
    return computeRegressionCurve( RimSummaryCurve::timeStepsY(), RimSummaryCurve::valuesY() ).first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::timeStepsX() const
{
    return computeRegressionCurve( RimSummaryCurve::timeStepsX(), RimSummaryCurve::valuesX() ).first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>>
    RimSummaryRegressionAnalysisCurve::computeRegressionCurve( const std::vector<time_t>& timeSteps, const std::vector<double>& values ) const
{
    if ( values.empty() || timeSteps.empty() ) return { timeSteps, values };

    auto convertToDouble = []( const std::vector<time_t>& timeSteps )
    {
        std::vector<double> doubleVector( timeSteps.size() );
        std::transform( timeSteps.begin(),
                        timeSteps.end(),
                        doubleVector.begin(),
                        []( const auto& timeVal ) { return static_cast<double>( timeVal ); } );
        return doubleVector;
    };

    std::vector<double> timeStepsD = convertToDouble( timeSteps );

    if ( m_regressionType == RegressionType::LINEAR )
    {
        regression::LinearRegression linearRegression;
        linearRegression.fit( timeStepsD, values );
        std::vector<double> predictedValues = linearRegression.predict( timeStepsD );
        return { timeSteps, predictedValues };
    }
    else if ( m_regressionType == RegressionType::POLYNOMINAL )
    {
        regression::PolynominalRegression polynominalRegression;
        polynominalRegression.fit( timeStepsD, values, m_polynominalDegree );
        std::vector<double> predictedValues = polynominalRegression.predict( timeStepsD );
        return { timeSteps, predictedValues };
    }

    return { timeSteps, values };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* regressionCurveGroup = uiOrdering.addNewGroup( "Regression Analysis" );
    regressionCurveGroup->add( &m_regressionType );

    if ( m_regressionType == RegressionType::POLYNOMINAL )
    {
        regressionCurveGroup->add( &m_polynominalDegree );
    }

    RimSummaryCurve::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    RimSummaryCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_regressionType || changedField == &m_polynominalDegree )
    {
        loadAndUpdateDataAndPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                               QString                    uiConfigName,
                                                               caf::PdmUiEditorAttribute* attribute )
{
    RimSummaryCurve::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_polynominalDegree )
    {
        if ( auto* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            // Polynominal degree should be a positive number.
            lineEditorAttr->validator = new QIntValidator( 1, 50, nullptr );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::createCurveAutoName()
{
    return RimSummaryCurve::createCurveAutoName() + " " + m_regressionType().uiText() + " Regression";
}
