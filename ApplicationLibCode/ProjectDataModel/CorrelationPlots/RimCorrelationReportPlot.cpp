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

#include "RimCorrelationReportPlot.h"

#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"

#include "RimCorrelationMatrixPlot.h"
#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimParameterResultCrossPlot.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddressSelector.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuPlotWidget.h"
#include "RiuQwtPlotWidget.h"

#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"

#include "cafAssert.h"
#include "cafPdmUiTreeOrdering.h"

#include <QImage>
#include <QStringList>
#include <QVBoxLayout>

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimCorrelationReportPlot, "CorrelationReportPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationReportPlot::RimCorrelationReportPlot()
{
    CAF_PDM_InitObject( "Correlation Report Plot", ":/CorrelationReportPlot16x16.png" );
    setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "PlotWindowTitle", "Title" );
    m_name.registerGetMethod( this, &RimCorrelationReportPlot::createDescription );

    CAF_PDM_InitFieldNoDefault( &m_correlationMatrixPlot, "MatrixPlot", "Matrix Plot" );
    CAF_PDM_InitFieldNoDefault( &m_correlationPlot, "CorrelationPlot", "Correlation Plot" );
    CAF_PDM_InitFieldNoDefault( &m_parameterResultCrossPlot, "CrossPlot", "Cross Plot" );

    CAF_PDM_InitFieldNoDefault( &m_subTitleFontSize, "SubTitleFontSize", "Sub Plot Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_labelFontSize, "LabelFontSize", "Label Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );

    CAF_PDM_InitField( &m_showSummaryPlot, "ShowSummaryPlot", false, "Show Summary Plot" );
    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot" );
    CAF_PDM_InitFieldNoDefault( &m_summaryAddressSelector, "SummaryAddressSelector", "Summary Vector" );

    CAF_PDM_InitField( &m_dockState, "DockState", QString(), "Dock State" );
    m_dockState.uiCapability()->setUiHidden( true );

    setAsPlotMdiWindow();

    m_showWindow      = true;
    m_showPlotLegends = false;

    m_titleFontSize     = caf::FontTools::RelativeSize::XLarge;
    m_subTitleFontSize  = caf::FontTools::RelativeSize::Large;
    m_labelFontSize     = caf::FontTools::RelativeSize::Small;
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Small;
    m_axisValueFontSize = caf::FontTools::RelativeSize::Small;

    m_correlationMatrixPlot = new RimCorrelationMatrixPlot;
    m_correlationMatrixPlot->setLegendsVisible( false );

    m_correlationPlot = new RimCorrelationPlot;
    m_correlationPlot->setLegendsVisible( false );

    m_parameterResultCrossPlot = new RimParameterResultCrossPlot;
    m_parameterResultCrossPlot->setLegendsVisible( true );

    m_summaryPlot = new RimSummaryPlot();
    m_summaryPlot->setLegendsVisible( false );
    m_summaryPlot->ensembleCurveSetCollection()->addCurveSet( new RimEnsembleCurveSet() );

    m_summaryAddressSelector = new RimSummaryAddressSelector();
    m_summaryAddressSelector->setShowResampling( false );
    m_summaryAddressSelector->setShowAxis( false );
    m_summaryAddressSelector->addressChanged.connect( this, &RimCorrelationReportPlot::onAddressSelectorChanged );

    uiCapability()->setUiTreeChildrenHidden( true );

    m_correlationMatrixPlot->matrixCellSelected.connect( this, &RimCorrelationReportPlot::onDataSelection );
    m_correlationPlot->tornadoItemSelected.connect( this, &RimCorrelationReportPlot::onDataSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationReportPlot::~RimCorrelationReportPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationReportPlot::viewWidget()
{
    return m_dockManager;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationReportPlot::description() const
{
    return createDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimCorrelationReportPlot::snapshotWindowContent()
{
    if ( m_dockManager ) return m_dockManager->grab().toImage();
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCorrelationReportPlot::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot* RimCorrelationReportPlot::matrixPlot() const
{
    return m_correlationMatrixPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot* RimCorrelationReportPlot::correlationPlot() const
{
    return m_correlationPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RimCorrelationReportPlot::crossPlot() const
{
    return m_parameterResultCrossPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::subTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_subTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationReportPlot::createPlotWindowTitle() const
{
    return "Correlation Report for " + createDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationReportPlot::createDescription() const
{
    QStringList ensembles;
    for ( auto entry : m_correlationMatrixPlot->curveDefinitions() )
    {
        if ( entry.ensemble() )
        {
            ensembles.push_back( entry.ensemble()->name() );
        }
    }
    ensembles.removeDuplicates();
    QString ensembleNames = ensembles.join( ", " );
    QString timeStep      = m_correlationMatrixPlot->timeStepString();

    return QString( "%1 at %2" ).arg( ensembleNames ).arg( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::recreatePlotWidgets()
{
    CAF_ASSERT( m_dockManager );

    // Create plot widgets
    m_correlationMatrixPlot->createPlotWidget( m_dockManager );
    m_correlationPlot->createPlotWidget( m_dockManager );
    m_parameterResultCrossPlot->createPlotWidget( m_dockManager );
    m_summaryPlot->createPlotWidget( m_dockManager );

    // Wrap each plot in a dock widget
    m_matrixDockWidget = new ads::CDockWidget( "Matrix Plot", m_dockManager );
    m_matrixDockWidget->setWidget( m_correlationMatrixPlot->viewer(), ads::CDockWidget::ForceNoScrollArea );
    m_matrixDockWidget->setFeature( ads::CDockWidget::DockWidgetClosable, false );

    m_correlationDockWidget = new ads::CDockWidget( "Correlation Plot", m_dockManager );
    m_correlationDockWidget->setWidget( m_correlationPlot->viewer(), ads::CDockWidget::ForceNoScrollArea );
    m_correlationDockWidget->setFeature( ads::CDockWidget::DockWidgetClosable, false );

    m_crossPlotDockWidget = new ads::CDockWidget( "Cross Plot", m_dockManager );
    m_crossPlotDockWidget->setWidget( m_parameterResultCrossPlot->viewer(), ads::CDockWidget::ForceNoScrollArea );
    m_crossPlotDockWidget->setFeature( ads::CDockWidget::DockWidgetClosable, false );

    m_summaryDockWidget = new ads::CDockWidget( "Summary Plot", m_dockManager );
    m_summaryDockWidget->setWidget( m_summaryPlot->plotWidget(), ads::CDockWidget::ForceNoScrollArea );
    m_summaryDockWidget->setFeature( ads::CDockWidget::DockWidgetClosable, false );

    // Connect summary plot click → time step update
    auto* summaryWidget = dynamic_cast<RiuQwtPlotWidget*>( m_summaryPlot->plotWidget() );
    if ( summaryWidget )
    {
        connect( summaryWidget, &RiuQwtPlotWidget::plotMousePressedAt, this, &RimCorrelationReportPlot::onSummaryPlotMousePressed );
    }

    // Restore saved dock state if available; otherwise set up default layout
    if ( !m_dockState().isEmpty() )
    {
        // Add all widgets to the manager first (required before restoreState)
        m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_matrixDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_correlationDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_crossPlotDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_summaryDockWidget );
        m_dockManager->restoreState( QByteArray::fromBase64( m_dockState().toLatin1() ), 1 );
    }
    else
    {
        // Default layout: matrix on left, corr top-right, cross bottom-right, summary far right
        auto* matrixArea = m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_matrixDockWidget );
        auto* corrArea   = m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_correlationDockWidget );
        auto* crossArea  = m_dockManager->addDockWidget( ads::BottomDockWidgetArea, m_crossPlotDockWidget, corrArea );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_summaryDockWidget, corrArea );

        (void)matrixArea;
        (void)crossArea;

        m_summaryDockWidget->toggleView( m_showSummaryPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::cleanupBeforeClose()
{
    m_correlationMatrixPlot->detachAllCurves();
    m_correlationPlot->detachAllCurves();
    m_parameterResultCrossPlot->detachAllCurves();
    m_summaryPlot->detachAllCurves();

    // Dock widget pointers are owned by the dock manager; nullify them before deletion
    m_matrixDockWidget      = nullptr;
    m_correlationDockWidget = nullptr;
    m_crossPlotDockWidget   = nullptr;
    m_summaryDockWidget     = nullptr;

    if ( m_dockManager )
    {
        delete m_dockManager;
        m_dockManager = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::setupBeforeSave()
{
    if ( m_dockManager )
    {
        m_dockState = QString::fromLatin1( m_dockManager->saveState( 1 ).toBase64() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_dockManager )
    {
        m_dockManager->render( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationReportPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    m_dockManager = new ads::CDockManager( mainWindowParent );
    m_dockManager->setStyleSheet( "ads--CDockSplitter::handle { width: 1px; height: 1px; }" );
    recreatePlotWidgets();

    return m_dockManager;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_showWindow )
    {
        auto cases = m_parameterResultCrossPlot->summaryCasesExcludedByFilter();
        m_correlationMatrixPlot->setExcludedSummaryCases( cases );
        m_correlationPlot->setExcludedSummaryCases( cases );

        m_correlationMatrixPlot->setLabelFontSize( m_labelFontSize() );
        m_correlationMatrixPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_correlationMatrixPlot->setAxisValueFontSize( m_axisValueFontSize() );

        auto timeStep                 = m_correlationMatrixPlot->timeStep().toSecsSinceEpoch();
        bool showOnlyTopNCorrelations = m_correlationMatrixPlot->showTopNCorrelations();
        int  topNFilterCount          = m_correlationMatrixPlot->topNFilterCount();
        bool useCaseFilter            = m_correlationMatrixPlot->isCaseFilterEnabled();
        auto caseFilterDataSource     = m_correlationMatrixPlot->caseFilterDataSource();

        m_correlationPlot->setTimeStep( timeStep );
        m_correlationPlot->setShowOnlyTopNCorrelations( showOnlyTopNCorrelations );
        m_correlationPlot->setTopNFilterCount( topNFilterCount );
        m_correlationPlot->enableCaseFilter( useCaseFilter );
        m_correlationPlot->setCaseFilterDataSource( caseFilterDataSource );

        m_correlationPlot->setLabelFontSize( m_labelFontSize() );
        m_correlationPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_correlationPlot->setAxisValueFontSize( m_axisValueFontSize() );
        m_correlationPlot->setShowAbsoluteValues( m_correlationMatrixPlot->showAbsoluteValues() );
        m_correlationPlot->setSortByAbsoluteValues( m_correlationMatrixPlot->sortByAbsoluteValues() );

        m_parameterResultCrossPlot->setTimeStep( timeStep );
        m_parameterResultCrossPlot->enableCaseFilter( useCaseFilter );
        m_parameterResultCrossPlot->setCaseFilterDataSource( caseFilterDataSource );
        m_parameterResultCrossPlot->setLabelFontSize( m_labelFontSize() );
        m_parameterResultCrossPlot->setLegendFontSize( m_legendFontSize() );
        m_parameterResultCrossPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_parameterResultCrossPlot->setAxisValueFontSize( m_axisValueFontSize() );

        m_correlationMatrixPlot->loadDataAndUpdate();
        if ( m_correlationMatrixPlot->viewer() ) m_correlationMatrixPlot->viewer()->setPlotTitleEnabled( true );

        m_correlationPlot->loadDataAndUpdate();
        if ( m_correlationPlot->viewer() ) m_correlationPlot->viewer()->setPlotTitleEnabled( true );

        m_parameterResultCrossPlot->loadDataAndUpdate();
        if ( m_parameterResultCrossPlot->viewer() ) m_parameterResultCrossPlot->viewer()->setPlotTitleEnabled( true );

        if ( m_showSummaryPlot() )
        {
            if ( !m_summaryAddressSelector->ensemble() )
            {
                auto ensembles = m_correlationMatrixPlot->ensembles();
                if ( !ensembles.empty() ) m_summaryAddressSelector->setEnsemble( *ensembles.begin() );
            }

            auto curveSets = m_summaryPlot->ensembleCurveSetCollection()->curveSets();
            if ( !curveSets.empty() )
            {
                curveSets[0]->setSummaryEnsemble( m_summaryAddressSelector->ensemble() );
                curveSets[0]->setSummaryAddressY( m_summaryAddressSelector->summaryAddress() );
            }

            m_summaryPlot->loadDataAndUpdate();
            if ( m_summaryPlot->plotWidget() ) m_summaryPlot->plotWidget()->setPlotTitleEnabled( true );
        }
    }

    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_correlationMatrixPlot->uiOrdering( "report", uiOrdering );

    auto filterGroup = uiOrdering.addNewGroup( "Filter" );
    m_parameterResultCrossPlot->appendFilterFields( *filterGroup );

    auto summaryGroup = uiOrdering.addNewGroup( "Summary Plot" );
    summaryGroup->add( &m_showSummaryPlot );
    if ( m_showSummaryPlot() )
    {
        m_summaryAddressSelector->uiOrdering( "", *summaryGroup );
    }

    auto plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->setCollapsedByDefault();
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_subTitleFontSize );
    plotGroup->add( &m_labelFontSize );
    plotGroup->add( &m_legendFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );
    m_correlationMatrixPlot->legendConfig()->uiOrdering( "ColorsOnly", *plotGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showSummaryPlot && m_summaryDockWidget )
    {
        m_summaryDockWidget->toggleView( m_showSummaryPlot() );
    }
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::doUpdateLayout()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onDataSelection( const caf::SignalEmitter*                     emitter,
                                                std::pair<QString, RiaSummaryCurveDefinition> parameterAndCurveDef )
{
    auto paramName = parameterAndCurveDef.first;
    auto curveDef  = parameterAndCurveDef.second;

    m_correlationPlot->setCurveDefinitions( { curveDef } );
    m_correlationPlot->loadDataAndUpdate();
    m_parameterResultCrossPlot->setCurveDefinitions( { curveDef } );
    m_parameterResultCrossPlot->setEnsembleParameter( paramName );
    m_parameterResultCrossPlot->loadDataAndUpdate();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onSummaryPlotMousePressed( double xPlotCoordinate )
{
    auto clickedTime    = RiaTimeTTools::fromDouble( xPlotCoordinate );
    auto availableSteps = m_correlationMatrixPlot->allAvailableTimeSteps();
    if ( availableSteps.empty() ) return;

    auto it = std::min_element( availableSteps.begin(),
                                availableSteps.end(),
                                [clickedTime]( time_t a, time_t b ) { return std::abs( a - clickedTime ) < std::abs( b - clickedTime ); } );

    m_correlationMatrixPlot->setTimeStep( *it );
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onAddressSelectorChanged( const caf::SignalEmitter* )
{
    loadDataAndUpdate();
}
