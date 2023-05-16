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
#include "cafPdmUiTextEditor.h"

#include "LinearRegression.hpp"
#include "PolynominalRegression.hpp"
#include "PowerFitRegression.hpp"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSummaryRegressionAnalysisCurve, "RegressionAnalysisCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::RegressionType>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LINEAR, "LINEAR", "Linear" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POLYNOMINAL, "POLYNOMINAL", "Polynominal" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POWER_FIT, "POWER_FIT", "Power Fit" );
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

    CAF_PDM_InitFieldNoDefault( &m_expressionText, "ExpressionText", "Expression" );
    m_expressionText.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_expressionText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_expressionText.uiCapability()->setUiReadOnly( true );
    m_expressionText.xmlCapability()->disableIO();
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
void RimSummaryRegressionAnalysisCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    QString descriptionX;
    std::tie( m_timeStepsX, m_valuesX, descriptionX ) = computeRegressionCurve( RimSummaryCurve::timeStepsX(), RimSummaryCurve::valuesX() );

    QString descriptionY;
    std::tie( m_timeStepsY, m_valuesY, descriptionY ) = computeRegressionCurve( RimSummaryCurve::timeStepsY(), RimSummaryCurve::valuesY() );

    m_expressionText = descriptionY;

    RimSummaryCurve::onLoadDataAndUpdate( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryRegressionAnalysisCurve::valuesY() const
{
    return m_valuesY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryRegressionAnalysisCurve::valuesX() const
{
    return m_valuesX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::timeStepsY() const
{
    return m_timeStepsY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::timeStepsX() const
{
    return m_timeStepsX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<std::vector<time_t>, std::vector<double>, QString>
    RimSummaryRegressionAnalysisCurve::computeRegressionCurve( const std::vector<time_t>& timeSteps, const std::vector<double>& values ) const
{
    if ( values.empty() || timeSteps.empty() ) return { timeSteps, values, "" };

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
        auto generateRegressionText = []( const regression::LinearRegression& reg ) {
            return QString( "Linear Regression<br>Intercept: %1<br>Slope: %2<br><br>r = %2x + %1" ).arg( reg.intercept() ).arg( reg.slope() );
        };

        regression::LinearRegression linearRegression;
        linearRegression.fit( timeStepsD, values );
        std::vector<double> predictedValues = linearRegression.predict( timeStepsD );
        return { timeSteps, predictedValues, generateRegressionText( linearRegression ) };
    }
    else if ( m_regressionType == RegressionType::POLYNOMINAL )
    {
        auto generateRegressionText = []( const regression::PolynominalRegression& reg )
        {
            QString str = "Polynominal Regression<br><br>r = ";

            std::vector<double> coeffs = reg.coeffisients();
            QStringList         parts;
            for ( size_t i = 0; i < coeffs.size(); i++ )
            {
                double coeff = coeffs[i];
                // Skip zero coeffs
                if ( coeff != 0.0 )
                {
                    if ( i == 0 )
                    {
                        parts.append( QString( " %1 " ).arg( coeff ) );
                    }
                    else
                    {
                        parts.append( QString( " %1x<sup>%2</sup>" ).arg( coeff ).arg( i ) );
                    }
                }
            }

            return str + parts.join( " + " );
        };

        regression::PolynominalRegression polynominalRegression;
        polynominalRegression.fit( timeStepsD, values, m_polynominalDegree );
        std::vector<double> predictedValues = polynominalRegression.predict( timeStepsD );
        return { timeSteps, predictedValues, generateRegressionText( polynominalRegression ) };
    }
    else if ( m_regressionType == RegressionType::POWER_FIT )
    {
        auto generateRegressionText = []( const regression::PowerFitRegression& reg )
        { return QString( "Power Fit Regression<br><br>r = %1 + x<sup>%2</sup>" ).arg( reg.scale() ).arg( reg.exponent() ); };

        regression::PowerFitRegression powerFitRegression;
        powerFitRegression.fit( timeStepsD, values );
        std::vector<double> predictedValues = powerFitRegression.predict( timeStepsD );
        return { timeSteps, predictedValues, generateRegressionText( powerFitRegression ) };
    }

    return { timeSteps, values, "" };
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

    regressionCurveGroup->add( &m_expressionText );

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
    else if ( field == &m_expressionText )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::curveExportDescription( const RifEclipseSummaryAddress& address ) const
{
    return RimSummaryCurve::curveExportDescription() + "." + m_regressionType().uiText() + "_Regression";
}
