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

#include "RiuSeismicHistogramPanel.h"

#include "Riu3dSelectionManager.h"
#include "RiuDockedQwtPlot.h"
#include "RiuGuiTheme.h"
#include "RiuPlotCurveSymbol.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtSymbol.h"

#include "RimSeismicData.h"
#include "RimSeismicSection.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPen>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSeismicHistogramPanel::RiuSeismicHistogramPanel( QWidget* parent )
    : QWidget( parent )
{
    m_qwtPlot = new RiuDockedQwtPlot( this );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget( m_qwtPlot );
    mainLayout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( mainLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSeismicHistogramPanel::~RiuSeismicHistogramPanel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSeismicHistogramPanel::setPlotData( QString title, std::vector<double> xvals, std::vector<double> yvals )
{
    m_qwtPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );

    RiuQwtPlotCurve* qwtCurve = new RiuQwtPlotCurve( nullptr, "Distribution" );

    qwtCurve->setSamplesFromXValuesAndYValues( xvals, yvals, false );

    qwtCurve->setStyle( QwtPlotCurve::Lines );

    Qt::PenStyle penStyle = Qt::SolidLine;

    QPen curvePen( QBrush(), 1, penStyle );
    curvePen.setColor( QColor( 0, 0, 0, 255 ) );
    qwtCurve->setPen( curvePen );

    RiuQwtSymbol* curveSymbol = new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
    curveSymbol->setBrush( Qt::NoBrush );
    qwtCurve->setSymbol( curveSymbol );

    qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
    qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );
    qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    qwtCurve->setYAxis( RiuPlotAxis::defaultLeft() );

    qwtCurve->attach( m_qwtPlot );

    RiuGuiTheme::styleQwtItem( qwtCurve );

    m_qwtPlot->setAxisScaleEngine( QwtAxis::YLeft, new QwtLinearScaleEngine );
    m_qwtPlot->setAxisAutoScale( QwtAxis::XBottom, true );
    m_qwtPlot->setAxisAutoScale( QwtAxis::YLeft, true );

    m_qwtPlot->setTitle( title );

    m_qwtPlot->setAxisTitle( QwtAxis::XBottom, "Value" );
    m_qwtPlot->setAxisTitle( QwtAxis::YLeft, "Count" );
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSeismicHistogramPanel::clearPlot()
{
    m_qwtPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSeismicHistogramPanel::applyFontSizes( bool replot )
{
    if ( m_qwtPlot ) m_qwtPlot->applyFontSizes( replot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSeismicHistogramPanel::showHistogram( caf::PdmObjectHandle* selectedObject )
{
    RimSeismicData* seisData = dynamic_cast<RimSeismicData*>( selectedObject );
    if ( seisData == nullptr )
    {
        RimSeismicSection* section = dynamic_cast<RimSeismicSection*>( selectedObject );
        if ( section != nullptr )
        {
            seisData = section->seismicData();
        }
    }

    if ( seisData == nullptr )
    {
        clearPlot();
        return;
    }

    setPlotData( seisData->userDescription(), seisData->histogramXvalues(), seisData->histogramYvalues() );
}
