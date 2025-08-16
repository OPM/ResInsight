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

#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "RiaRegressionTextTools.h"
#include "RiaStatisticsTools.h"
#include "RiaTimeTTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleStatistics.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"
#include "RimTimeAxisAnnotation.h"

#include "cafPdmUiDateEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiSliderTools.h"
#include "cafPdmUiTextEditor.h"

#include "ExponentialRegression.hpp"
#include "LinearRegression.hpp"
#include "LogarithmicRegression.hpp"
#include "LogisticRegression.hpp"
#include "PolynomialRegression.hpp"
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
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POLYNOMIAL, "POLYNOMIAL", "Polynomial" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::POWER_FIT, "POWER_FIT", "Power Fit" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::EXPONENTIAL, "EXPONENTIAL", "Exponential" );
    addItem( RimSummaryRegressionAnalysisCurve::RegressionType::LOGARITHMIC, "LOGARITHMIC", "Logarithmic" );
    setDefault( RimSummaryRegressionAnalysisCurve::RegressionType::POLYNOMIAL );
}

template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::ForecastUnit>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::DAYS, "DAYS", "Days" );
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::MONTHS, "MONTHS", "Months" );
    addItem( RimSummaryRegressionAnalysisCurve::ForecastUnit::YEARS, "YEARS", "Years" );
    setDefault( RimSummaryRegressionAnalysisCurve::ForecastUnit::YEARS );
}

template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::DataSource>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::DataSource::SUMMARY_ADDRESS, "SUMMARY_ADDRESS", "Summary Address" );
    addItem( RimSummaryRegressionAnalysisCurve::DataSource::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    setDefault( RimSummaryRegressionAnalysisCurve::DataSource::SUMMARY_ADDRESS );
}

