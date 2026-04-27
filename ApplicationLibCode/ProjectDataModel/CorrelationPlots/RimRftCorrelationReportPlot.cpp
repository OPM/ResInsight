/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimRftCorrelationReportPlot.h"

#include "RimParameterRftCrossPlot.h"
#include "RimRftTornadoPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellRftPlot.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotWidget.h"
#include "RiuQwtPlotWidget.h"

#include "DockAreaTitleBar.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"
#include <QContextMenuEvent>
#include <QFrame>
#include <QSettings>
#include <QVBoxLayout>

static const char* RFT_DOCK_LAYOUT_REGISTRY_KEY = "RftCorrelationReportPlot/defaultDockLayout";

//--------------------------------------------------------------------------------------------------
/// Thin wrapper that implements RiuInterfaceToViewWindow for the dock manager frame.
//--------------------------------------------------------------------------------------------------
class RiuRftCorrelationReportPlotWidget : public QFrame, public RiuInterfaceToViewWindow
{
public:
    RiuRftCorrelationReportPlotWidget( RimViewWindow* viewWindow, QWidget* parent = nullptr )
        : QFrame( parent )
        , m_viewWindow( viewWindow )
    {
        setLayout( new QVBoxLayout );
        layout()->setContentsMargins( 0, 0, 0, 0 );
        layout()->setSpacing( 0 );
    }

    RimViewWindow* ownerViewWindow() const override { return m_viewWindow; }

private:
    RimViewWindow* m_viewWindow;
};

//--------------------------------------------------------------------------------------------------
/// Sets the report plot as the CAF-selected item whenever a context menu fires
/// anywhere inside the dock manager.
//--------------------------------------------------------------------------------------------------
class RftSelectionFixerOnContextMenu : public QObject
{
public:
    RftSelectionFixerOnContextMenu( caf::PdmObject* item, QObject* parent )
        : QObject( parent )
        , m_item( item )
    {
    }

