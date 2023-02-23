/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimWellConnectivityTable.h"

#include "RiaPreferences.h"

#include "RimEclipseCaseTools.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"

#include "RiuMatrixPlotWidget.h"

#include "cvfScalarMapper.h"

CAF_PDM_SOURCE_INIT( RimWellConnectivityTable, "RimWellConnectivityTable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellConnectivityTable::RimWellConnectivityTable()
{
    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolution, "FlowDiagSolution", "Plot Type" );
    CAF_PDM_InitField( &m_timeStep, "PlotTimeStep", 0, "Time Step" );

    CAF_PDM_InitField( &m_rowCount, "RowCount", 5, "Number rows" );
    CAF_PDM_InitField( &m_colCount, "ColCount", 5, "Number columns" );

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisLabelFontSize, "AxisLabelFontSize", "Axis Label Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_valueLabelFontSize, "ValueLabelFontSize", "Value Label Font Size" );
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Large;
    m_axisLabelFontSize = caf::FontTools::RelativeSize::Medium;

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setAutomaticRanges( -1.0, 1.0, -1.0, 1.0 );
    m_legendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED ) );

    setLegendsVisible( true );
    setAsPlotMdiWindow();
    setShowWindow( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellConnectivityTable::~RimWellConnectivityTable()
{
    if ( isMdiWindow() ) removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::cleanupBeforeClose()
{
    if ( m_matrixPlotWidget )
    {
        m_matrixPlotWidget->qwtPlot()->detachItems();
        m_matrixPlotWidget->setParent( nullptr );
        delete m_matrixPlotWidget;
        m_matrixPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_rowCount || changedField == &m_colCount )
    {
        onLoadDataAndUpdate();
    }
    if ( changedField == &m_case )
    {
        if ( m_flowDiagSolution && m_case )
        {
            m_flowDiagSolution = m_case->defaultFlowDiagSolution();
        }
        else
        {
            m_flowDiagSolution = nullptr;
        }
    }
    else if ( changedField == &m_flowDiagSolution || changedField == &m_timeStep )
    {
        onLoadDataAndUpdate();
    }
    else if ( m_matrixPlotWidget && ( changedField == &m_titleFontSize || changedField == &m_legendFontSize ||
                                      changedField == &m_axisTitleFontSize || changedField == &m_axisLabelFontSize ) )
    {
        m_matrixPlotWidget->setPlotTitleFontSize( titleFontSize() );
        m_matrixPlotWidget->setLegendFontSize( legendFontSize() );
        m_matrixPlotWidget->setAxisTitleFontSize( axisTitleFontSize() );
        m_matrixPlotWidget->setAxisLabelFontSize( axisLabelFontSize() );
    }
    else if ( changedField == &m_valueLabelFontSize && m_matrixPlotWidget )
    {
        m_matrixPlotWidget->setValueFontSize( valueLabelFontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    RimViewWindow::childFieldChangedByUi( changedChildField );

    if ( changedChildField == &m_legendConfig )
    {
        if ( m_matrixPlotWidget )
        {
            m_matrixPlotWidget->setScalarMapper( m_legendConfig->scalarMapper() );
        }
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimViewWindow::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup& dataGroup = *uiOrdering.addNewGroup( "Plot Data" );
    dataGroup.add( &m_case );
    dataGroup.add( &m_flowDiagSolution );
    dataGroup.add( &m_timeStep );

    caf::PdmUiGroup* tableMatrixGroup = uiOrdering.addNewGroup( "Table Settings" );
    tableMatrixGroup->add( &m_rowCount );
    tableMatrixGroup->add( &m_colCount );
    tableMatrixGroup->add( &m_legendConfig );

    caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Fonts" );
    fontGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForFonts( uiConfigName, *fontGroup );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisLabelFontSize );
    fontGroup->add( &m_valueLabelFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_matrixPlotWidget->clearPlotData();

    std::vector<QString> columnHeaders;
    for ( int i = 0; i < m_colCount; ++i )
    {
        columnHeaders.push_back( QString( "%1" ).arg( i ) );
    }
    m_matrixPlotWidget->setColumnHeaders( columnHeaders );

    auto denominator = static_cast<double>( ( m_rowCount - 1 ) * ( m_colCount - 1 ) );
    denominator      = std::max( denominator, 1.0 );
    for ( int i = 0; i < m_rowCount; ++i )
    {
        std::vector<double> rowValues;
        for ( int j = 0; j < m_colCount; ++j )
        {
            auto value = i * j / denominator;
            rowValues.push_back( static_cast<double>( value ) );
        }
        m_matrixPlotWidget->setRowValues( QString( "%1" ).arg( i ), rowValues );
    }

    m_matrixPlotWidget->createPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellConnectivityTable::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() )
    {
        return options;
    }

    if ( fieldNeedingOptions == &m_case )
    {
        auto resultCases = RimEclipseCaseTools::eclipseResultCases();
        for ( RimEclipseResultCase* c : resultCases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }
    else if ( m_case && fieldNeedingOptions == &m_flowDiagSolution )
    {
        RimFlowDiagSolution* defaultFlowSolution = m_case->defaultFlowDiagSolution();
        options.push_back( caf::PdmOptionItemInfo( "Well Flow", nullptr ) );
        if ( defaultFlowSolution )
        {
            options.push_back( caf::PdmOptionItemInfo( "Allocation", defaultFlowSolution ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_case, &options );
        if ( options.size() == 0 )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", -1 ) );
        }
    }
    else if ( fieldNeedingOptions == &m_axisTitleFontSize || fieldNeedingOptions == &m_axisLabelFontSize ||
              fieldNeedingOptions == &m_valueLabelFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellConnectivityTable::viewWidget()
{
    return m_matrixPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellConnectivityTable::snapshotWindowContent()
{
    QImage image;

    if ( m_matrixPlotWidget )
    {
        QPixmap pix = m_matrixPlotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellConnectivityTable::createViewWidget( QWidget* mainWindowParent )
{
    m_matrixPlotWidget = new RiuMatrixPlotWidget( this, mainWindowParent );
    m_matrixPlotWidget->setScalarMapper( m_legendConfig->scalarMapper() );
    m_matrixPlotWidget->setPlotTitle( m_tableTitle );
    return m_matrixPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellConnectivityTable::description() const
{
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellConnectivityTable::doRenderWindowContent( QPaintDevice* paintDevice )
{
    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellConnectivityTable::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellConnectivityTable::axisLabelFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisLabelFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellConnectivityTable::valueLabelFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_valueLabelFontSize() );
}
