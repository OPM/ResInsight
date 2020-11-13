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

#include "RimGridStatisticsPlot.h"

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimWellAllocationPlot.h"

// #include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
// #include "RiuPlotMainWindowTools.h"
// //#include "RiuQwtPlotWidget.h"
// #include "RiuWellLogPlot.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cvfAssert.h"

#include <QKeyEvent>

#include <QtCharts/QCategoryAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimGridStatisticsPlot, "GridStatisticsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlot::RimGridStatisticsPlot()
{
    CAF_PDM_InitObject( "Grid Statistics Plot", "", "", "A Plot of Grid Statistics" );

    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name", "", "", "" );
    m_plotWindowTitle.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitScriptableFieldNoDefault( &m_subTitleFontSize, "SubTitleFontSize", "Track Title Font Size", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size", "", "", "" );

    // m_plotLegendsHorizontal = false;
    // setPlotTitleVisible( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlot::~RimGridStatisticsPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridStatisticsPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridStatisticsPlot::createPlotWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    return createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridStatisticsPlot::description() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimGridStatisticsPlot::snapshotWindowContent()
{
    QImage image;

    // if ( m_viewer )
    // {
    //     QPixmap pix( m_viewer->size() );
    //     m_viewer->renderTo( &pix );
    //     image = pix.toImage();
    // }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridStatisticsPlot::createViewWidget( QWidget* mainWindowParent )
{
    std::cout << "Creating view widget!!" << std::endl;

    using namespace QtCharts;

    //
    QChart* chart = new QChart();
    chart->setTitle( "Multiaxis chart example" );

    QValueAxis* axisX = new QValueAxis;
    axisX->setTickCount( 10 );
    chart->addAxis( axisX, Qt::AlignBottom );

    QSplineSeries* series = new QSplineSeries;
    *series << QPointF( 1, 5 ) << QPointF( 3.5, 18 ) << QPointF( 4.8, 7.5 ) << QPointF( 10, 2.5 );
    chart->addSeries( series );

    QValueAxis* axisY = new QValueAxis;
    axisY->setLinePenColor( series->pen().color() );

    chart->addAxis( axisY, Qt::AlignLeft );
    series->attachAxis( axisX );
    series->attachAxis( axisY );

    series = new QSplineSeries;
    *series << QPointF( 1, 0.5 ) << QPointF( 1.5, 4.5 ) << QPointF( 2.4, 2.5 ) << QPointF( 4.3, 12.5 )
            << QPointF( 5.2, 3.5 ) << QPointF( 7.4, 16.5 ) << QPointF( 8.3, 7.5 ) << QPointF( 10, 17 );
    chart->addSeries( series );

    QCategoryAxis* axisY3 = new QCategoryAxis;
    axisY3->append( "Low", 5 );
    axisY3->append( "Medium", 12 );
    axisY3->append( "High", 17 );
    axisY3->setLinePenColor( series->pen().color() );
    axisY3->setGridLinePen( ( series->pen() ) );

    chart->addAxis( axisY3, Qt::AlignRight );
    series->attachAxis( axisX );
    series->attachAxis( axisY3 );

    m_viewer = new QChartView( chart );
    recreatePlotWidgets();
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::recreatePlotWidgets()
{
    // CVF_ASSERT( m_viewer );

    // auto plotVector = plots();

    // m_viewer->removeAllPlots();

    // for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    // {
    //     plotVector[tIdx]->createPlotWidget();
    //     m_viewer->addPlot( plotVector[tIdx]->viewer() );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::onPlotAdditionOrRemoval()
{
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    // if ( m_viewer )
    // {
    //     m_viewer->renderTo( paintDevice );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::doUpdateLayout()
{
    if ( m_viewer )
    {
        // m_viewer->setTitleFontSizes( titleFontSize(), subTitleFontSize() );
        // m_viewer->setLegendFontSize( legendFontSize() );
        // m_viewer->setAxisFontSizes( axisTitleFontSize(), axisValueFontSize() );
        // m_viewer->scheduleUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::cleanupBeforeClose()
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
void RimGridStatisticsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    // uiOrderingForAutoName( uiConfigName, *titleGroup );

    caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
    RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );
    plotLayoutGroup->add( &m_subTitleFontSize );
    plotLayoutGroup->add( &m_axisTitleFontSize );
    plotLayoutGroup->add( &m_axisValueFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGridStatisticsPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_subTitleFontSize || fieldNeedingOptions == &m_axisTitleFontSize ||
         fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }

    ( *useOptionsOnly ) = true;
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::initAfterRead()
{
    RimPlotWindow::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    //    performAutoNameUpdate();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::updatePlots()
{
    // if ( m_showWindow )
    // {
    //     for ( RimPlot* plot : plots() )
    //     {
    //         plot->loadDataAndUpdate();
    //     }
    //     this->updateZoom();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridStatisticsPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                            std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    // calculateAvailableDepthRange();
    // updateZoom();
    // RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    // mainPlotWindow->updateWellLogPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridStatisticsPlot::subTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_subTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridStatisticsPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridStatisticsPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}
