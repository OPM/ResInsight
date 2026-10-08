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

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimDepthTrackPlot.h"
#include "RimParameterRftCrossPlot.h"
#include "RimRftCrossPlotTools.h"
#include "RimRftTornadoPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellRftEnsembleCurveSet.h"
#include "RimWellRftPlot.h"

#include "Formations/RimWellFormationsFile.h"
#include "Well/RigWellPathFormations.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotWidget.h"
#include "RiuQwtPlotWidget.h"

#include "DockAreaTitleBar.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"

#include "cafPdmOptionItemInfo.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"
#include "qwt_plot.h"
#include "qwt_plot_item.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"

#include <QContextMenuEvent>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <optional>

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

//--------------------------------------------------------------------------------------------------
/// Thin gray translucent bar drawn in pixel width along the left edge (vertical depth axis) or
/// bottom edge (horizontal depth axis) of the canvas, spanning a depth interval.
//--------------------------------------------------------------------------------------------------
class RftSelectedZoneBar : public QwtPlotItem
{
public:
    RftSelectedZoneBar( double top, double base, bool isDepthVertical )
        : m_top( top )
        , m_base( base )
        , m_isDepthVertical( isDepthVertical )
    {
        setZ( 5.0 );
    }

    void draw( QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const override
    {
        constexpr double barWidth = 6.0;

        QRectF bar;
        if ( m_isDepthVertical )
        {
            const double y0 = yMap.transform( m_top );
            const double y1 = yMap.transform( m_base );
            bar             = QRectF( canvasRect.left(), std::min( y0, y1 ), barWidth, std::abs( y1 - y0 ) );
        }
        else
        {
            const double x0 = xMap.transform( m_top );
            const double x1 = xMap.transform( m_base );
            bar             = QRectF( std::min( x0, x1 ), canvasRect.bottom() - barWidth, std::abs( x1 - x0 ), barWidth );
        }

        painter->fillRect( bar, QColor( 90, 90, 90, 110 ) );
    }

private:
    double m_top;
    double m_base;
    bool   m_isDepthVertical;
};

//--------------------------------------------------------------------------------------------------
/// Reports the depth value under a left-button click (not drag) on a depth track canvas.
//--------------------------------------------------------------------------------------------------
class RftTrackDepthClickFilter : public QObject
{
public:
    RftTrackDepthClickFilter( QwtPlot* plot, QwtAxisId depthAxis, bool isDepthVertical, std::function<void( double )> callback, QObject* parent )
        : QObject( parent )
        , m_plot( plot )
        , m_depthAxis( depthAxis )
        , m_isDepthVertical( isDepthVertical )
        , m_callback( std::move( callback ) )
    {
    }

    bool eventFilter( QObject*, QEvent* event ) override
    {
        if ( !m_plot ) return false;

        if ( event->type() == QEvent::MouseButtonPress )
        {
            auto* mouseEvent = static_cast<QMouseEvent*>( event );
            if ( mouseEvent->button() == Qt::LeftButton ) m_pressPos = mouseEvent->pos();
        }
        else if ( event->type() == QEvent::MouseButtonRelease )
        {
            auto* mouseEvent = static_cast<QMouseEvent*>( event );
            if ( mouseEvent->button() == Qt::LeftButton && m_pressPos && ( mouseEvent->pos() - *m_pressPos ).manhattanLength() < 4 )
            {
                const double pixel = m_isDepthVertical ? mouseEvent->pos().y() : mouseEvent->pos().x();
                const double depth = m_plot->invTransform( m_depthAxis, pixel );

                // Defer so the model and plot updates run after this mouse event is fully handled
                QTimer::singleShot( 0, this, [this, depth]() { m_callback( depth ); } );
            }
            m_pressPos.reset();
        }
        return false;
    }

private:
    QPointer<QwtPlot>             m_plot;
    QwtAxisId                     m_depthAxis;
    bool                          m_isDepthVertical;
    std::function<void( double )> m_callback;
    std::optional<QPoint>         m_pressPos;
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
    CAF_PDM_InitFieldNoDefault( &m_tornadoPlot, "TornadoPlot", "Tornado Plot" );

    CAF_PDM_InitField( &m_depthType, "DepthType", caf::AppEnum<RiaDefines::DepthType>( RiaDefines::DepthType::TRUE_VERTICAL_DEPTH ), "Depth Unit" );

