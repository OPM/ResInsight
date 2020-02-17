/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimAnalysisPlot.h"

#include "RiaColorTables.h"
#include "RimAnalysisPlotDataEntry.h"
#include "RiuGroupedBarChartBuilder.h"
#include "RiuSummaryQwtPlot.h"

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot_barchart.h"
#include "qwt_scale_draw.h"

#include <limits>
#include <map>

namespace caf
{
template <>
void caf::AppEnum<RimAnalysisPlot::SortGroupType>::setUp()
{
    addItem( RimAnalysisPlot::SUMMARY_ITEM, "SUMMARY_ITEM", "Summary Item" );
    addItem( RimAnalysisPlot::CASE, "CASE", "Case" );
    addItem( RimAnalysisPlot::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimAnalysisPlot::VALUE, "VALUE", "Value" );
    addItem( RimAnalysisPlot::ABS_VALUE, "ABS_VALUE", "abs(Value)" );
    addItem( RimAnalysisPlot::OTHER_VALUE, "OTHER_VALUE", "Other Value" );
    addItem( RimAnalysisPlot::ABS_OTHER_VALUE, "ABS_OTHER_VALUE", "abs(Other Value)" );
    addItem( RimAnalysisPlot::TIME_STEP, "TIME_STEP", "Time Step" );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimAnalysisPlot, "AnalysisPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::RimAnalysisPlot()
    : RimPlot()
{
    CAF_PDM_InitObject( "Analysis Plot", ":/Histogram16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Plot Title", "", "", "" );
    m_showPlotTitle.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title", "", "", "" );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_data, "AnalysisPlotData", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_sortGroupSortingOrder, "sortingOrder", "Sort Order", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_sortGroupsToGroup, "grouping", "Grouping", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_sortGroupForLegend, "groupForLegend", "Legend Using", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::~RimAnalysisPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnalysisPlot::showPlotTitle() const
{
    return m_showPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setShowPlotTitle( bool showTitle )
{
    m_showPlotTitle = showTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::detachAllCurves() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::reattachAllCurves() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateAxes() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimAnalysisPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAnalysisPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::asciiDataForPlotExport() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateLegend() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnalysisPlot::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnalysisPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                     int                         oldFontSize,
                                     int                         fontSize,
                                     bool                        forceChange /*= false */ )
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setAutoScaleXEnabled( bool enabled ) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setAutoScaleYEnabled( bool enabled ) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::zoomAll() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateZoomInQwt() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateZoomFromQwt() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimAnalysisPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onAxisSelected( int axis, bool toggle ) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateCaseNameHasChanged()
{
    // Todo
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAnalysisPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

        this->connect( m_plotWidget, SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        // updatePlotTitle();
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::doUpdateLayout() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::doRemoveFromCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimAnalysisPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnalysisPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ ) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_plotWidget )
    {
        RiuGroupedBarChartBuilder chartBuilder( Qt::Horizontal );

        chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R1", "", 0.4 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R2", "", 0.45 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 0.5 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 0.55 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R1", "", 0.7 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R2", "", 0.75 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 1.05 );
        chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 1.0 );

        chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
        chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
        chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
        chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );

        chartBuilder.addBarEntry( "T3", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
        chartBuilder.addBarEntry( "T3", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
        chartBuilder.addBarEntry( "T3", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
        chartBuilder.addBarEntry( "T3", "W2", "4", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
        chartBuilder.addBarEntry( "T3", "W2", "5", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );

        chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
        chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
        chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
        chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );
        chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.6 );
        chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.6 );
        chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.6 );
        chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", -0.3 );

        chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G1", 1.5 );
        chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G2", 1.5 );
        chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G3", 2.0 );
        chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G4", 2.0 );
        chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G5", 1.6 );
        chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G6", 1.6 );
        chartBuilder.addBarEntry( "T5", "", "", 2.6, "R3", "G7", 2.6 );
        chartBuilder.addBarEntry( "T5", "", "", -0.1, "R3", "G8", -0.1 );

        chartBuilder.addBarEntry( "", "", "", 1.2, "", "A", 1.2 );
        chartBuilder.addBarEntry( "", "", "", 1.5, "", "B", 1.5 );
        chartBuilder.addBarEntry( "", "", "", 2.3, "", "C", 2.3 );
        chartBuilder.addBarEntry( "", "", "", 2.0, "", "D", 2.0 );
        chartBuilder.addBarEntry( "", "", "", 1.6, "", "E", 1.6 );
        chartBuilder.addBarEntry( "", "", "", 2.4, "", "F", -2.4 );

        chartBuilder.addBarChartToPlot( m_plotWidget );

        if ( m_showPlotLegends && m_plotWidget->legend() == nullptr )
        {
            QwtLegend* legend = new QwtLegend( m_plotWidget );
            m_plotWidget->insertLegend( legend, QwtPlot::RightLegend );
        }
        else
        {
            m_plotWidget->insertLegend( nullptr );
        }

        // m_plotWidget->setLegendFontSize( m_legendFontSize() );
        m_plotWidget->updateLegend();
    }

    this->updateAxes();
}
