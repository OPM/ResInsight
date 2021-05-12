/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStatisticsPlot.h"

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimPlot.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RiuPlotMainWindow.h"

#include "RigHistogramData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include "cvfAssert.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <cmath>

using namespace QtCharts;

CAF_PDM_ABSTRACT_SOURCE_INIT( RimStatisticsPlot, "StatisticsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsPlot::RimStatisticsPlot()
{
    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name", "", "", "" );
    m_plotWindowTitle.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_numHistogramBins, "NumHistogramBins", 50, "Number of Bins", "", "", "" );
    m_numHistogramBins.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    m_plotLegendsHorizontal.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsPlot::~RimStatisticsPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::createPlotWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    return createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsPlot::description() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimStatisticsPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix = m_viewer->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuQtChartView( this, mainWindowParent );
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::onPlotAdditionOrRemoval()
{
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        QPainter painter( paintDevice );
        m_viewer->render( &painter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::doUpdateLayout()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::cleanupBeforeClose()
{
    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
    if ( field == &m_numHistogramBins && lineEditorAttr != nullptr )
    {
        // Limit histogram bins to something resonable
        QIntValidator* validator  = new QIntValidator( 20, 1000, nullptr );
        lineEditorAttr->validator = validator;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::uiOrderingForHistogram( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* histogramGroup = uiOrdering.addNewGroup( "Histogram" );
    histogramGroup->add( &m_numHistogramBins );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    performAutoNameUpdate();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::updatePlots()
{
    if ( !hasStatisticsData() ) return;

    RigHistogramData histogramData = createStatisticsData();

    if ( !histogramData.isHistogramVectorValid() ) return;

    QBarSet* set0     = new QBarSet( m_plotWindowTitle );
    double   minValue = std::numeric_limits<double>::max();
    double   maxValue = -std::numeric_limits<double>::max();
    for ( double value : histogramData.histogram )
    {
        *set0 << value;
        minValue = std::min( minValue, value );
        maxValue = std::max( maxValue, value );
    }

    QBarSeries* series = new QBarSeries();
    series->append( set0 );

    QChart* chart = new QChart();
    chart->addSeries( series );
    chart->setTitle( uiName() );

    // Axis
    double xAxisSize      = histogramData.max - histogramData.min;
    double xAxisExtension = xAxisSize * 0.02;

    QValueAxis* axisX = new QValueAxis();
    axisX->setRange( histogramData.min - xAxisExtension, histogramData.max + xAxisExtension );
    chart->addAxis( axisX, Qt::AlignBottom );

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange( minValue, maxValue );
    chart->addAxis( axisY, Qt::AlignLeft );

    if ( !std::isinf( histogramData.p10 ) )
    {
        QLineSeries* p10series = new QLineSeries();
        chart->addSeries( p10series );
        p10series->setName( "P10" );
        p10series->append( histogramData.p10, minValue );
        p10series->append( histogramData.p10, maxValue );
        p10series->attachAxis( axisX );
        p10series->attachAxis( axisY );
    }

    if ( !std::isinf( histogramData.p10 ) )
    {
        QLineSeries* p90series = new QLineSeries();
        chart->addSeries( p90series );
        p90series->setName( "P90" );
        p90series->append( histogramData.p90, minValue );
        p90series->append( histogramData.p90, maxValue );
        p90series->attachAxis( axisX );
        p90series->attachAxis( axisY );
    }

    QLineSeries* meanSeries = new QLineSeries();
    chart->addSeries( meanSeries );
    meanSeries->setName( "Mean" );
    meanSeries->append( histogramData.mean, minValue );
    meanSeries->append( histogramData.mean, maxValue );
    meanSeries->attachAxis( axisX );
    meanSeries->attachAxis( axisY );

    // Set font sizes
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize( titleFontSize() );
    chart->setTitleFont( titleFont );

    QLegend* legend = chart->legend();
    if ( legend )
    {
        QFont legendFont = legend->font();
        legendFont.setPixelSize( legendFontSize() );
        legend->setFont( legendFont );
        legend->setVisible( legendsVisible() );
    }

    m_viewer->setChart( chart );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimStatisticsPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::performAutoNameUpdate()
{
    QString name      = createAutoName();
    m_plotWindowTitle = name;
    setUiName( name );
}
