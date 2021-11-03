/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimQwtPlotCurve.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaCurveDataTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimNameConfig.h"
#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuRimQwtPlotCurve.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_date.h"
#include "qwt_interval_symbol.h"
#include "qwt_plot.h"
#include "qwt_symbol.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimQwtPlotCurve, "QwtPlotCurve" );

#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlotCurve::RimQwtPlotCurve()
{
    CAF_PDM_InitObject( "Curve", ":/WellLogCurve16x16.png", "", "" );

    m_qwtPlotCurve      = new RiuRimQwtPlotCurve( this );
    m_qwtCurveErrorBars = new QwtPlotIntervalCurve();
    m_qwtCurveErrorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_qwtCurveErrorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_qwtCurveErrorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_qwtCurveErrorBars->setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ERROR_BARS ) );

    m_parentQwtPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlotCurve::~RimQwtPlotCurve()
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->detach();
        delete m_qwtPlotCurve;
        m_qwtPlotCurve = nullptr;
    }

    if ( m_qwtCurveErrorBars )
    {
        m_qwtCurveErrorBars->detach();
        delete m_qwtCurveErrorBars;
        m_qwtCurveErrorBars = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setErrorBarsVisible( bool isVisible )
{
    m_showErrorBars = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateCurveVisibility()
{
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
    else
    {
        m_qwtPlotCurve->detach();
        m_qwtCurveErrorBars->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setParentQwtPlotAndReplot( QwtPlot* plot )
{
    m_parentQwtPlot = plot;
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();

        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setParentQwtPlotNoReplot( QwtPlot* plot )
{
    m_parentQwtPlot = plot;
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
    else
    {
        m_qwtPlotCurve->detach();
        m_qwtCurveErrorBars->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::detachQwtCurve()
{
    m_qwtPlotCurve->detach();
    m_qwtCurveErrorBars->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::reattachQwtCurve()
{
    detachQwtCurve();
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimQwtPlotCurve::qwtPlotCurve() const
{
    return m_qwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateCurveNameAndUpdatePlotLegendAndTitle()
{
    updateCurveName();
    updateLegendEntryVisibilityAndPlotLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateCurveNameNoLegendUpdate()
{
    updateCurveName();
    updateLegendEntryVisibilityNoPlotUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateOptionSensitivity()
{
    m_curveName.uiCapability()->setUiReadOnly( m_isUsingAutoName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updatePlotTitle()
{
    RimNameConfigHolderInterface* nameConfigHolder = nullptr;
    this->firstAncestorOrThisOfType( nameConfigHolder );
    if ( nameConfigHolder )
    {
        nameConfigHolder->updateAutoName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateLegendsInPlot()
{
    nameChanged.send( curveName() );
    if ( m_parentQwtPlot != nullptr )
    {
        m_parentQwtPlot->updateLegend();
    }
}

void RimQwtPlotCurve::setCurveTitle( const QString& title )
{
    if ( m_qwtPlotCurve ) m_qwtPlotCurve->setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::refreshParentPlot()
{
    if ( m_parentQwtPlot ) m_parentQwtPlot->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::replotParentPlot()
{
    if ( m_parentQwtPlot ) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimQwtPlotCurve::hasParentPlot() const
{
    return ( m_parentQwtPlot != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setSamplesFromXYErrorValues(
    const std::vector<double>&   xValues,
    const std::vector<double>&   yValues,
    const std::vector<double>&   errorValues,
    bool                         keepOnlyPositiveValues,
    RiaCurveDataTools::ErrorAxis errorAxis /*= RiuQwtPlotCurve::ERROR_ALONG_Y_AXIS */ )
{
    CVF_ASSERT( xValues.size() == yValues.size() );
    CVF_ASSERT( xValues.size() == errorValues.size() );

    auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues, keepOnlyPositiveValues );
    std::vector<double> filteredYValues;
    std::vector<double> filteredXValues;

    RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &filteredYValues );
    RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &filteredXValues );

    std::vector<double> filteredErrorValues;
    RiaCurveDataTools::getValuesByIntervals( errorValues, intervalsOfValidValues, &filteredErrorValues );

    QVector<QwtIntervalSample> errorIntervals;

    errorIntervals.reserve( static_cast<int>( filteredXValues.size() ) );

    for ( size_t i = 0; i < filteredXValues.size(); i++ )
    {
        if ( filteredYValues[i] != DOUBLE_INF && filteredErrorValues[i] != DOUBLE_INF )
        {
            if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
            {
                errorIntervals << QwtIntervalSample( filteredXValues[i],
                                                     filteredYValues[i] - filteredErrorValues[i],
                                                     filteredYValues[i] + filteredErrorValues[i] );
            }
            else
            {
                errorIntervals << QwtIntervalSample( filteredYValues[i],
                                                     filteredXValues[i] - filteredErrorValues[i],
                                                     filteredXValues[i] + filteredErrorValues[i] );
            }
        }
    }

    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamples( filteredXValues.data(),
                                    filteredYValues.data(),
                                    static_cast<int>( filteredXValues.size() ) );

        m_qwtPlotCurve->setLineSegmentStartStopIndices( intervalsOfValidValues );
    }

    if ( m_qwtCurveErrorBars )
    {
        m_qwtCurveErrorBars->setSamples( errorIntervals );
        if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Vertical );
        }
        else
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Horizontal );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setSamplesFromXYValues( const std::vector<double>& xValues,
                                              const std::vector<double>& yValues,
                                              bool                       keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromXValuesAndYValues( xValues, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                     const std::vector<double>&    yValues,
                                                     bool                          keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromDatesAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                     const std::vector<double>& yValues,
                                                     bool                       keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromTimeTAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateUiIconFromPlotSymbol()
{
    if ( m_curveAppearance->symbol() != RiuQwtSymbol::SYMBOL_NONE && m_qwtPlotCurve )
    {
        CVF_ASSERT( RiaGuiApplication::isRunning() );
        QSizeF     iconSize( 24, 24 );
        QwtGraphic graphic = m_qwtPlotCurve->legendIcon( 0, iconSize );
        QPixmap    pixmap  = graphic.toPixmap();
        setUiIcon( caf::IconProvider( pixmap ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::attachCurveAndErrorBars()
{
    m_qwtPlotCurve->attach( m_parentQwtPlot );

    if ( m_showErrorBars )
    {
        m_qwtCurveErrorBars->attach( m_parentQwtPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::clearErrorBars()
{
    m_qwtCurveErrorBars->setSamples( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateCurveAppearance()
{
    QColor     curveColor = RiaColorTools::toQColor( m_curveAppearance->color() );
    QwtSymbol* symbol     = nullptr;

    if ( m_curveAppearance->symbol() != RiuQwtSymbol::SYMBOL_NONE )
    {
        int legendFontSize        = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                                caf::FontTools::RelativeSize::Small );
        RimPlotWindow* plotWindow = nullptr;
        this->firstAncestorOrThisOfType( plotWindow );
        if ( plotWindow )
        {
            legendFontSize = plotWindow->legendFontSize();
        }

        // QwtPlotCurve will take ownership of the symbol
        symbol = new RiuQwtSymbol( m_curveAppearance->symbol(),
                                   m_curveAppearance->symbolLabel(),
                                   m_curveAppearance->symbolLabelPosition(),
                                   legendFontSize );
        symbol->setSize( m_curveAppearance->symbolSize(), m_curveAppearance->symbolSize() );
        symbol->setColor( curveColor );

        // If the symbol is a "filled" symbol, we can have a different edge color
        // Otherwise we'll have to use the curve color.
        if ( RiuQwtSymbol::isFilledSymbol( m_curveAppearance->symbol() ) )
        {
            QColor symbolEdgeColor = RiaColorTools::toQColor( m_curveAppearance->symbolEdgeColor() );
            symbol->setPen( symbolEdgeColor );
        }
        else
        {
            symbol->setPen( curveColor );
        }
    }

    // TODO:

    if ( m_qwtCurveErrorBars )
    {
        QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        newSymbol->setPen( QPen( curveColor ) );
        m_qwtCurveErrorBars->setSymbol( newSymbol );
    }

    if ( m_qwtPlotCurve )
    {
        QColor fillColor = RiaColorTools::toQColor( m_curveAppearance->fillColor() );

        fillColor = RiaColorTools::blendQColors( fillColor, QColor( Qt::white ), 3, 1 );
        QBrush fillBrush( fillColor, m_curveAppearance->fillStyle() );
        m_qwtPlotCurve->setAppearance( m_curveAppearance->lineStyle(),
                                       m_curveAppearance->interpolation(),
                                       m_curveAppearance->lineThickness(),
                                       curveColor,
                                       fillBrush );
        m_qwtPlotCurve->setSymbol( symbol );
        m_qwtPlotCurve->setSymbolSkipPixelDistance( m_curveAppearance->symbolSkipDistance() );

        // Make sure the legend lines are long enough to distinguish between line types.
        // Standard width in Qwt is 8 which is too short.
        // Use 10 and scale this by curve thickness + add space for displaying symbol.
        if ( m_curveAppearance->lineStyle() != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
        {
            QSize legendIconSize = m_qwtPlotCurve->legendIconSize();

            int symbolWidth = 0;
            if ( symbol )
            {
                symbolWidth = symbol->boundingRect().size().width() + 2;
            }

            int width = std::max( 10 * m_curveAppearance->lineThickness(), ( symbolWidth * 3 ) / 2 );

            legendIconSize.setWidth( width );
            m_qwtPlotCurve->setLegendIconSize( legendIconSize );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::loadDataAndUpdate( bool updateParentPlot )
{
    this->onLoadDataAndUpdate( updateParentPlot );
    if ( updateParentPlot )
    {
        dataChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimQwtPlotCurve::xValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_qwtPlotCurve );

    if ( m_qwtPlotCurve->data()->size() < 1 )
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minXValue();
    *maximumValue = m_qwtPlotCurve->maxXValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimQwtPlotCurve::yValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_qwtPlotCurve );

    if ( m_qwtPlotCurve->data()->size() < 1 )
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minYValue();
    *maximumValue = m_qwtPlotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimQwtPlotCurve::errorBarsVisible() const
{
    return m_showErrorBars;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::setZOrder( double z )
{
    if ( m_qwtPlotCurve != nullptr )
    {
        m_qwtPlotCurve->setZ( z );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateLegendEntryVisibilityAndPlotLegend()
{
    updateLegendEntryVisibilityNoPlotUpdate();
    updateLegendsInPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_qwtPlotCurve ) return;

    RimEnsembleCurveSet* ensembleCurveSet = nullptr;
    this->firstAncestorOrThisOfType( ensembleCurveSet );
    if ( ensembleCurveSet )
    {
        return;
    }

    bool showLegendInQwt = m_showLegend();

    RimSummaryPlot* summaryPlot = nullptr;
    this->firstAncestorOrThisOfType( summaryPlot );
    if ( summaryPlot )
    {
        bool anyCalculated = false;
        for ( const auto c : summaryPlot->summaryCurves() )
        {
            if ( c->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
            {
                // Never hide the legend for calculated curves, as the curve legend is used to
                // show some essential auto generated data
                anyCalculated = true;
            }
        }

        if ( !anyCalculated && summaryPlot->ensembleCurveSetCollection()->curveSets().empty() &&
             summaryPlot->curveCount() == 1 )
        {
            // Disable display of legend if the summary plot has only one single curve
            showLegendInQwt = false;
        }
    }
    m_qwtPlotCurve->setItemAttribute( QwtPlotItem::Legend, showLegendInQwt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlotCurve::updateAxisInPlot( RiaDefines::PlotAxis plotAxis )
{
    if ( m_qwtPlotCurve )
    {
        if ( plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        {
            m_qwtPlotCurve->setYAxis( QwtPlot::yLeft );
        }
        else
        {
            m_qwtPlotCurve->setYAxis( QwtPlot::yRight );
        }
    }
}
