/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiuDepthQwtPlot.h"

#include "RiaApplication.h"
#include "RiaCurveDataTools.h"

#include "RimCase.h"
#include "RimContextCommandBuilder.h"

#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtSymbol.h"
#include "RiuTextDialog.h"

#include "cvfColor3.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_engine.h"

#include <QContextMenuEvent>
#include <QMenu>

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDepthQwtPlot::RiuDepthQwtPlot( QWidget* parent )
    : RiuDockedQwtPlot( parent )
{
    setAutoFillBackground( true );
    setDefaults();
    resetRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDepthQwtPlot::~RiuDepthQwtPlot()
{
    deleteAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::resetRanges()
{
    m_minX = std::numeric_limits<double>::max();
    m_minY = std::numeric_limits<double>::max();
    m_maxX = std::numeric_limits<double>::lowest();
    m_maxY = std::numeric_limits<double>::lowest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::addCurve( const RimCase*             rimCase,
                                const QString&             curveName,
                                const cvf::Color3f&        curveColor,
                                const std::vector<int>&    kIndexes,
                                const std::vector<double>& depthValues,
                                const std::vector<double>& resultValues )
{
    if ( kIndexes.empty() || resultValues.empty() || depthValues.empty() )
    {
        return;
    }

    std::vector<double> filtered;
    for ( double x : depthValues )
    {
        if ( !std::isnan( x ) ) filtered.push_back( x );
    }

    if ( !filtered.empty() )
    {
        auto [min_it, max_it] = std::minmax_element( filtered.begin(), filtered.end() );
        m_maxY                = std::max( *max_it, m_maxY );
        m_minY                = std::min( *min_it, m_minY );
    }

    std::vector<double> tmpDepths;
    const double        nan = std::nan( "" );

    for ( int i = 0; i < (int)kIndexes.size(); i++ )
    {
        double val = resultValues[i];

        if ( std::isnan( val ) )
        {
            // the plot framework needs nan values in the Y data to detect valid intervals and keep good plot performance
            // if we have nan in the X data, set Y to nan, too
            tmpDepths.push_back( nan );
        }
        else
        {
            if ( val > m_maxX ) m_maxX = val;
            if ( val < m_minX ) m_minX = val;
            tmpDepths.push_back( depthValues[i] );
        }
    }

    RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve( nullptr );

    plotCurve->setSamplesFromXValuesAndYValues( resultValues, tmpDepths, false );
    plotCurve->setTitle( curveName );
    plotCurve->setAxes( QwtAxis::XTop, QwtAxis::YLeft );

    auto color = QColor( curveColor.rByte(), curveColor.gByte(), curveColor.bByte() );

    plotCurve->setPen( QPen( color ) );

    RiuQwtSymbol* symbol = new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS, "" );
    symbol->setSize( 6, 6 );
    symbol->setColor( color );
    plotCurve->setSymbol( symbol );

    plotCurve->attach( this );
    m_plotCurves.push_back( plotCurve );

    updateAxisScaling();

    applyFontSizes( false );

    replot();

    int caseId = rimCase->caseId();

    m_caseNames[caseId] = rimCase->caseUserDescription();
    m_curveNames[caseId].push_back( curveName );
    m_curveData[caseId].push_back( resultValues );
    m_kSteps[caseId] = kIndexes;
    m_depthValues[caseId].push_back( depthValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::deleteAllCurves()
{
    for ( size_t i = 0; i < m_plotCurves.size(); i++ )
    {
        m_plotCurves[i]->detach();
        delete m_plotCurves[i];
    }

    m_plotCurves.clear();

    m_caseNames.clear();
    m_curveNames.clear();
    m_curveData.clear();
    m_kSteps.clear();
    m_depthValues.clear();

    resetRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuDepthQwtPlot::sizeHint() const
{
    return QSize( 100, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuDepthQwtPlot::minimumSizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu;

    const int curveCount = itemList( QwtPlotItem::Rtti_PlotCurve ).count();

    QAction* act = menu.addAction( "Show Plot Data", this, SLOT( slotCurrentPlotDataInTextDialog() ) );
    act->setEnabled( curveCount > 0 );

    if ( !menu.actions().empty() )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::setDefaults()
{
    RiuQwtPlotTools::setCommonPlotBehaviour( this );

    setAxesCount( QwtAxis::XTop, 1 );
    setAxesCount( QwtAxis::YLeft, 1 );

    setAxisVisible( QwtAxis::XTop, true );
    setAxisVisible( QwtAxis::XBottom, false );

    setAxisMaxMinor( QwtAxis::XTop, 2 );
    setAxisMaxMinor( QwtAxis::YLeft, 6 );

    setAxisTitle( QwtAxis::YLeft, "Depth" );

    setAxisAutoScale( QwtAxis::YLeft, false );
    setAxisAutoScale( QwtAxis::XTop, false );

    applyFontSizes( false );

    QwtLegend* legend = new QwtLegend( this );
    insertLegend( legend, BottomLegend );

    RiuQwtPlotTools::enableGridLines( this, QwtAxis::XBottom, false, false );
    RiuQwtPlotTools::enableGridLines( this, QwtAxis::XTop, true, true );
    RiuQwtPlotTools::enableGridLines( this, QwtAxis::YLeft, true, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDepthQwtPlot::asciiDataForUiSelectedCurves() const
{
    QString out;

    for ( std::pair<int, QString> caseIdAndName : m_caseNames )
    {
        int caseId = caseIdAndName.first;
        out += "Case: " + caseIdAndName.second;
        out += "\n";

        for ( size_t i = 0; i < m_kSteps.at( caseId ).size(); i++ ) // time steps & data points
        {
            if ( i == 0 )
            {
                out += "K Index";
                for ( QString curveName : m_curveNames.at( caseId ) )
                {
                    out += "\tDepth";
                    out += "\t" + curveName;
                }
            }
            out += "\n";

            QString kString = QString::number( m_kSteps.at( caseId )[i] + 1 );

            out += kString;

            for ( size_t j = 0; j < m_curveData.at( caseId ).size(); j++ ) // curves
            {
                QString depthString = QString::number( m_depthValues.at( caseId )[j][i], 'f', 2 );
                out += "\t" + depthString;
                out += "\t" + QString::number( m_curveData.at( caseId )[j][i], 'g', 6 );
            }
        }
        out += "\n\n";
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::slotCurrentPlotDataInTextDialog()
{
    QString outTxt = asciiDataForUiSelectedCurves();

    RiuTextDialog* textDialog = new RiuTextDialog( this );
    textDialog->setMinimumSize( 600, 600 );
    textDialog->setWindowTitle( "Depth Plot Data" );
    textDialog->setText( outTxt );
    textDialog->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDepthQwtPlot::updateAxisScaling()
{
    double valRangeX = m_maxX - m_minX;
    if ( valRangeX == 0.0 ) valRangeX = 1.0;
    double valRangeY = m_maxY - m_minY;
    if ( valRangeY == 0.0 ) valRangeY = 1.0;

    setAxisScale( QwtAxis::YLeft, m_maxY + 0.02 * valRangeY, m_minY - 0.02 * valRangeY );
    setAxisScale( QwtAxis::XTop, m_minX - 0.02 * valRangeX, m_maxX + 0.1 * valRangeX );
}
