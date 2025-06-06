/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuResultQwtPlot.h"

#include "RiaApplication.h"
#include "RiaCurveDataTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "RimCase.h"
#include "RimContextCommandBuilder.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotTools.h"
#include "RiuTextDialog.h"

#include "cafCmdFeatureMenuBuilder.h"

#include "cvfColor3.h"

#include "qwt_axis_id.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_scale_engine.h"
#include "qwt_text.h"

#include <QContextMenuEvent>
#include <QMenu>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot::RiuResultQwtPlot( QWidget* parent )
    : RiuDockedQwtPlot( parent )
    , m_timeMarker( nullptr )
{
    setAutoFillBackground( true );
    setDefaults();
    setAxisVisible( QwtAxis::XBottom, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot::~RiuResultQwtPlot()
{
    deleteAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::addCurve( const RimCase*                rimCase,
                                 const QString&                curveName,
                                 const cvf::Color3f&           curveColor,
                                 const std::vector<QDateTime>& dateTimes,
                                 const std::vector<double>&    timeHistoryValues )
{
    if ( dateTimes.empty() || timeHistoryValues.empty() )
    {
        return;
    }

    auto plotCurve = new RiuQwtPlotCurve( nullptr, "Curve 1" );

    plotCurve->setSamplesFromDatesAndYValues( dateTimes, timeHistoryValues, false );
    plotCurve->setTitle( curveName );

    plotCurve->setPen( QPen( QColor( curveColor.rByte(), curveColor.gByte(), curveColor.bByte() ) ) );

    plotCurve->attach( this );
    m_plotItems.push_back( plotCurve );

    setAxisScale( QwtAxis::XTop, QwtDate::toDouble( dateTimes.front() ), QwtDate::toDouble( dateTimes.back() ) );
    applyFontSizes( false );

    setAxisVisible( QwtAxis::XBottom, true );

    replot();

    int caseId = rimCase->caseId();

    m_caseNames[caseId] = rimCase->caseUserDescription();
    m_curveNames[caseId].push_back( curveName );
    m_curveData[caseId].push_back( timeHistoryValues );
    m_timeSteps[caseId] = dateTimes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::addCurve( const RimCase*             rimCase,
                                 const QString&             curveName,
                                 const cvf::Color3f&        curveColor,
                                 const std::vector<double>& frameTimes,
                                 const std::vector<double>& timeHistoryValues )
{
    std::vector<QDateTime> dateTimes;

    for ( double frameTime : frameTimes )
    {
        dateTimes.push_back( QwtDate::toDateTime( frameTime ) );
    }

    addCurve( rimCase, curveName, curveColor, dateTimes, timeHistoryValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::showTimeStep( const QDateTime& dateTime )
{
    if ( !dateTime.isValid() ) return;

    if ( m_timeMarker == nullptr )
    {
        m_timeMarker = new QwtPlotMarker();
        QPen pen;
        pen.setStyle( Qt::DashLine );
        m_timeMarker->setLinePen( pen );
        m_timeMarker->setLineStyle( QwtPlotMarker::VLine );
        m_timeMarker->setLabel( QString( "Time Step" ) );
        m_timeMarker->setLabelAlignment( Qt::AlignTop | Qt::AlignRight );
        m_timeMarker->setLabelOrientation( Qt::Vertical );
        m_timeMarker->attach( this );
    }

    m_timeMarker->setXValue( QwtDate::toDouble( dateTime ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::deleteAllCurves()
{
    for ( size_t i = 0; i < m_plotItems.size(); i++ )
    {
        m_plotItems[i]->detach();
        delete m_plotItems[i];
    }

    m_plotItems.clear();

    m_caseNames.clear();
    m_curveNames.clear();
    m_curveData.clear();
    m_timeSteps.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuResultQwtPlot::sizeHint() const
{
    return QSize( 100, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuResultQwtPlot::minimumSizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewGridTimeHistoryCurveFeature";

    const int curveCount = itemList( QwtPlotItem::Rtti_PlotCurve ).count();

    QAction* act = menu.addAction( "Show Plot Data", this, SLOT( slotCurrentPlotDataInTextDialog() ) );
    act->setEnabled( curveCount > 0 );

    menuBuilder.appendToMenu( &menu );

    if ( !menu.actions().empty() )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::setDefaults()
{
    RiuQwtPlotTools::setCommonPlotBehaviour( this );

    setAxesCount( QwtAxis::XBottom, 1 );
    setAxesCount( QwtAxis::YLeft, 1 );

    QString dateFormat = RiaPreferences::current()->dateFormat();
    QString timeFormat = RiaPreferences::current()->timeFormat();

    RiuQwtPlotTools::enableDateBasedBottomXAxis( this, dateFormat, timeFormat );

    setAxisMaxMinor( QwtAxis::XBottom, 2 );
    setAxisMaxMinor( QwtAxis::YLeft, 3 );

    applyFontSizes( false );

    // The legend will be deleted in the destructor of the plot or when
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend( this );
    insertLegend( legend, BottomLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultQwtPlot::asciiDataForUiSelectedCurves() const
{
    QString out;

    for ( std::pair<int, QString> caseIdAndName : m_caseNames )
    {
        int caseId = caseIdAndName.first;
        out += "Case: " + caseIdAndName.second;
        out += "\n";

        for ( size_t i = 0; i < m_timeSteps.at( caseId ).size(); i++ ) // time steps & data points
        {
            if ( i == 0 )
            {
                out += "Date and time";
                for ( QString curveName : m_curveNames.at( caseId ) )
                {
                    out += "\t" + curveName;
                }
            }
            out += "\n";

            QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale( m_timeSteps.at( caseId )[i], "yyyy-MM-dd hh:mm:ss " );

            out += dateString;

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
void RiuResultQwtPlot::slotCurrentPlotDataInTextDialog()
{
    QString outTxt = asciiDataForUiSelectedCurves();

    RiuTextDialog* textDialog = new RiuTextDialog( this );
    textDialog->setMinimumSize( 400, 600 );
    textDialog->setWindowTitle( "Result Plot Data" );
    textDialog->setText( outTxt );
    textDialog->show();
}
