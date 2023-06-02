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

#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

#include "ExponentialRegression.hpp"
#include "LinearRegression.hpp"
#include "LogarithmicRegression.hpp"
#include "LogisticRegression.hpp"
#include "PolynominalRegression.hpp"
#include "PowerFitRegression.hpp"

#include <QDateTime>

#include <cmath>
#include <vector>

CAF_PDM_SOURCE_INIT( RimSummaryRegressionAnalysisCurve, "RegressionAnalysisCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::RegressionType>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LINEAR, "LINEAR", "Linear" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POLYNOMINAL, "POLYNOMINAL", "Polynominal" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POWER_FIT, "POWER_FIT", "Power Fit" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::EXPONENTIAL, "EXPONENTIAL", "Exponential" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LOGARITHMIC, "LOGARITHMIC", "Logarithmic" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LOGISTIC, "LOGISTIC", "Logistic" );
    setDefault( RimSummaryRegressionAnalysisCurve::RegressionType::LINEAR );
}

template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::ForecastUnit>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::DAYS, "DAYS", "Days" );
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::MONTHS, "MONTHS", "Months" );
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::YEARS, "YEARS", "Years" );
    setDefault( RimSummaryRegressionAnalysisCurve::ForecastUnit::YEARS );
}

}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve::RimSummaryRegressionAnalysisCurve()
{
    CAF_PDM_InitObject( "Regression Analysis Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_regressionType, "RegressionType", "Type" );
    CAF_PDM_InitField( &m_forecastForward, "ForecastForward", 0, "Forward" );
    CAF_PDM_InitField( &m_forecastBackward, "ForecastBackward", 0, "Backward" );
    CAF_PDM_InitFieldNoDefault( &m_forecastUnit, "ForecastUnit", "Unit" );
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

    std::vector<double> timeStepsD = convertToDouble( timeSteps );

    std::vector<time_t> outputTimeSteps = getOutputTimeSteps( timeSteps, m_forecastBackward(), m_forecastForward(), m_forecastUnit() );

    std::vector<double> outputTimeStepsD = convertToDouble( outputTimeSteps );

    if ( m_regressionType == RegressionType::LINEAR )
    {
        regression::LinearRegression linearRegression;
        linearRegression.fit( timeStepsD, values );
        std::vector<double> predictedValues = linearRegression.predict( outputTimeStepsD );
        return { outputTimeSteps, predictedValues, generateRegressionText( linearRegression ) };
    }
    else if ( m_regressionType == RegressionType::POLYNOMINAL )
    {
        regression::PolynominalRegression polynominalRegression;
        polynominalRegression.fit( timeStepsD, values, m_polynominalDegree );
        std::vector<double> predictedValues = polynominalRegression.predict( outputTimeStepsD );
        return { outputTimeSteps, predictedValues, generateRegressionText( polynominalRegression ) };
    }
    else if ( m_regressionType == RegressionType::POWER_FIT )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsD, values );
        regression::PowerFitRegression powerFitRegression;
        powerFitRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = powerFitRegression.predict( outputTimeStepsD );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( powerFitRegression ) };
    }
    else if ( m_regressionType == RegressionType::EXPONENTIAL )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsD, values );
        regression::ExponentialRegression exponentialRegression;
        exponentialRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = exponentialRegression.predict( outputTimeStepsD );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( exponentialRegression ) };
    }
    else if ( m_regressionType == RegressionType::LOGARITHMIC )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsD, values );
        regression::LogarithmicRegression logarithmicRegression;
        logarithmicRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = logarithmicRegression.predict( outputTimeStepsD );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( logarithmicRegression ) };
    }
    else if ( m_regressionType == RegressionType::LOGISTIC )
    {
        regression::LogisticRegression logisticRegression;
        logisticRegression.fit( timeStepsD, values );
        std::vector<double> predictedValues = logisticRegression.predict( outputTimeStepsD );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( logisticRegression ) };
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

    caf::PdmUiGroup* forecastingGroup = uiOrdering.addNewGroup( "Forecasting" );
    forecastingGroup->add( &m_forecastForward );
    forecastingGroup->add( &m_forecastBackward );
    forecastingGroup->add( &m_forecastUnit );

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
    if ( changedField == &m_regressionType || changedField == &m_polynominalDegree || changedField == &m_forecastBackward ||
         changedField == &m_forecastForward || changedField == &m_forecastUnit )
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
    else if ( field == &m_forecastForward || field == &m_forecastBackward )
    {
        if ( auto* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            // Block negative forecast
            lineEditorAttr->validator = new QIntValidator( 0, 50, nullptr );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::formatDouble( double v )
{
    return QString::number( v, 'g', 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::LinearRegression& reg )
{
    QString sign = reg.intercept() < 0.0 ? "-" : "+";
    return QString( "y = %1x %2 %3" ).arg( formatDouble( reg.slope() ) ).arg( sign ).arg( formatDouble( std::fabs( reg.intercept() ) ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::PolynominalRegression& reg )
{
    QString str = "y = ";

    bool                isFirst = true;
    std::vector<double> coeffs  = reg.coeffisients();
    QStringList         parts;
    for ( size_t i = 0; i < coeffs.size(); i++ )
    {
        double coeff = coeffs[i];
        // Skip zero coeffs
        if ( coeff != 0.0 )
        {
            if ( coeff < 0.0 )
                parts.append( "-" );
            else if ( !isFirst )
                parts.append( "+" );

            if ( i == 0 )
            {
                parts.append( QString( "%1" ).arg( formatDouble( std::fabs( coeff ) ) ) );
            }
            else
            {
                parts.append( QString( " %1x<sup>%2</sup>" ).arg( formatDouble( std::fabs( coeff ) ) ).arg( i ) );
            }

            isFirst = false;
        }
    }

    return str + parts.join( " " ) + QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::PowerFitRegression& reg )
{
    return QString( "y = %1 + x<sup>%2</sup>" ).arg( formatDouble( reg.scale() ) ).arg( formatDouble( reg.exponent() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::ExponentialRegression& reg )
{
    return QString( "y = %1 * e<sup>%2x</sup>" ).arg( formatDouble( reg.a() ) ).arg( formatDouble( reg.b() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::LogarithmicRegression& reg )
{
    return QString( "y = %1 + %2 * ln(x)" ).arg( formatDouble( reg.a() ) ).arg( formatDouble( reg.b() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::LogisticRegression& reg )
{
    // TODO: Display more parameters here.
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::appendTimeSteps( std::vector<time_t>& destinationTimeSteps, const std::set<QDateTime>& sourceTimeSteps )
{
    for ( const QDateTime& t : sourceTimeSteps )
        destinationTimeSteps.push_back( RiaTimeTTools::fromQDateTime( t ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::getOutputTimeSteps( const std::vector<time_t>& timeSteps,
                                                                           int                        forecastBackward,
                                                                           int                        forecastForward,
                                                                           ForecastUnit               forecastUnit )
{
    auto getTimeSpan = []( int value, ForecastUnit unit )
    {
        if ( unit == ForecastUnit::YEARS ) return DateTimeSpan( value, 0, 0 );
        if ( unit == ForecastUnit::MONTHS ) return DateTimeSpan( 0, value, 0 );
        CAF_ASSERT( unit == ForecastUnit::DAYS );
        return DateTimeSpan( 0, 0, value );
    };

    int numDates = 50;

    std::vector<time_t> outputTimeSteps;
    if ( forecastBackward > 0 )
    {
        QDateTime firstTimeStepInData = RiaQDateTimeTools::fromTime_t( timeSteps.front() );
        QDateTime forecastStartTimeStep = RiaQDateTimeTools::subtractSpan( firstTimeStepInData, getTimeSpan( forecastBackward, forecastUnit ) );
        auto forecastTimeSteps =
            RiaQDateTimeTools::createEvenlyDistributedDatesInInterval( forecastStartTimeStep, firstTimeStepInData, numDates );
        appendTimeSteps( outputTimeSteps, forecastTimeSteps );
    }

    outputTimeSteps.insert( std::end( outputTimeSteps ), std::begin( timeSteps ), std::end( timeSteps ) );

    if ( forecastForward > 0 )
    {
        QDateTime lastTimeStepInData  = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
        QDateTime forecastEndTimeStep = RiaQDateTimeTools::addSpan( lastTimeStepInData, getTimeSpan( forecastForward, forecastUnit ) );
        auto forecastTimeSteps = RiaQDateTimeTools::createEvenlyDistributedDatesInInterval( lastTimeStepInData, forecastEndTimeStep, numDates );
        appendTimeSteps( outputTimeSteps, forecastTimeSteps );
    }

    return outputTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryRegressionAnalysisCurve::convertToDouble( const std::vector<time_t>& timeSteps )
{
    std::vector<double> doubleVector( timeSteps.size() );
    std::transform( timeSteps.begin(),
                    timeSteps.end(),
                    doubleVector.begin(),
                    []( const auto& timeVal ) { return static_cast<double>( timeVal ); } );
    return doubleVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryRegressionAnalysisCurve::convertToTimeT( const std::vector<double>& timeSteps )
{
    std::vector<time_t> tVector( timeSteps.size() );
    std::transform( timeSteps.begin(), timeSteps.end(), tVector.begin(), []( const auto& timeVal ) { return static_cast<time_t>( timeVal ); } );
    return tVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>>
    RimSummaryRegressionAnalysisCurve::getPositiveValues( const std::vector<double>& timeSteps, const std::vector<double>& values )
{
    std::vector<double> filteredTimeSteps;
    std::vector<double> filteredValues;
    for ( size_t i = 0; i < timeSteps.size(); i++ )
    {
        if ( timeSteps[i] > 0.0 && values[i] > 0.0 )
        {
            filteredTimeSteps.push_back( timeSteps[i] );
            filteredValues.push_back( values[i] );
        }
    }

    return std::make_pair( filteredTimeSteps, filteredValues );
}
