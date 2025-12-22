/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimGridCrossPlotRegressionCurve.h"

#include "RiaNumericalTools.h"
#include "RiaRegressionTextTools.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimPlotRectAnnotation.h"

#include "RimPlotRectAnnotation.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"

#include "ExponentialRegression.hpp"
#include "LinearRegression.hpp"
#include "LogarithmicRegression.hpp"
#include "PolynomialRegression.hpp"
#include "PowerFitRegression.hpp"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfMath.h"

CAF_PDM_SOURCE_INIT( RimGridCrossPlotRegressionCurve, "GridCrossPlotRegressionCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimGridCrossPlotRegressionCurve::RegressionType>::setUp()
{
    addItem( RimGridCrossPlotRegressionCurve::RegressionType::LINEAR, "LINEAR", "Linear" );
    addItem( RimGridCrossPlotRegressionCurve::RegressionType::POLYNOMIAL, "POLYNOMIAL", "Polynomial" );
    addItem( RimGridCrossPlotRegressionCurve::RegressionType::POWER_FIT, "POWER_FIT", "Power Fit" );
    addItem( RimGridCrossPlotRegressionCurve::RegressionType::EXPONENTIAL, "EXPONENTIAL", "Exponential" );
    addItem( RimGridCrossPlotRegressionCurve::RegressionType::LOGARITHMIC, "LOGARITHMIC", "Logarithmic" );
    setDefault( RimGridCrossPlotRegressionCurve::RegressionType::LINEAR );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotRegressionCurve::RimGridCrossPlotRegressionCurve()
    : m_dataSetIndex( 0 )
    , m_groupIndex( 0 )
{
    CAF_PDM_InitObject( "Cross Plot Regression Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_regressionType, "RegressionType", "Type" );

    CAF_PDM_InitFieldNoDefault( &m_minExtrapolationRangeX, "MinExtrapolationRangeX", "Min" );
    m_minExtrapolationRangeX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxExtrapolationRangeX, "MaxExtrapolationRangeX", "Max" );
    m_maxExtrapolationRangeX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_polynomialDegree, "PolynomialDegree", 3, "Degree" );
    m_polynomialDegree.setRange( 1, 50 );

    CAF_PDM_InitFieldNoDefault( &m_minRangeX, "MinRangeX", "Min X" );
    m_minRangeX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxRangeX, "MaxRangeX", "Max X" );
    m_maxRangeX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_minRangeY, "MinRangeY", "Min Y" );
    m_minRangeY.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxRangeY, "MaxRangeY", "Max Y" );
    m_maxRangeY.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showDataSelectionInPlot, "ShowDataSelectionInPlot", false, "Show In Plot" );

    CAF_PDM_InitFieldNoDefault( &m_expressionText, "ExpressionText", "Expression" );
    m_expressionText.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_expressionText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_expressionText.uiCapability()->setUiReadOnly( true );
    m_expressionText.xmlCapability()->disableIO();

    setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    setSymbol( RiuPlotCurveSymbol::SYMBOL_RECT );
    setSymbolSize( 6 );
    setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_REGRESSION_CURVE ) );

    m_dataRangeX = { cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE };
    m_dataRangeY = { cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE };

    auto rectAnnotation = new RimPlotRectAnnotation;
    rectAnnotation->setName( "Data Selection" );
    m_rectAnnotations.push_back( rectAnnotation );
    m_rectAnnotations.uiCapability()->setUiTreeChildrenHidden( true );

    setCheckState( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::setGroupingInformation( int dataSetIndex, int groupIndex )
{
    m_dataSetIndex = dataSetIndex;
    m_groupIndex   = groupIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::setSamples( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    CVF_ASSERT( xValues.size() == yValues.size() );

    if ( xValues.empty() || yValues.empty() || !m_plotCurve ) return;

    auto [minX, maxX] = minmax_element( xValues.begin(), xValues.end() );
    auto [minY, maxY] = minmax_element( yValues.begin(), yValues.end() );

    m_dataRangeX = { *minX, *maxX };
    m_dataRangeY = { *minY, *maxY };

    auto filterValues = []( const std::vector<double>& x, const std::vector<double>& y, double minX, double maxX, double minY, double maxY )
    {
        std::vector<double> filteredX;
        std::vector<double> filteredY;

        for ( size_t i = 0; i < x.size(); i++ )
        {
            if ( x[i] >= minX && x[i] <= maxX && y[i] >= minY && y[i] <= maxY )
            {
                filteredX.push_back( x[i] );
                filteredY.push_back( y[i] );
            }
        }

        return std::make_pair( filteredX, filteredY );
    };

    auto [filteredX, filteredY] = filterValues( xValues, yValues, m_minRangeX, m_maxRangeX, m_minRangeY, m_maxRangeY );
    if ( filteredX.empty() || filteredX.size() != filteredY.size() ) return;

    auto subsampleValues = []( double min, double max, int numSamples )
    {
        double step = ( max - min ) / numSamples;

        std::vector<double> subSampledRange( numSamples );
        for ( int i = 0; i < numSamples; i++ )
            subSampledRange[i] = min + step * i;
        subSampledRange.push_back( max );

        return subSampledRange;
    };

    std::vector<double> subsampledXValues = subsampleValues( m_minExtrapolationRangeX, m_maxExtrapolationRangeX, 50 );

    auto [outputXValues, outputYValues, regressionText] = calculateRegression( m_regressionType(), filteredX, filteredY, subsampledXValues );

    m_expressionText = regressionText;

    bool useLogarithmicScale = false;
    m_plotCurve->setSamplesFromXValuesAndYValues( outputXValues, outputYValues, useLogarithmicScale );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::setRangeDefaults( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    CVF_ASSERT( xValues.size() == yValues.size() );

    if ( xValues.empty() || yValues.empty() ) return;

    auto [minX, maxX] = minmax_element( xValues.begin(), xValues.end() );
    auto [minY, maxY] = minmax_element( yValues.begin(), yValues.end() );

    m_minRangeX = *minX;
    m_maxRangeX = *maxX;

    m_minRangeY = *minY;
    m_maxRangeY = *maxY;

    m_maxExtrapolationRangeX = *maxX;
    m_minExtrapolationRangeX = *minX;

    m_dataRangeX = { *minX, *maxX };
    m_dataRangeY = { *minY, *maxY };

    updateRectAnnotation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::swapAxis()
{
    std::swap( m_minRangeX, m_minRangeY );
    std::swap( m_maxRangeX, m_maxRangeY );

    m_maxExtrapolationRangeX = m_maxRangeX;
    m_minExtrapolationRangeX = m_minRangeX;

    std::swap( m_dataRangeX, m_dataRangeY );

    updateRectAnnotation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::setCurveAutoAppearance()
{
    updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::updateCurveAppearance()
{
    RimPlotCurve::updateCurveAppearance();
    updateRectAnnotation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlotRegressionCurve::groupIndex() const
{
    return m_groupIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGridCrossPlotRegressionCurve::sampleCount() const
{
    return m_plotCurve ? m_plotCurve->numSamples() : 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::determineLegendIcon()
{
    if ( !m_plotCurve ) return;

    auto plot     = firstAncestorOrThisOfTypeAsserted<RimGridCrossPlot>();
    int  fontSize = plot->legendFontSize();
    m_plotCurve->setLegendIconSize( QSize( fontSize, fontSize ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::setBlackAndWhiteLegendIcons( bool blackAndWhite )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setBlackAndWhiteLegendIcon( blackAndWhite );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::updateZoomInParentPlot()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimGridCrossPlot>();
    plot->calculateZoomRangeAndUpdateQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotRegressionCurve::createCurveAutoName()
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotRegressionCurve::getRegressionTypeString() const
{
    return m_regressionType().uiText() + " Regression";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( updateParentPlot )
    {
        m_parentPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* regressionCurveGroup = uiOrdering.addNewGroup( "Regression Analysis" );
    regressionCurveGroup->add( &m_regressionType );

    if ( m_regressionType == RegressionType::POLYNOMIAL )
    {
        regressionCurveGroup->add( &m_polynomialDegree );
    }

    regressionCurveGroup->add( &m_expressionText );

    caf::PdmUiGroup* dataSelectionGroup = uiOrdering.addNewGroup( "Data Selection" );
    dataSelectionGroup->add( &m_minRangeX );
    dataSelectionGroup->add( &m_maxRangeX );
    dataSelectionGroup->add( &m_minRangeY );
    dataSelectionGroup->add( &m_maxRangeY );
    dataSelectionGroup->add( &m_showDataSelectionInPlot );

    caf::PdmUiGroup* forecastingGroup = uiOrdering.addNewGroup( "Extrapolation" );
    forecastingGroup->add( &m_minExtrapolationRangeX );
    forecastingGroup->add( &m_maxExtrapolationRangeX );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    // Implement an empty method to avoid the base class implementation in RimPlotCurve
    // The color tag is not used for Grid Cross Plot Curves
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                             QString                    uiConfigName,
                                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_minRangeX || field == &m_maxRangeX )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            auto [min, max]    = m_dataRangeX;
            myAttr->m_minimum  = RiaNumericalTools::roundToNumSignificantDigitsFloor( min, 2 );
            myAttr->m_maximum  = RiaNumericalTools::roundToNumSignificantDigitsCeil( max, 2 );
            myAttr->m_decimals = 3;
        }
    }
    else if ( field == &m_minRangeY || field == &m_maxRangeY )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            auto [min, max]    = m_dataRangeY;
            myAttr->m_minimum  = RiaNumericalTools::roundToNumSignificantDigitsFloor( min, 2 );
            myAttr->m_maximum  = RiaNumericalTools::roundToNumSignificantDigitsCeil( max, 2 );
            myAttr->m_decimals = 3;
        }
    }
    else if ( field == &m_minExtrapolationRangeX || field == &m_maxExtrapolationRangeX )
    {
        caf::PdmUiDoubleValueEditorAttribute::testAndSetFixedWithTwoDecimals( attribute );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_showCurve == changedField )
    {
        // RimPlotCurve::fieldChangedByUi always replot, and this is usually unnecessary except for visibility changes
        RimPlotCurve::fieldChangedByUi( changedField, oldValue, newValue );
    }

    auto enforceRange = []( const caf::PdmFieldHandle* changedField, caf::PdmField<double>& minRange, caf::PdmField<double>& maxRange )
    {
        if ( &minRange == changedField && minRange > maxRange ) maxRange = minRange;
        if ( &maxRange == changedField && maxRange < minRange ) minRange = maxRange;
    };

    enforceRange( changedField, m_minRangeX, m_maxRangeX );
    enforceRange( changedField, m_minRangeY, m_maxRangeY );

    if ( &m_minRangeX == changedField || &m_maxRangeX == changedField || &m_minRangeY == changedField || &m_maxRangeY == changedField ||
         &m_showDataSelectionInPlot == changedField )
    {
        updateRectAnnotation();
    }

    if ( &m_minRangeX == changedField || &m_maxRangeX == changedField || &m_minRangeY == changedField || &m_maxRangeY == changedField ||
         &m_minExtrapolationRangeX == changedField || &m_maxExtrapolationRangeX == changedField || &m_regressionType == changedField ||
         &m_polynomialDegree == changedField || &m_showDataSelectionInPlot == changedField || &m_showCurve == changedField )
    {
        auto dataSet = firstAncestorOrThisOfTypeAsserted<RimGridCrossPlotDataSet>();
        dataSet->updateRegressionCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<std::vector<double>, std::vector<double>, QString>
    RimGridCrossPlotRegressionCurve::calculateRegression( RimGridCrossPlotRegressionCurve::RegressionType regressionType,
                                                          const std::vector<double>&                      xValues,
                                                          const std::vector<double>&                      yValues,
                                                          const std::vector<double>&                      outputXValues ) const
{
    if ( regressionType == RegressionType::LINEAR )
    {
        regression::LinearRegression linearRegression;
        linearRegression.fit( xValues, yValues );
        std::vector<double> predictedValues = linearRegression.predict( outputXValues );
        return { outputXValues, predictedValues, RiaRegressionTextTools::generateRegressionText( linearRegression ) };
    }
    else if ( m_regressionType == RegressionType::POLYNOMIAL )
    {
        regression::PolynomialRegression polynomialRegression;
        polynomialRegression.fit( xValues, yValues, m_polynomialDegree );
        std::vector<double> predictedValues = polynomialRegression.predict( outputXValues );
        return { outputXValues, predictedValues, RiaRegressionTextTools::generateRegressionText( polynomialRegression ) };
    }
    else if ( m_regressionType == RegressionType::POWER_FIT )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( xValues, yValues );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::PowerFitRegression powerFitRegression;
        powerFitRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = powerFitRegression.predict( outputXValues );
        return { outputXValues, predictedValues, RiaRegressionTextTools::generateRegressionText( powerFitRegression ) };
    }
    else if ( m_regressionType == RegressionType::EXPONENTIAL )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( xValues, yValues );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::ExponentialRegression exponentialRegression;
        exponentialRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = exponentialRegression.predict( outputXValues );
        return { outputXValues, predictedValues, RiaRegressionTextTools::generateRegressionText( exponentialRegression ) };
    }
    else if ( m_regressionType == RegressionType::LOGARITHMIC )
    {
        auto [filteredTimeSteps, filteredValues] = getPositiveValues( xValues, yValues );
        if ( filteredTimeSteps.empty() || filteredValues.empty() ) return {};

        regression::LogarithmicRegression logarithmicRegression;
        logarithmicRegression.fit( filteredTimeSteps, filteredValues );
        std::vector<double> predictedValues = logarithmicRegression.predict( outputXValues );
        return { outputXValues, predictedValues, RiaRegressionTextTools::generateRegressionText( logarithmicRegression ) };
    }

    return { {}, {}, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>> RimGridCrossPlotRegressionCurve::getPositiveValues( const std::vector<double>& xValues,
                                                                                                        const std::vector<double>& yValues )
{
    std::vector<double> filteredXValues;
    std::vector<double> filteredYValues;
    for ( size_t i = 0; i < xValues.size(); i++ )
    {
        if ( xValues[i] > 0.0 && yValues[i] > 0.0 )
        {
            filteredXValues.push_back( xValues[i] );
            filteredYValues.push_back( yValues[i] );
        }
    }

    return std::make_pair( filteredXValues, filteredYValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotRegressionCurve::updateRectAnnotation()
{
    if ( !m_rectAnnotations.empty() )
    {
        RimPlotRectAnnotation* annotation = m_rectAnnotations[0];
        annotation->setRangeX( m_minRangeX, m_maxRangeX );
        annotation->setRangeY( m_minRangeY, m_maxRangeY );
        annotation->setColor( m_curveAppearance->color() );
        annotation->setCheckState( m_showDataSelectionInPlot() );

        auto dataSet = firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();
        if ( dataSet )
        {
            QString textLines;
            textLines += QString( "<b>Case:</b> %1<br>" ).arg( dataSet->caseNameString() );
            textLines += QString( "<b>%1:</b> %2 - %3<br>" ).arg( dataSet->xAxisName() ).arg( m_minRangeX() ).arg( m_maxRangeX() );
            textLines += QString( "<b>%1:</b> %2 - %3<br>" ).arg( dataSet->yAxisName() ).arg( m_minRangeY() ).arg( m_maxRangeY() );
            annotation->setText( textLines );
        }
    }
}
