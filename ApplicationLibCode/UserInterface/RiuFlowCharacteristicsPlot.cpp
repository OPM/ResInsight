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

#include "RiuFlowCharacteristicsPlot.h"

#include "RiaColorTables.h"
#include "RiaFeatureCommandContext.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include "RimFlowCharacteristicsPlot.h"

#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtSymbol.h"
#include "RiuResultQwtPlot.h"

#include "cvfColor3.h"

#include "cafCmdFeatureMenuBuilder.h"

#include "qwt_date.h"
#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_zoomer.h"
#include "qwt_text.h"

#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QLabel>
#include <QMenu>

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuFlowCharacteristicsPlot::RiuFlowCharacteristicsPlot( RimFlowCharacteristicsPlot* plotDefinition, QWidget* parent )
    : QFrame( parent )
    , m_plotDefinition( plotDefinition )

{
    Q_ASSERT( m_plotDefinition );

    QGridLayout* mainLayout = new QGridLayout();
    setLayout( mainLayout );
    layout()->setContentsMargins( 3, 3, 3, 3 );
    layout()->setSpacing( 3 );

    // White background
    QPalette pal = palette();
    pal.setColor( QPalette::Window, Qt::white );
    setAutoFillBackground( true );
    setPalette( pal );

    m_lorenzPlot              = new QwtPlot( this );
    m_flowCapVsStorageCapPlot = new QwtPlot( this );
    m_sweepEffPlot            = new QwtPlot( this );

    mainLayout->addWidget( m_lorenzPlot, 0, 0, 1, 2 );
    mainLayout->addWidget( m_flowCapVsStorageCapPlot, 1, 0 );
    mainLayout->addWidget( m_sweepEffPlot, 1, 1 );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_lorenzPlot );
    new RiuQwtPlotWheelZoomer( m_lorenzPlot );
    addWindowZoom( m_lorenzPlot );

    QString dateFormat = RiaPreferences::current()->dateFormat();
    QString timeFormat = RiaPreferences::current()->timeFormat();

    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_lorenzPlot, dateFormat, timeFormat );
    m_lorenzPlot->setTitle( "Lorenz Coefficient" );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_sweepEffPlot );
    new RiuQwtPlotWheelZoomer( m_sweepEffPlot );
    addWindowZoom( m_sweepEffPlot );
    m_sweepEffPlot->setTitle( "Sweep Efficiency" );

    int legendFontSize =
        caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), caf::FontTools::RelativeSize::Small );

    {
        QwtText axisTitle = m_sweepEffPlot->axisTitle( QwtAxis::XBottom );
        auto    font      = axisTitle.font();
        font.setPointSize( legendFontSize );
        axisTitle.setFont( font );
        axisTitle.setText( "Dimensionless Time" );
        m_sweepEffPlot->setAxisTitle( QwtAxis::XBottom, axisTitle );
    }
    {
        QwtText axisTitle = m_sweepEffPlot->axisTitle( QwtAxis::YLeft );
        auto    font      = axisTitle.font();
        font.setPointSize( legendFontSize );
        axisTitle.setFont( font );
        axisTitle.setText( "Sweep Efficiency" );
        m_sweepEffPlot->setAxisTitle( QwtAxis::YLeft, axisTitle );
    }

    RiuQwtPlotTools::setCommonPlotBehaviour( m_flowCapVsStorageCapPlot );
    new RiuQwtPlotWheelZoomer( m_flowCapVsStorageCapPlot );
    addWindowZoom( m_flowCapVsStorageCapPlot );
    m_flowCapVsStorageCapPlot->setTitle( "Flow Capacity vs Storage Capacity" );

    {
        QwtText axisTitle = m_flowCapVsStorageCapPlot->axisTitle( QwtAxis::XBottom );
        auto    font      = axisTitle.font();
        font.setPointSize( legendFontSize );
        axisTitle.setFont( font );
        axisTitle.setText( "Storage Capacity [C]" );
        m_flowCapVsStorageCapPlot->setAxisTitle( QwtAxis::XBottom, axisTitle );
    }
    {
        QwtText axisTitle = m_flowCapVsStorageCapPlot->axisTitle( QwtAxis::YLeft );
        auto    font      = axisTitle.font();
        font.setPointSize( legendFontSize );
        axisTitle.setFont( font );
        axisTitle.setText( "Flow Capacity [F]" );
        m_flowCapVsStorageCapPlot->setAxisTitle( QwtAxis::YLeft, axisTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addWindowZoom( QwtPlot* plot )
{
    auto zoomer = new RiuQwtPlotZoomer( plot->canvas() );
    zoomer->setTrackerMode( QwtPicker::AlwaysOff );
    zoomer->initMousePattern( 1 );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuFlowCharacteristicsPlot::~RiuFlowCharacteristicsPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::setCaseName( const QString& caseName )
{
    QString title = "Lorenz Coefficient";
    if ( !caseName.isEmpty() )
    {
        title += " - " + caseName;
    }
    m_lorenzPlot->setTitle( title );

    setWindowTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::setLorenzCurve( const QStringList&            dateTimeStrings,
                                                 const std::vector<QDateTime>& dateTimes,
                                                 const std::vector<double>&    timeHistoryValues )
{
    m_lorenzPlot->detachItems( QwtPlotItem::Rtti_PlotCurve, true );

    if ( !dateTimes.empty() )
    {
        initializeColors( dateTimes );

        for ( size_t tsIdx = 0; tsIdx < dateTimes.size(); ++tsIdx )
        {
            if ( timeHistoryValues[tsIdx] == HUGE_VAL ) continue;

            QDateTime dateTime         = dateTimes[tsIdx];
            double    timeHistoryValue = timeHistoryValues[tsIdx];

            QString curveName = dateTimeStrings[static_cast<int>( tsIdx )];

            RiuFlowCharacteristicsPlot::addCurveWithLargeSymbol( m_lorenzPlot, curveName, m_dateToColorMap[dateTime], dateTime, timeHistoryValue );
        }
    }

    m_lorenzPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addCurveWithLargeSymbol( QwtPlot*         plot,
                                                          const QString&   curveName,
                                                          const QColor&    color,
                                                          const QDateTime& dateTime,
                                                          double           timeHistoryValue )
{
    auto curve = createEmptyCurve( plot, curveName, color );

    RiuPlotCurveSymbol* symbol = new RiuQwtSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_DIAMOND );
    symbol->setSize( 15, 15 );
    symbol->setColor( color );

    curve->setSymbol( symbol );

    // Add date and value twice to avoid a cross as symbol generated by RiuQwtPlotCurve

    std::vector<QDateTime> dateTimes;
    dateTimes.push_back( dateTime );
    dateTimes.push_back( dateTime );

    std::vector<double> timeHistoryValues;
    timeHistoryValues.push_back( timeHistoryValue );
    timeHistoryValues.push_back( timeHistoryValue );

    curve->setSamplesFromDatesAndYValues( dateTimes, timeHistoryValues, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve* RiuFlowCharacteristicsPlot::createEmptyCurve( QwtPlot* plot, const QString& curveName, const QColor& curveColor )
{
    RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve( nullptr, curveName );

    plotCurve->setTitle( curveName );
    plotCurve->setPen( QPen( curveColor ) );
    plotCurve->attach( plot );
    return plotCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addFlowCapStorageCapCurve( const QDateTime&           dateTime,
                                                            const std::vector<double>& xVals,
                                                            const std::vector<double>& yVals )
{
    CVF_ASSERT( !m_dateToColorMap.empty() );

    RiuQwtPlotCurve* plotCurve           = createEmptyCurve( m_flowCapVsStorageCapPlot, dateTime.toString(), m_dateToColorMap[dateTime] );
    bool             useLogarithmicScale = false;
    plotCurve->setSamplesFromXValuesAndYValues( xVals, yVals, useLogarithmicScale );
    m_flowCapVsStorageCapPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addSweepEfficiencyCurve( const QDateTime&           dateTime,
                                                          const std::vector<double>& xVals,
                                                          const std::vector<double>& yVals )
{
    CVF_ASSERT( !m_dateToColorMap.empty() );

    RiuQwtPlotCurve* plotCurve           = createEmptyCurve( m_sweepEffPlot, dateTime.toString(), m_dateToColorMap[dateTime] );
    bool             useLogarithmicScale = false;
    plotCurve->setSamplesFromXValuesAndYValues( xVals, yVals, useLogarithmicScale );

    m_sweepEffPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::removeAllCurves()
{
    m_lorenzPlot->detachItems( QwtPlotItem::Rtti_PlotCurve, true );
    m_lorenzPlot->replot();
    m_sweepEffPlot->detachItems( QwtPlotItem::Rtti_PlotCurve, true );
    m_sweepEffPlot->replot();
    m_flowCapVsStorageCapPlot->detachItems( QwtPlotItem::Rtti_PlotCurve, true );
    m_flowCapVsStorageCapPlot->replot();
    m_dateToColorMap.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void zoomAllInPlot( QwtPlot* plot )
{
    plot->setAxisAutoScale( QwtAxis::XBottom, true );
    plot->setAxisAutoScale( QwtAxis::YLeft, true );
    plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::zoomAll()
{
    zoomAllInPlot( m_lorenzPlot );
    zoomAllInPlot( m_sweepEffPlot );
    zoomAllInPlot( m_flowCapVsStorageCapPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::showLegend( bool show )
{
    if ( show )
    {
        // Will be released in plot destructor or when a new legend is set
        QwtLegend* legend = new QwtLegend( m_lorenzPlot );
        m_lorenzPlot->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_lorenzPlot->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuFlowCharacteristicsPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuFlowCharacteristicsPlot::minimumSizeHint() const
{
    return QSize( 0, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::contextMenuEvent( QContextMenuEvent* event )
{
    if ( m_plotDefinition )
    {
        QString curveDataAsText = m_plotDefinition->curveDataAsText();

        QString dialogTitle = "Flow Characteristics";

        RiaFeatureCommandContextTextHelper helper( dialogTitle, curveDataAsText );

        caf::CmdFeatureMenuBuilder menuBuilder;
        menuBuilder << "RicShowPlotDataFeature";

        QMenu menu;
        menuBuilder.appendToMenu( &menu );

        if ( !menu.actions().empty() )
        {
            menu.exec( event->globalPos() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuFlowCharacteristicsPlot::sizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::setDefaults()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::initializeColors( const std::vector<QDateTime>& dateTimes )
{
    CVF_ASSERT( m_dateToColorMap.empty() );

    const caf::ColorTable& palette    = RiaColorTables::timestepsPaletteColors();
    cvf::Color3ubArray     colorArray = caf::ColorTable::interpolateColorArray( palette.color3ubArray(), dateTimes.size() );

    for ( size_t tsIdx = 0; tsIdx < dateTimes.size(); ++tsIdx )
    {
        m_dateToColorMap[dateTimes[tsIdx]] = QColor( colorArray[tsIdx].r(), colorArray[tsIdx].g(), colorArray[tsIdx].b() );
    }
}