template <>
void caf::AppEnum<RimSummaryRegressionAnalysisCurve::RangeType>::setUp()
{
    addItem( RimSummaryRegressionAnalysisCurve::RangeType::FULL_RANGE, "FULL_RANGE", "Full Range" );
    addItem( RimSummaryRegressionAnalysisCurve::RangeType::USER_DEFINED_RANGE, "USER_DEFINED_RANGE", "Custom Range" );
    setDefault( RimSummaryRegressionAnalysisCurve::RangeType::FULL_RANGE );
}

}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve::RimSummaryRegressionAnalysisCurve()
{
    CAF_PDM_InitObject( "Regression Analysis Curve", ":/regression-curve.svg" );

    CAF_PDM_InitFieldNoDefault( &m_dataSourceForRegression, "DataSourceForRegression", "Data Source" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "SourceCurveSet", "Source Curve Set" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleStatisticsType, "EnsembleStatisticsType", "Ensemble Statistics Type" );

    CAF_PDM_InitFieldNoDefault( &m_regressionType, "RegressionType", "Type" );
    CAF_PDM_InitField( &m_forecastForward, "ForecastForward", 0, "Forward" );
    CAF_PDM_InitField( &m_forecastBackward, "ForecastBackward", 0, "Backward" );
    CAF_PDM_InitFieldNoDefault( &m_forecastUnit, "ForecastUnit", "Unit" );
    CAF_PDM_InitField( &m_polynomialDegree, "PolynomialDegree", 3, "Degree" );

    CAF_PDM_InitFieldNoDefault( &m_timeRangeSelection, "TimeRangeSelection", "Time Range" );
    CAF_PDM_InitFieldNoDefault( &m_minTimeSliderPosition, "MinTimeSliderPosition", "From" );
    m_minTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxTimeSliderPosition, "MaxTimeSliderPosition", "To" );
    m_maxTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showTimeSelectionInPlot, "ShowTimeSelectionInPlot", false, "Show In Plot" );

    CAF_PDM_InitFieldNoDefault( &m_expressionText, "ExpressionText", "Expression" );
    m_expressionText.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_expressionText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_expressionText.uiCapability()->setUiReadOnly( true );
    m_expressionText.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_xRangeSelection, "XRangeSelection", "X Value Range" );
    CAF_PDM_InitField( &m_valueRangeX, "ValueRangeX", std::make_pair( 0.0, 0.0 ), "Value Range X" );
    m_valueRangeX.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_yRangeSelection, "YRangeSelection", "Y Value Range" );
    CAF_PDM_InitField( &m_valueRangeY, "ValueRangeY", std::make_pair( 0.0, 0.0 ), "Value Range Y" );
    m_valueRangeY.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::setEnsembleCurveSet( RimEnsembleCurveSet* ensembleCurveSet )
{
    m_dataSourceForRegression = DataSource::ENSEMBLE;
    m_ensembleCurveSet        = ensembleCurveSet;
    m_ensembleStatisticsType  = RifEclipseSummaryAddressDefines::StatisticsType::P10;

    setSummaryAddressY( {} );
    setSummaryAddressX( {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    extractSourceCurveData();
    updateDefaultValues();

    // Clear derived data
    m_valuesX.clear();
    m_valuesY.clear();
    m_timeStepsX.clear();
    m_timeStepsY.clear();
    m_expressionText = "Undefined";

    std::vector<double> xValues    = m_sourceValuesX;
    std::vector<double> yValues    = m_sourceValuesY;
    std::vector<time_t> timeStepsX = m_sourceTimeStepsX;
    std::vector<time_t> timeStepsY = m_sourceTimeStepsY;

    QStringList errorMessages;

    if ( yValues.empty() )
    {
        errorMessages += "No Y values found for regression curve.";
    }

    if ( axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
    {
        if ( xValues.size() != yValues.size() ) errorMessages += "X value count and Y value count differs.";
        if ( xValues.size() != timeStepsX.size() ) errorMessages += "X value count and X time step count differs.";
        if ( xValues.size() != timeStepsY.size() ) errorMessages += "X value count and Y time step count differs.";

        if ( timeStepsX != timeStepsY )
        {
            errorMessages +=
                "Differences in time steps for X and Y axis detected. This is currently not supported. Make sure that the same "
                "case is used for both axis.";
        }
    }

    if ( !errorMessages.isEmpty() )
    {
        // Call parent class to make sure the curve is removed from the plot
        RimSummaryCurve::onLoadDataAndUpdate( updateParentPlot );

        QString errMsg = errorMessages.join( "\n" );
        RiaLogging::error( errMsg );

        return;
    }

    std::vector<size_t> indicesToRemove;

    if ( axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
    {
        for ( size_t i = 0; i < xValues.size(); i++ )
        {
            if ( xValues[i] < m_valueRangeX().first || xValues[i] > m_valueRangeX().second || !RiaStatisticsTools::isValidNumber( xValues[i] ) )
            {
                indicesToRemove.push_back( i );
            }
        }

        for ( size_t i = 0; i < yValues.size(); i++ )
        {
            if ( yValues[i] < m_valueRangeY().first || yValues[i] > m_valueRangeY().second || !RiaStatisticsTools::isValidNumber( yValues[i] ) )
            {
                indicesToRemove.push_back( i );
            }
        }
    }

    // Sort indices in descending order
    std::sort( indicesToRemove.rbegin(), indicesToRemove.rend() );

    // There might be duplicates, remove them
    indicesToRemove.erase( std::unique( indicesToRemove.begin(), indicesToRemove.end() ), indicesToRemove.end() );

    // Remove elements at the specified indices
    for ( auto index : indicesToRemove )
    {
        if ( index < yValues.size() )
        {
            yValues.erase( yValues.begin() + index );
            timeStepsY.erase( timeStepsY.begin() + index );
        }

        if ( index < xValues.size() )
        {
            xValues.erase( xValues.begin() + index );
            timeStepsX.erase( timeStepsX.begin() + index );
        }
    }

    QString descriptionX;
    std::tie( m_timeStepsX, m_valuesX, descriptionX ) = computeRegressionCurve( timeStepsX, xValues );

    QString descriptionY;
    std::tie( m_timeStepsY, m_valuesY, descriptionY ) = computeRegressionCurve( timeStepsY, yValues );

    m_expressionText = descriptionY;

    RimSummaryCurve::onLoadDataAndUpdate( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::extractSourceCurveData()
{
    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<time_t> xTimeSteps;
    std::vector<time_t> yTimeSteps;

    if ( m_dataSourceForRegression() == DataSource::ENSEMBLE )
    {
        auto findStatisticsCurve = []( RimEnsembleCurveSet*                            curveSet,
                                       RifEclipseSummaryAddressDefines::StatisticsType statisticsType ) -> RimSummaryCurve*
        {
            if ( curveSet == nullptr ) return nullptr;

            for ( auto curve : curveSet->curves() )
            {
                auto yAddr = curve->summaryAddressY();
                if ( yAddr.statisticsType() == statisticsType ) return curve;
            }
            return nullptr;
        };

        auto curve = findStatisticsCurve( m_ensembleCurveSet(), m_ensembleStatisticsType() );
        if ( curve )
        {
            yValues = curve->valuesY();
            xValues = curve->valuesX();

            auto curveTimeY = curve->timeStepsY();
            if ( curveTimeY.size() == yValues.size() )
            {
                yTimeSteps = curveTimeY;
            }
            else
            {
                // Fallback to use time steps from summary case
                // The time steps are used for reference, not used when computing the regression curve
                auto summaryCase = m_ensembleCurveSet->summaryEnsemble()->allSummaryCases().back();
                if ( auto reader = summaryCase->summaryReader() )
                {
                    auto allTimeSteps = reader->timeSteps( {} );
                    yTimeSteps        = allTimeSteps;

                    yTimeSteps.resize( yValues.size() );
                }
            }

            if ( xValues.size() == yValues.size() ) xTimeSteps = yTimeSteps;
        }
    }
    else
    {
        // Get curve data from the summary data defined by X and Y axis data

        xValues    = RimSummaryCurve::valuesX();
        yValues    = RimSummaryCurve::valuesY();
        xTimeSteps = RimSummaryCurve::timeStepsX();
        yTimeSteps = RimSummaryCurve::timeStepsY();
    }

    m_sourceTimeStepsX = xTimeSteps;
    m_sourceTimeStepsY = yTimeSteps;
    m_sourceValuesX    = xValues;
    m_sourceValuesY    = yValues;
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

    auto [minTimeStep, maxTimeStep]        = selectedTimeStepRange();
    auto [timeStepsInRange, valuesInRange] = getInRangeValues( timeSteps, values, minTimeStep, maxTimeStep );

    if ( timeStepsInRange.empty() || valuesInRange.empty() ) return {};

    const std::vector<double> timeStepsD = convertToDouble( timeStepsInRange );

    // Create time steps which includes forecasting backward and forward
    const std::vector<time_t> outputTimeSteps =
        getOutputTimeSteps( timeStepsInRange, m_forecastBackward(), m_forecastForward(), m_forecastUnit() );
    const std::vector<double> outputTimeStepsD = convertToDouble( outputTimeSteps );

    // Move the time scale from seconds since epoch to years from first data point.
    // This gives better precision for the regression analysis.
    const double offset                          = timeStepsD[0];
    auto         convertToYearsFromFirstTimeStep = []( const std::vector<double>& timeSteps, double offset )
    {
        const double secondsPerYear = 60 * 60 * 24 * 365;

        std::vector<double> timeStepsH( timeSteps.size() );
        for ( size_t i = 0; i < timeSteps.size(); i++ )
        {
            timeStepsH[i] = ( timeSteps[i] - offset ) / secondsPerYear;
        }
        return timeStepsH;
    };

    const std::vector<double> timeStepsDYears       = convertToYearsFromFirstTimeStep( timeStepsD, offset );
    const std::vector<double> outputTimeStepsDYears = convertToYearsFromFirstTimeStep( outputTimeStepsD, offset );

    if ( m_regressionType == RegressionType::LINEAR )
    {
        regression::LinearRegression linearRegression;
        linearRegression.fit( timeStepsDYears, valuesInRange );
        std::vector<double> predictedValues = linearRegression.predict( outputTimeStepsDYears );
        return { outputTimeSteps, predictedValues, generateRegressionText( linearRegression ) };
    }
    else if ( m_regressionType == RegressionType::POLYNOMIAL )
    {
        regression::PolynomialRegression polynomialRegression;
        polynomialRegression.fit( timeStepsDYears, valuesInRange, m_polynomialDegree );
        std::vector<double> predictedValues = polynomialRegression.predict( outputTimeStepsDYears );
        return { outputTimeSteps, predictedValues, generateRegressionText( polynomialRegression ) };
    }
    else if ( m_regressionType == RegressionType::POWER_FIT )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsDYears, valuesInRange );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::PowerFitRegression powerFitRegression;
        powerFitRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = powerFitRegression.predict( outputTimeStepsDYears );
        return { outputTimeSteps, predictedValues, generateRegressionText( powerFitRegression ) };
    }
    else if ( m_regressionType == RegressionType::EXPONENTIAL )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsDYears, valuesInRange );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::ExponentialRegression exponentialRegression;
        exponentialRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = exponentialRegression.predict( outputTimeStepsDYears );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( exponentialRegression ) };
    }
    else if ( m_regressionType == RegressionType::LOGARITHMIC )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( timeStepsDYears, valuesInRange );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::LogarithmicRegression logarithmicRegression;
        logarithmicRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = logarithmicRegression.predict( outputTimeStepsDYears );
        return { convertToTimeT( outputTimeStepsD ), predictedValues, generateRegressionText( logarithmicRegression ) };
    }

    return { timeSteps, values, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    if ( m_dataSourceForRegression() == DataSource::ENSEMBLE )
    {
        uiOrdering.add( &m_ensembleCurveSet );
        uiOrdering.add( &m_ensembleStatisticsType );
    }

    caf::PdmUiGroup* regressionCurveGroup = uiOrdering.addNewGroup( "Regression Analysis" );
    regressionCurveGroup->add( &m_regressionType );

    if ( m_regressionType == RegressionType::POLYNOMIAL )
    {
        regressionCurveGroup->add( &m_polynomialDegree );
    }

    regressionCurveGroup->add( &m_expressionText );

    caf::PdmUiGroup* valueRangeYGroup = uiOrdering.addNewGroup( "Value Range Y" );
    valueRangeYGroup->add( &m_yRangeSelection );
    if ( m_yRangeSelection() == RangeType::USER_DEFINED_RANGE )
    {
        valueRangeYGroup->add( &m_valueRangeY );
    }

    if ( axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
    {
        caf::PdmUiGroup* valueRangeXGroup = uiOrdering.addNewGroup( "Value Range X" );
        valueRangeXGroup->add( &m_xRangeSelection );
        if ( m_xRangeSelection() == RangeType::USER_DEFINED_RANGE )
        {
            valueRangeXGroup->add( &m_valueRangeX );
        }
    }
    else
    {
        caf::PdmUiGroup* timeSelectionGroup = uiOrdering.addNewGroup( "Time Selection" );
        timeSelectionGroup->add( &m_timeRangeSelection );
        if ( m_timeRangeSelection() == RangeType::USER_DEFINED_RANGE )
        {
            timeSelectionGroup->add( &m_minTimeSliderPosition );
            timeSelectionGroup->add( &m_maxTimeSliderPosition );
        }
        timeSelectionGroup->add( &m_showTimeSelectionInPlot );
    }

    caf::PdmUiGroup* forecastingGroup = uiOrdering.addNewGroup( "Forecasting" );
    forecastingGroup->add( &m_forecastForward );
    forecastingGroup->add( &m_forecastBackward );
    forecastingGroup->add( &m_forecastUnit );

    if ( m_dataSourceForRegression() == DataSource::ENSEMBLE )
    {
        caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
        RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

        caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
        nameGroup->setCollapsedByDefault();
        nameGroup->add( &m_showLegend );
        RimPlotCurve::curveNameUiOrdering( *nameGroup );
    }
    else
    {
        RimSummaryCurve::defineUiOrdering( uiConfigName, uiOrdering );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    RimSummaryCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( &m_minTimeSliderPosition == changedField && m_minTimeSliderPosition > m_maxTimeSliderPosition )
    {
        m_maxTimeSliderPosition = m_minTimeSliderPosition;
    }

    if ( &m_maxTimeSliderPosition == changedField && m_maxTimeSliderPosition < m_minTimeSliderPosition )
    {
        m_minTimeSliderPosition = m_maxTimeSliderPosition;
    }

    loadAndUpdateDataAndPlot();

    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    if ( plot ) plot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                               QString                    uiConfigName,
                                                               caf::PdmUiEditorAttribute* attribute )
{
    RimSummaryCurve::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_polynomialDegree )
    {
        if ( auto* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            // Polynomial degree should be a positive number.
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
    else if ( field == &m_minTimeSliderPosition || field == &m_maxTimeSliderPosition )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum     = 0;
            myAttr->m_maximum     = 100;
            myAttr->m_showSpinBox = false;
        }
    }
    else if ( field == &m_expressionText )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;

            QFont font;
            auto  pointSize = font.pointSize();
            font.setPointSize( pointSize + 2 );
            myAttr->font = font;
        }
    }
    else if ( field == &m_valueRangeX )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_decimals        = 2;
            attr->m_sliderTickCount = 100;

            auto values = m_sourceValuesX;
            if ( !values.empty() )
            {
                attr->m_minimum = *std::min_element( values.begin(), values.end() );
                attr->m_maximum = *std::max_element( values.begin(), values.end() );
            }
        }
    }
    else if ( field == &m_valueRangeY )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_decimals        = 2;
            attr->m_sliderTickCount = 100;

            auto values = m_sourceValuesY;
            if ( !values.empty() )
            {
                attr->m_minimum = *std::min_element( values.begin(), values.end() );
                attr->m_maximum = *std::max_element( values.begin(), values.end() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryRegressionAnalysisCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_ensembleCurveSet )
    {
        QList<caf::PdmOptionItemInfo> options;
        options.append( { "None", nullptr } );

        auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( plot )
        {
            auto curveSets = plot->ensembleCurveSetCollection()->curveSets();

            for ( auto curveSet : curveSets )
            {
                options.append( { curveSet->name(), curveSet } );
            }
        }

        return options;
    }

    if ( fieldNeedingOptions == &m_ensembleStatisticsType )
    {
        QList<caf::PdmOptionItemInfo> options;
        options.append( { "None", nullptr } );

        if ( m_ensembleCurveSet() )
        {
            auto statisticsOptions = m_ensembleCurveSet()->statisticsOptions();

            std::vector<RifEclipseSummaryAddressDefines::StatisticsType> availableStatistics;
            if ( statisticsOptions->showP10Curve() )
            {
                availableStatistics.push_back( RifEclipseSummaryAddressDefines::StatisticsType::P10 );
            }
            if ( statisticsOptions->showP50Curve() )
            {
                availableStatistics.push_back( RifEclipseSummaryAddressDefines::StatisticsType::P50 );
            }
            if ( statisticsOptions->showP90Curve() )
            {
                availableStatistics.push_back( RifEclipseSummaryAddressDefines::StatisticsType::P90 );
            }
            if ( statisticsOptions->showMeanCurve() )
            {
                availableStatistics.push_back( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );
            }

            for ( const auto& statisticsType : availableStatistics )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseSummaryAddressDefines::StatisticsType>::uiText( statisticsType ),
                                            statisticsType ) );
            }
        }

        return options;
    }

    return RimSummaryCurve::calculateValueOptions( fieldNeedingOptions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::createCurveAutoName()
{
    QString sourceCurveName;
    if ( m_dataSourceForRegression() == DataSource::ENSEMBLE )
    {
        if ( m_ensembleCurveSet() )
        {
            sourceCurveName = m_ensembleCurveSet()->name();
        }
    }
    else
    {
        sourceCurveName = RimSummaryCurve::createCurveAutoName();
    }

    return sourceCurveName + " " + m_regressionType().uiText() + " Regression";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::curveExportDescription( const RifEclipseSummaryAddress& address ) const
{
    return RimSummaryCurve::curveExportDescription( {} ) + "." + m_regressionType().uiText() + "_Regression";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::LinearRegression& reg )
{
    return RiaRegressionTextTools::generateRegressionText( reg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::PolynomialRegression& reg )
{
    return RiaRegressionTextTools::generateRegressionText( reg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::PowerFitRegression& reg )
{
    return RiaRegressionTextTools::generateRegressionText( reg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::ExponentialRegression& reg )
{
    return RiaRegressionTextTools::generateRegressionText( reg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryRegressionAnalysisCurve::generateRegressionText( const regression::LogarithmicRegression& reg )
{
    return RiaRegressionTextTools::generateRegressionText( reg );
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
std::pair<time_t, time_t> RimSummaryRegressionAnalysisCurve::fullTimeStepRange() const
{
    auto timeSteps = m_sourceTimeStepsY;
    if ( !timeSteps.empty() )
    {
        return std::make_pair( *timeSteps.begin(), *timeSteps.rbegin() );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimSummaryRegressionAnalysisCurve::selectedTimeStepRange() const
{
    // Scale the slider values to the full time step range

    auto [min, max]  = fullTimeStepRange();
    auto range       = max - min;
    auto selectedMin = min + static_cast<time_t>( range * ( m_minTimeSliderPosition / 100.0 ) );
    auto selectedMax = min + static_cast<time_t>( range * ( m_maxTimeSliderPosition / 100.0 ) );

    return { selectedMin, selectedMax };
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
bool RimSummaryRegressionAnalysisCurve::isRegressionCurve() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTimeAxisAnnotation*> RimSummaryRegressionAnalysisCurve::createTimeAnnotations() const
{
    if ( m_showTimeSelectionInPlot && isChecked() )
    {
        auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

        return { RimTimeAxisAnnotation::createTimeRangeAnnotation( minTimeStep, maxTimeStep, color() ) };
    }

    return {};
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>> RimSummaryRegressionAnalysisCurve::getInRangeValues( const std::vector<time_t>& timeSteps,
                                                                                                         const std::vector<double>& values,
                                                                                                         time_t minTimeStep,
                                                                                                         time_t maxTimeStep )
{
    CAF_ASSERT( timeSteps.size() == values.size() );

    std::vector<time_t> filteredTimeSteps;
    std::vector<double> filteredValues;
    for ( size_t i = 0; i < timeSteps.size(); i++ )
    {
        time_t timeStep = timeSteps[i];
        if ( timeStep >= minTimeStep && timeStep <= maxTimeStep )
        {
            filteredTimeSteps.push_back( timeStep );
            filteredValues.push_back( values[i] );
        }
    }

    return std::make_pair( filteredTimeSteps, filteredValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::updateTimeAnnotations()
{
    if ( auto plot = firstAncestorOrThisOfType<RimSummaryPlot>() )
    {
        plot->updateAndRedrawTimeAnnotations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryRegressionAnalysisCurve::updateDefaultValues()
{
    if ( !m_sourceTimeStepsY.empty() && m_timeRangeSelection() == RangeType::FULL_RANGE )
    {
        m_minTimeSliderPosition = 0;
        m_maxTimeSliderPosition = 100;
    }

    if ( !m_sourceValuesX.empty() && m_xRangeSelection() == RangeType::FULL_RANGE )
    {
        m_valueRangeX = std::make_pair( *std::min_element( m_sourceValuesX.begin(), m_sourceValuesX.end() ),
                                        *std::max_element( m_sourceValuesX.begin(), m_sourceValuesX.end() ) );
    }

    if ( !m_sourceValuesY.empty() && m_yRangeSelection() == RangeType::FULL_RANGE )
    {
        m_valueRangeY = std::make_pair( *std::min_element( m_sourceValuesY.begin(), m_sourceValuesY.end() ),
                                        *std::max_element( m_sourceValuesY.begin(), m_sourceValuesY.end() ) );
    }
}