    bool eventFilter( QObject*, QEvent* event ) override
    {
        if ( event->type() == QEvent::ContextMenu ) caf::SelectionManager::instance()->setSelectedItem( m_item );
        return false;
    }

private:
    caf::PdmObject* m_item;
};

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimRftCorrelationReportPlot, "RftCorrelationReportPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftCorrelationReportPlot::RimRftCorrelationReportPlot()
{
    CAF_PDM_InitObject( "RFT Correlation Report Plot", ":/CorrelationReportPlot16x16.png" );
    setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "PlotWindowTitle", "Title" );
    m_name.registerGetMethod( this, &RimRftCorrelationReportPlot::createDescription );

    CAF_PDM_InitFieldNoDefault( &m_wellRftPlot, "WellRftPlot", "RFT Plot" );
    CAF_PDM_InitFieldNoDefault( &m_parameterRftCrossPlot, "ParameterRftCrossPlot", "Cross Plot" );
    CAF_PDM_InitFieldNoDefault( &m_correlationPlot, "CorrelationPlot", "Tornado Plot" );

    CAF_PDM_InitField( &m_showDockTitleBars, "ShowDockTitleBars", false, "Show Title Bars" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showDockTitleBars );

    CAF_PDM_InitField( &m_dockState, "DockState", QString(), "Dock State" );
    m_dockState.uiCapability()->setUiHidden( true );

    setAsPlotMdiWindow();

    m_showWindow      = true;
    m_showPlotLegends = false;

    m_wellRftPlot = new RimWellRftPlot;
    m_wellRftPlot->revokeMdiWindowStatus();
    m_wellRftPlot->setShowWindow( true );

    m_parameterRftCrossPlot = new RimParameterRftCrossPlot;

    m_correlationPlot = new RimRftTornadoPlot;
    m_correlationPlot->setParameterSelectedCallback( [this]( const QString& paramName ) { onTornadoParameterSelected( paramName ); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftCorrelationReportPlot::~RimRftCorrelationReportPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimRftCorrelationReportPlot::viewWidget()
{
    return m_viewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftCorrelationReportPlot::description() const
{
    return createDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimRftCorrelationReportPlot::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftPlot* RimRftCorrelationReportPlot::wellRftPlot() const
{
    return m_wellRftPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterRftCrossPlot* RimRftCorrelationReportPlot::crossPlot() const
{
    return m_parameterRftCrossPlot();
}

//--------------------------------------------------------------------------------------------------
/// Initialize the owned RimWellRftPlot from the source plot's selected data sources.
/// We keep the fresh (curve-free) child plot and call initializeDataSources(source) so
/// syncCurvesFromUiSelection builds curves from scratch without touching unresolved copies.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::initializeFromSourcePlot( RimWellRftPlot* source )
{
    if ( !source ) return;

    m_wellRftPlot->setSimWellOrWellPathName( source->simWellOrWellPathName() );

    // A fresh RimWellRftPlot has no tracks; syncCurvesFromUiSelection exits early without one.
    // Guard against duplicate track creation if this is called more than once.
    if ( m_wellRftPlot->plotCount() == 0 )
    {
        auto* track = new RimWellLogTrack();
        m_wellRftPlot->addPlot( track );
        track->setDescription( QString( "Track %1" ).arg( m_wellRftPlot->plotCount() ) );
    }

    m_wellRftPlot->initializeDataSources( source );

    // The correlation report operates on a single time step; trim any extras that
    // initializeDataSources may have preselected when only a few were available.
    auto selectedTimeSteps = m_wellRftPlot->selectedTimeSteps();
    if ( selectedTimeSteps.size() > 1 )
    {
        m_wellRftPlot->setSelectedTimeSteps( { selectedTimeSteps.front() } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftCorrelationReportPlot::createDescription() const
{
    if ( m_parameterRftCrossPlot() )
    {
        const QString wellName = m_parameterRftCrossPlot()->wellName();
        const QString param    = m_parameterRftCrossPlot()->ensembleParameter();
        if ( !wellName.isEmpty() && !param.isEmpty() )
        {
            return QString( "RFT Correlation: %1 vs %2" ).arg( param ).arg( wellName );
        }
    }
    return "RFT Correlation Report";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::recreatePlotWidgets()
{
    CAF_ASSERT( m_dockManager );

    m_wellRftPlot->createPlotWidget( m_dockManager );
    m_correlationPlot->createPlotWidget( m_dockManager );
    m_parameterRftCrossPlot->createPlotWidget( m_dockManager );

    // Context menu fixer — ensures this report is selected in CAF on any right-click
    delete m_contextMenuFilter;
    m_contextMenuFilter = new RftSelectionFixerOnContextMenu( this, this );
    if ( auto* w = m_wellRftPlot->viewWidget() ) w->installEventFilter( m_contextMenuFilter );
    if ( auto* w = m_correlationPlot->viewer() ) w->installEventFilter( m_contextMenuFilter );
    if ( auto* w = m_parameterRftCrossPlot->viewer() ) w->installEventFilter( m_contextMenuFilter );

    auto makeDockWidget = [&]( const QString& title, RimPlotWindow* plot, QWidget* widget ) -> ads::CDockWidget*
    {
        auto* dock = new ads::CDockWidget( title, m_dockManager );
        dock->setWidget( widget, ads::CDockWidget::ForceNoScrollArea );
        connect( dock,
                 &ads::CDockWidget::closed,
                 this,
                 [this, plot]()
                 {
                     plot->setShowWindow( false );
                     updateConnectedEditors();
                 } );
        return dock;
    };

    m_rftDockWidget         = makeDockWidget( "RFT Plot", m_wellRftPlot(), m_wellRftPlot->viewWidget() );
    m_correlationDockWidget = makeDockWidget( "Tornado Plot", m_correlationPlot(), m_correlationPlot->viewer() );
    m_crossPlotDockWidget   = makeDockWidget( "Cross Plot", m_parameterRftCrossPlot(), m_parameterRftCrossPlot->viewer() );

    // Restore saved dock state or apply hard-coded default layout
    QByteArray stateToRestore;
    if ( !m_dockState().isEmpty() )
    {
        stateToRestore = QByteArray::fromBase64( m_dockState().toLatin1() );
    }
    else
    {
        QSettings settings;
        QVariant  v = settings.value( RFT_DOCK_LAYOUT_REGISTRY_KEY );
        if ( v.isValid() ) stateToRestore = v.toByteArray();
    }

    if ( !stateToRestore.isEmpty() )
    {
        m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_rftDockWidget );
        auto* rightArea = m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_correlationDockWidget );
        m_dockManager->addDockWidget( ads::BottomDockWidgetArea, m_crossPlotDockWidget, rightArea );
        m_dockManager->restoreState( stateToRestore, 1 );
    }
    else
    {
        // Default: RFT plot on the left, tornado top-right, cross plot bottom-right
        m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_rftDockWidget );
        auto* rightArea = m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_correlationDockWidget );
        m_dockManager->addDockWidget( ads::BottomDockWidgetArea, m_crossPlotDockWidget, rightArea );
    }

    m_rftDockWidget->toggleView( m_wellRftPlot->showWindow() );
    m_correlationDockWidget->toggleView( m_correlationPlot->showWindow() );
    m_crossPlotDockWidget->toggleView( m_parameterRftCrossPlot->showWindow() );

    updateDockTitleBarsVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::cleanupBeforeClose()
{
    // Detach and delete legend curves before the QwtPlot inside the track widget is destroyed.
    // QwtPlot has autoDelete=true, so any curves still attached when it is deleted are freed by QWT
    // — leaving m_legendPlotCurves with dangling pointers on the next loadDataAndUpdate().
    if ( m_wellRftPlot() ) m_wellRftPlot->cleanupLegendCurves();
    if ( m_correlationPlot() ) m_correlationPlot->detachAllCurves();
    if ( m_parameterRftCrossPlot() ) m_parameterRftCrossPlot->detachAllCurves();

    m_rftDockWidget         = nullptr;
    m_correlationDockWidget = nullptr;
    m_crossPlotDockWidget   = nullptr;

    if ( m_dockManager )
    {
        delete m_dockManager;
        m_dockManager = nullptr;
    }

    if ( m_viewWidget )
    {
        m_viewWidget->setParent( nullptr );
        delete m_viewWidget;
        m_viewWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::setupBeforeSave()
{
    if ( m_dockManager )
    {
        m_dockState = QString::fromLatin1( m_dockManager->saveState( 1 ).toBase64() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewWidget ) m_viewWidget->render( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimRftCorrelationReportPlot::createViewWidget( QWidget* mainWindowParent )
{
    auto* wrapper = new RiuRftCorrelationReportPlotWidget( this, mainWindowParent );
    m_viewWidget  = wrapper;
    m_dockManager = new ads::CDockManager( wrapper );
    m_dockManager->setStyleSheet( "ads--CDockSplitter::handle { width: 2px; height: 2px; background-color: #a0a0a0; }" );
    wrapper->layout()->addWidget( m_dockManager );
    recreatePlotWidgets();
    return m_viewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_showWindow )
    {
        m_wellRftPlot->loadDataAndUpdate();
        syncTornadoInputsFromCrossPlot();
        m_correlationPlot->loadDataAndUpdate();
        m_parameterRftCrossPlot->loadDataAndUpdate();
    }

    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Delegate cross-plot settings (ensemble, well, depth range, parameter) to the cross plot
    m_parameterRftCrossPlot->uiOrdering( uiConfigName, uiOrdering );

    auto* layoutGroup = uiOrdering.addNewGroup( "Dock Layout" );
    layoutGroup->setCollapsedByDefault();
    layoutGroup->add( &m_showDockTitleBars );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString /*uiConfigName*/ )
{
    uiTreeOrdering.add( m_wellRftPlot() );
    uiTreeOrdering.add( m_correlationPlot() );
    uiTreeOrdering.add( m_parameterRftCrossPlot() );
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showDockTitleBars )
    {
        updateDockTitleBarsVisibility();
        return;
    }
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    if ( m_rftDockWidget && changedChildField == &m_wellRftPlot )
        m_rftDockWidget->toggleView( m_wellRftPlot->showWindow() );
    else if ( m_correlationDockWidget && changedChildField == &m_correlationPlot )
        m_correlationDockWidget->toggleView( m_correlationPlot->showWindow() );
    else if ( m_crossPlotDockWidget && changedChildField == &m_parameterRftCrossPlot )
    {
        m_crossPlotDockWidget->toggleView( m_parameterRftCrossPlot->showWindow() );
        syncCrossPlotSelectionToRftPlot();
    }

    updateDockTitleBarsVisibility();
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::syncCrossPlotSelectionToRftPlot()
{
    if ( !m_wellRftPlot() || !m_parameterRftCrossPlot() ) return;

    const QString   wellName = m_parameterRftCrossPlot->wellName();
    const QDateTime timeStep = m_parameterRftCrossPlot->selectedTimeStep();

    if ( !wellName.isEmpty() ) m_wellRftPlot->setSimWellOrWellPathName( wellName );

    if ( timeStep.isValid() ) m_wellRftPlot->setSelectedTimeSteps( { timeStep } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTornadoPlot* RimRftCorrelationReportPlot::correlationPlot() const
{
    return m_correlationPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimRftCorrelationReportPlot::childPlotsForTextExport() const
{
    return { crossPlot(), correlationPlot() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::onTornadoParameterSelected( const QString& paramName )
{
    if ( m_correlationPlot() )
    {
        m_correlationPlot->setSelectedParameter( paramName );
        m_correlationPlot->loadDataAndUpdate();
    }
    if ( m_parameterRftCrossPlot() )
    {
        m_parameterRftCrossPlot->setEnsembleParameter( paramName );
        m_parameterRftCrossPlot->loadDataAndUpdate();
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::syncTornadoInputsFromCrossPlot()
{
    if ( !m_correlationPlot() || !m_parameterRftCrossPlot() ) return;

    m_correlationPlot->setEnsemble( m_parameterRftCrossPlot->ensemble() );
    m_correlationPlot->setWellName( m_parameterRftCrossPlot->wellName() );
    m_correlationPlot->setTimeStep( m_parameterRftCrossPlot->selectedTimeStep() );
    m_correlationPlot->setSelectedParameter( m_parameterRftCrossPlot->ensembleParameter() );
    m_correlationPlot->setEclipseCase( m_parameterRftCrossPlot->eclipseCase() );
    m_correlationPlot->setUseDepthRange( m_parameterRftCrossPlot->useDepthRange() );
    m_correlationPlot->setDepthRange( m_parameterRftCrossPlot->depthRangeMin(), m_parameterRftCrossPlot->depthRangeMax() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::updateDockTitleBarsVisibility()
{
    if ( !m_dockManager ) return;
    for ( auto* area : m_dockManager->openedDockAreas() )
        area->titleBar()->setVisible( m_showDockTitleBars() );
}
