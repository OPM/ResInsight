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

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGridView.h"
#include "RimHistogramCalculator.h"
#include "RimHistogramData.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cvfAssert.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

#include <cmath>

using namespace QtCharts;

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

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case", "", "", "" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", -1, "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility", "", "", "" );

    // CAF_PDM_InitFieldNoDefault( &m_grouping, "Grouping", "Group Data by", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_property, "Property", "Property", "", "", "" );
    m_property = new RimEclipseResultDefinition( caf::PdmUiItemInfo::TOP );
    m_property.uiCapability()->setUiHidden( true );
    m_property.uiCapability()->setUiTreeChildrenHidden( true );
    m_property->setTernaryEnabled( false );

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
void RimGridStatisticsPlot::setDefaults()
{
    RimProject* project = RimProject::current();
    if ( project )
    {
        if ( !project->eclipseCases().empty() )
        {
            RimEclipseCase* eclipseCase = project->eclipseCases().front();
            m_case                      = eclipseCase;
            m_property->setEclipseCase( eclipseCase );

            m_property->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
            m_property->setResultVariable( "PORO" );

            // m_grouping = NO_GROUPING;
            // if ( eclipseCase->activeFormationNames() )
            // {
            //     m_grouping = GROUP_BY_FORMATION;
            //     m_groupingProperty->legendConfig()->setColorLegend(
            //         RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::CATEGORY ) );
            // }
        }
    }
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
QWidget* RimGridStatisticsPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new QChartView();
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

    if ( changedField == &m_case )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
        if ( eclipseCase )
        {
            m_property->setEclipseCase( eclipseCase );
            // m_groupingProperty->setEclipseCase( eclipseCase );
            // TODO: Do we need all these??
            m_property->updateConnectedEditors();
            // m_groupingProperty->updateConnectedEditors();

            // if ( m_grouping == GROUP_BY_FORMATION && !eclipseCase->activeFormationNames() )
            // {
            //     m_grouping = NO_GROUPING;
            // }

            // destroyCurves();
            loadDataAndUpdate();
        }
    }
    else if ( changedField == &m_timeStep )
    {
        // if ( m_timeStep != -1 && m_grouping == GROUP_BY_TIME )
        // {
        //     m_grouping = NO_GROUPING;
        // }

        // destroyCurves();
        loadDataAndUpdate();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    if ( m_case )
    {
        uiOrdering.add( &m_timeStep );
        uiOrdering.add( &m_cellFilterView );
        //        m_plotCellFilterCollection->setCase( eclipseCase );
        caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup( "Property" );
        m_property->uiOrdering( uiConfigName, *propertyGroup );
    }

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
    else if ( fieldNeedingOptions == &m_case )
    {
        RimTools::eclipseCaseOptionItems( &options );
        if ( options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        options.push_back( caf::PdmOptionItemInfo( "All Time Steps", -1 ) );

        RimTools::timeStepsForCase( m_case, &options );
    }
    else if ( fieldNeedingOptions == &m_cellFilterView )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );
            for ( RimEclipseView* view : eclipseCase->reservoirViews.childObjects() )
            {
                CVF_ASSERT( view && "Really always should have a valid view pointer in ReservoirViews" );
                options.push_back( caf::PdmOptionItemInfo( view->name(), view, false, view->uiIconProvider() ) );
            }
        }
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

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
    if ( eclipseCase )
    {
        m_property->setEclipseCase( eclipseCase );
        // m_groupingProperty->setEclipseCase( eclipseCase );
    }
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
    std::cout << "Update plots!" << std::endl;

    if ( m_viewer )
    {
        if ( m_cellFilterView.value() )
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( m_cellFilterView.value() );
            RimHistogramCalculator::StatisticsCellRangeType cellRange =
                RimHistogramCalculator::StatisticsCellRangeType::ALL_CELLS;
            RimHistogramCalculator::StatisticsTimeRangeType timeRange =
                RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;

            std::unique_ptr<RimHistogramCalculator> histogramCalculator;
            histogramCalculator.reset( new RimHistogramCalculator );

            RimHistogramData histogramData = histogramCalculator->histogramData( eclipseView, cellRange, timeRange );
            if ( histogramData.isHistogramVectorValid() )
            {
                QBarSet* set0     = new QBarSet( "data" );
                double   minValue = std::numeric_limits<double>::max();
                double   maxValue = std::numeric_limits<double>::min();
                for ( double value : *histogramData.histogram )
                {
                    *set0 << value;
                    minValue = std::min( minValue, value );
                    maxValue = std::max( maxValue, value );
                }

                QBarSeries* series = new QBarSeries();
                series->append( set0 );

                QChart* chart = new QChart();
                chart->addSeries( series );
                chart->setTitle( "Simple barchart example" );
                chart->setAnimationOptions( QChart::SeriesAnimations );

                // Axis
                double xAxisSize      = histogramData.max - histogramData.min;
                double xAxisExtension = xAxisSize * 0.02;

                QValueAxis* axisX = new QValueAxis();
                axisX->setRange( histogramData.min - xAxisExtension, histogramData.max + xAxisExtension );
                chart->addAxis( axisX, Qt::AlignBottom );

                QValueAxis* axisY = new QValueAxis();
                axisY->setRange( minValue, maxValue );
                chart->addAxis( axisY, Qt::AlignLeft );

                QLineSeries* p10series = new QLineSeries();
                chart->addSeries( p10series );
                p10series->setName( "P10" );
                p10series->append( histogramData.p10, minValue );
                p10series->append( histogramData.p10, maxValue );
                p10series->attachAxis( axisX );
                p10series->attachAxis( axisY );

                QLineSeries* p90series = new QLineSeries();
                chart->addSeries( p90series );
                p90series->setName( "P90" );
                p90series->append( histogramData.p90, minValue );
                p90series->append( histogramData.p90, maxValue );
                p90series->attachAxis( axisX );
                p90series->attachAxis( axisY );

                QLineSeries* meanSeries = new QLineSeries();
                chart->addSeries( meanSeries );
                meanSeries->setName( "Mean" );
                meanSeries->append( histogramData.mean, minValue );
                meanSeries->append( histogramData.mean, maxValue );
                meanSeries->attachAxis( axisX );
                meanSeries->attachAxis( axisY );

                m_viewer->setChart( chart );
            }
        }
    }
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
