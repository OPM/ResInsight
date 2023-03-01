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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDepthQwtPlot::RiuDepthQwtPlot( QWidget* parent )
    : RiuDockedQwtPlot( parent )
{
    setAutoFillBackground( true );
    setDefaults();
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
void RiuDepthQwtPlot::addCurve( const RimCase*             rimCase,
                                const QString&             curveName,
                                const cvf::Color3f&        curveColor,
                                const std::vector<int>&    kIndexes,
                                const std::vector<double>& depthValues )
{
    if ( kIndexes.empty() || depthValues.empty() )
    {
        return;
    }

    std::vector<double> kValues;
    for ( auto k : kIndexes )
    {
        kValues.push_back( 1.0 * k + 1.0 ); // adjust to eclipse index
    }

    RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve( nullptr, "Curve 1" );

    plotCurve->setSamplesFromXValuesAndYValues( kValues, depthValues, false );
    plotCurve->setTitle( curveName );

    auto color = QColor( curveColor.rByte(), curveColor.gByte(), curveColor.bByte() );

    plotCurve->setPen( QPen( color ) );

    RiuQwtSymbol* symbol = new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS, "" );
    symbol->setSize( 6, 6 );
    symbol->setColor( color );
    plotCurve->setSymbol( symbol );

    plotCurve->attach( this );
    m_plotCurves.push_back( plotCurve );

    this->setAxisScale( QwtAxis::XBottom, kIndexes.front(), kIndexes.back() );
    this->applyFontSizes( false );

    this->replot();

    int caseId = rimCase->caseId();

    m_caseNames[caseId] = rimCase->caseUserDescription();
    m_curveNames[caseId].push_back( curveName );
    m_curveData[caseId].push_back( depthValues );
    m_kSteps[caseId] = kValues;
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

    const int curveCount = this->itemList( QwtPlotItem::Rtti_PlotCurve ).count();

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

    setAxesCount( QwtAxis::XBottom, 1 );
    setAxesCount( QwtAxis::YLeft, 1 );

    setAxisMaxMinor( QwtAxis::XBottom, 2 );
    setAxisMaxMinor( QwtAxis::YLeft, 3 );

    applyFontSizes( false );

    // The legend will be deleted in the destructor of the plot or when
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend( this );
    this->insertLegend( legend, BottomLegend );
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
                    out += "\t" + curveName;
                }
            }
            out += "\n";

            QString kString = QString::number( m_kSteps.at( caseId )[i] );

            out += kString;

            for ( size_t j = 0; j < m_curveData.at( caseId ).size(); j++ ) // curves
            {
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
    textDialog->setMinimumSize( 400, 600 );
    textDialog->setWindowTitle( "Depth Plot Data" );
    textDialog->setText( outTxt );
    textDialog->show();
}