    CAF_PDM_InitField( &m_showDockTitleBars, "ShowDockTitleBars", false, "Show Title Bars" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showDockTitleBars );

    CAF_PDM_InitField( &m_dockState, "DockState", QString(), "Dock State" );
    m_dockState.uiCapability()->setUiHidden( true );

    dockAsPlotWindow();

    m_showWindow      = true;
    m_showPlotLegends = false;

    m_wellRftPlot = new RimWellRftPlot;
    m_wellRftPlot->detachFromDockPermanently();
    m_wellRftPlot->setShowWindow( true );

    m_parameterRftCrossPlot = new RimParameterRftCrossPlot;

    m_tornadoPlot = new RimRftTornadoPlot;
    m_tornadoPlot->setParameterSelectedCallback( [this]( const QString& paramName ) { onTornadoParameterSelected( paramName ); } );

    applyDepthTypeToSubPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftCorrelationReportPlot::~RimRftCorrelationReportPlot()
{
    removeWindowFromDock();
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

    applyDepthTypeToSubPlots();

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
    m_tornadoPlot->createPlotWidget( m_dockManager );
    m_parameterRftCrossPlot->createPlotWidget( m_dockManager );

    // Context menu fixer — ensures this report is selected in CAF on any right-click
    delete m_contextMenuFilter;
    m_contextMenuFilter = new RftSelectionFixerOnContextMenu( this, this );
    if ( auto* w = m_wellRftPlot->viewWidget() ) w->installEventFilter( m_contextMenuFilter );
    if ( auto* w = m_tornadoPlot->viewer() ) w->installEventFilter( m_contextMenuFilter );
    if ( auto* w = m_parameterRftCrossPlot->viewer() ) w->installEventFilter( m_contextMenuFilter );

    installTrackClickFilters();

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
    m_correlationDockWidget = makeDockWidget( "Tornado Plot", m_tornadoPlot(), m_tornadoPlot->viewer() );
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
    m_correlationDockWidget->toggleView( m_tornadoPlot->showWindow() );
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
    if ( m_tornadoPlot() ) m_tornadoPlot->detachAllCurves();
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
/// Re-apply dock detachment after project load, since the PDM factory bypasses the constructor.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::initAfterRead()
{
    if ( m_wellRftPlot() )
    {
        m_wellRftPlot->detachFromDockPermanently();
        m_wellRftPlot->setShowWindow( true );
    }

    applyDepthTypeToSubPlots();
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
    updateDockWindowVisibility();

    if ( m_showWindow )
    {
        m_wellRftPlot->loadDataAndUpdate();
        installTrackClickFilters();
        updateSelectedZoneHighlight();
        syncZoneColorsToCrossPlot();
        syncTornadoInputsFromCrossPlot();
        m_tornadoPlot->loadDataAndUpdate();
        m_parameterRftCrossPlot->loadDataAndUpdate();
    }

    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_depthType );

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
    uiTreeOrdering.add( m_tornadoPlot() );
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
    if ( changedField == &m_depthType )
    {
        applyDepthTypeToSubPlots();
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
    else if ( m_correlationDockWidget && changedChildField == &m_tornadoPlot )
        m_correlationDockWidget->toggleView( m_tornadoPlot->showWindow() );
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
RimRftTornadoPlot* RimRftCorrelationReportPlot::tornadoPlot() const
{
    return m_tornadoPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimRftCorrelationReportPlot::childPlotsForTextExport() const
{
    return { crossPlot(), tornadoPlot() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::onTornadoParameterSelected( const QString& paramName )
{
    if ( m_tornadoPlot() )
    {
        m_tornadoPlot->setSelectedParameter( paramName );
        m_tornadoPlot->loadDataAndUpdate();
    }
    if ( m_parameterRftCrossPlot() )
    {
        m_parameterRftCrossPlot->setEnsembleParameter( paramName );
        m_parameterRftCrossPlot->loadDataAndUpdate();

        if ( m_wellRftPlot() )
        {
            auto* curveSet = m_wellRftPlot->findEnsembleCurveSet( m_parameterRftCrossPlot->ensemble() );
            if ( curveSet && curveSet->syncEnsembleParameter( paramName ) )
            {
                m_wellRftPlot->rebuildCurves();
                curveSet->updateConnectedEditors();
            }
        }
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::syncTornadoInputsFromCrossPlot()
{
    if ( !m_tornadoPlot() || !m_parameterRftCrossPlot() ) return;

    applyWellFormationsToSubPlots();

    m_tornadoPlot->setEnsemble( m_parameterRftCrossPlot->ensemble() );
    m_tornadoPlot->setWellName( m_parameterRftCrossPlot->wellName() );
    m_tornadoPlot->setTimeStep( m_parameterRftCrossPlot->selectedTimeStep() );
    m_tornadoPlot->setSelectedParameter( m_parameterRftCrossPlot->ensembleParameter() );
    m_tornadoPlot->setEclipseCase( m_parameterRftCrossPlot->eclipseCase() );
    m_tornadoPlot->setDepthRange( m_parameterRftCrossPlot->depthRangeMin(), m_parameterRftCrossPlot->depthRangeMax() );
    m_tornadoPlot->setDepthType( m_parameterRftCrossPlot->depthType() );
    m_tornadoPlot->setFilterMode( m_parameterRftCrossPlot->filterMode() );
    m_tornadoPlot->setSelectedZones( m_parameterRftCrossPlot->selectedZones() );
}

//--------------------------------------------------------------------------------------------------
/// Makes a click on a formation in the RFT plot tracks select that formation as the zone filter
/// used by the cross plot and tornado plot.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::installTrackClickFilters()
{
    if ( !m_wellRftPlot() ) return;

    if ( m_trackClickFilter ) m_trackClickFilter->deleteLater();
    m_trackClickFilter = new QObject( this );

    const auto orientation = m_wellRftPlot->depthOrientation();
    const bool isVertical  = orientation == RiaDefines::Orientation::VERTICAL;

    for ( size_t i = 0; i < m_wellRftPlot->plotCount(); ++i )
    {
        auto* track = dynamic_cast<RimWellLogTrack*>( m_wellRftPlot->plotByIndex( i ) );
        if ( !track ) continue;

        RiuQwtPlotWidget* widget = track->viewer();
        if ( !widget || !widget->qwtPlot() ) continue;

        auto* filter = new RftTrackDepthClickFilter(
            widget->qwtPlot(),
            widget->toQwtPlotAxis( RimDepthTrackPlot::depthAxis( orientation ) ),
            isVertical,
            [this]( double depth ) { onRftTrackDepthClicked( depth ); },
            m_trackClickFilter );
        widget->qwtPlot()->canvas()->installEventFilter( filter );
    }
}

//--------------------------------------------------------------------------------------------------
/// Marks the depth interval of each selected zone in the RFT plot tracks with a thin bar along the
/// depth axis side. Stale bars are found by type and removed before new ones are attached.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::updateSelectedZoneHighlight()
{
    if ( !m_wellRftPlot() || !m_parameterRftCrossPlot() ) return;

    const auto filterMode = m_parameterRftCrossPlot->filterMode();

    std::vector<RimRftCrossPlotTools::DepthInterval> intervals =
        RimRftCrossPlotTools::buildDepthIntervals( filterMode,
                                                   m_parameterRftCrossPlot->depthRangeMin(),
                                                   m_parameterRftCrossPlot->depthRangeMax(),
                                                   m_parameterRftCrossPlot->wellFormationsFile(),
                                                   m_parameterRftCrossPlot->wellName(),
                                                   m_parameterRftCrossPlot->selectedZones(),
                                                   m_depthType() );

    const bool isVertical = m_wellRftPlot->depthOrientation() == RiaDefines::Orientation::VERTICAL;

    for ( size_t i = 0; i < m_wellRftPlot->plotCount(); ++i )
    {
        auto* track = dynamic_cast<RimWellLogTrack*>( m_wellRftPlot->plotByIndex( i ) );
        if ( !track || !track->viewer() ) continue;

        QwtPlot* qwtPlot = track->viewer()->qwtPlot();
        if ( !qwtPlot ) continue;

        // itemList() is a live reference; copy it before detaching items
        const QwtPlotItemList items = qwtPlot->itemList();
        for ( QwtPlotItem* item : items )
        {
            if ( dynamic_cast<RftSelectedZoneBar*>( item ) ) item->detach();
        }

        for ( const auto& interval : intervals )
        {
            auto* bar = new RftSelectedZoneBar( interval.top, interval.base, isVertical );
            bar->attach( qwtPlot );
        }

        qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// Gives the cross plot the formation colors used by the RFT plot tracks, looked up by formation name.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::syncZoneColorsToCrossPlot()
{
    if ( !m_wellRftPlot() || !m_parameterRftCrossPlot() ) return;

    std::map<QString, QColor> zoneColors;
    for ( size_t i = 0; i < m_wellRftPlot->plotCount() && zoneColors.empty(); ++i )
    {
        auto* track = dynamic_cast<RimWellLogTrack*>( m_wellRftPlot->plotByIndex( i ) );
        if ( !track ) continue;

        for ( const auto& [name, color] : track->formationZoneColors() )
        {
            // The alpha carries the track's shading transparency, used for the observed pressure band
            zoneColors[name] = QColor( color.r(), color.g(), color.b(), track->formationShadingAlpha() );
        }
    }

    m_parameterRftCrossPlot->setZoneColors( zoneColors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::onRftTrackDepthClicked( double depth )
{
    if ( !m_parameterRftCrossPlot() ) return;

    RimWellFormationsFile* wellFormationsFile = m_parameterRftCrossPlot->wellFormationsFile();
    if ( !wellFormationsFile ) return;

    const RigWellPathFormations* formations = wellFormationsFile->formationsForWell( m_parameterRftCrossPlot->wellName() );
    if ( !formations ) return;

    const bool useTvd = m_depthType() == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH;
    for ( size_t i = 0; i < formations->formationCount(); ++i )
    {
        const RigWellPathFormation& formation = formations->formationAt( i );
        const double                top       = useTvd ? formation.tvdTop : formation.mdTop;
        const double                base      = useTvd ? formation.tvdBase : formation.mdBase;
        if ( depth < top || depth > base ) continue;

        std::vector<QString> zones = m_parameterRftCrossPlot->selectedZones();
        auto                 it    = std::find( zones.begin(), zones.end(), formation.formationName );
        if ( it != zones.end() )
            zones.erase( it ); // clicking a selected zone again deselects it
        else
            zones.push_back( formation.formationName );

        m_parameterRftCrossPlot->setFilterMode( RimRftCrossPlotTools::DepthFilterMode::ZONES );
        m_parameterRftCrossPlot->setSelectedZones( zones );
        loadDataAndUpdate();
        updateConnectedEditors();
        return;
    }
}

//--------------------------------------------------------------------------------------------------
/// Pushes the user's manual "Well Formations File" selection in the cross plot down to the sub
/// plots so their "Filter By: Zones" option becomes available.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::applyWellFormationsToSubPlots()
{
    if ( !m_parameterRftCrossPlot() ) return;

    RimWellFormationsFile* wellFormationsFile = m_parameterRftCrossPlot->wellFormationsFile();

    if ( m_tornadoPlot() ) m_tornadoPlot->setWellFormations( wellFormationsFile );
}

//--------------------------------------------------------------------------------------------------
/// Pushes the selected depth unit (MD/TVD) down to the sub plots that present a depth axis or
/// filter samples by depth.
//--------------------------------------------------------------------------------------------------
void RimRftCorrelationReportPlot::applyDepthTypeToSubPlots()
{
    if ( m_wellRftPlot() )
    {
        m_wellRftPlot->setAvailableDepthTypes( { RiaDefines::DepthType::MEASURED_DEPTH, RiaDefines::DepthType::TRUE_VERTICAL_DEPTH } );
        m_wellRftPlot->setDepthType( m_depthType() );
    }

    if ( m_parameterRftCrossPlot() ) m_parameterRftCrossPlot->setDepthType( m_depthType() );
    if ( m_tornadoPlot() ) m_tornadoPlot->setDepthType( m_depthType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftCorrelationReportPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_depthType )
    {
        using DepthAppEnum = caf::AppEnum<RiaDefines::DepthType>;
        for ( auto depthType : { RiaDefines::DepthType::MEASURED_DEPTH, RiaDefines::DepthType::TRUE_VERTICAL_DEPTH } )
        {
            options.push_back( caf::PdmOptionItemInfo( DepthAppEnum::uiText( depthType ), depthType ) );
        }
    }

    return options;
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
