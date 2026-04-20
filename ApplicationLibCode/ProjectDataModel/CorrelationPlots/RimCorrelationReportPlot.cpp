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
#include "RimEnsembleStatistics.h"
#include "RimParameterResultCrossPlot.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimTimeAxisAnnotation.h"

#include "RiuInterfaceToViewWindow.h"

#include "RimViewWindow.h"
#include "RiuPlotWidget.h"
#include "RiuQwtPlotWidget.h"

#include "DockAreaTitleBar.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"

#include "cafAssert.h"
#include "cafPdmPointer.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"

#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_picker.h"
#include "qwt_text.h"

#include <QContextMenuEvent>
#include <QFrame>
#include <QSettings>
#include <QStringList>
#include <QVBoxLayout>

static const char*        DOCK_LAYOUT_REGISTRY_KEY  = "CorrelationReportPlot/defaultDockLayout";
static const cvf::Color3f TRACKING_ANNOTATION_COLOR = cvf::Color3f( 0.6f, 0.4f, 0.08f );

//--------------------------------------------------------------------------------------------------
/// Thin wrapper around CDockManager that implements RiuInterfaceToViewWindow so that
/// activeViewWindow() / viewWindowFromWidget() can resolve the owning RimViewWindow.
//--------------------------------------------------------------------------------------------------
class RiuCorrelationReportPlotWidget : public QFrame, public RiuInterfaceToViewWindow
{
public:
    RiuCorrelationReportPlotWidget( RimViewWindow* viewWindow, QWidget* parent = nullptr )
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
/// Local picker for time tracking readout — draws a dashed vertical line following the mouse cursor.
/// Manages only its own annotation so the static selected-time annotation is preserved.
//--------------------------------------------------------------------------------------------------
class TimeReadoutPicker : public QwtPlotPicker
{
public:
    TimeReadoutPicker( RimSummaryPlot* summaryPlot, QwtPlot* plot )
        : QwtPlotPicker( plot->canvas() )
        , m_summaryPlot( summaryPlot )
    {
        setTrackerMode( QwtPlotPicker::AlwaysOn );
        plot->canvas()->setMouseTracking( true );
        setStateMachine( new QwtPickerTrackerMachine );
    }

    QwtText trackerText( const QPoint& screenPixelCoordinates ) const override
    {
        if ( !plot() || !m_summaryPlot ) return {};

        const auto timeTValue    = RiaTimeTTools::fromDouble( invTransform( screenPixelCoordinates ).x() );
        auto*      timeAxisProps = m_summaryPlot->timeAxisProperties();
        if ( timeAxisProps )
        {
            removeTrackingAnnotation( timeAxisProps );
            auto* anno = RimTimeAxisAnnotation::createTimeAnnotation( timeTValue, TRACKING_ANNOTATION_COLOR );
            anno->setPenStyle( Qt::DashLine );
            timeAxisProps->appendAnnotation( anno );
        }

        m_summaryPlot->updateAnnotationsInPlotWidget();
        m_summaryPlot->updatePlotWidgetFromAxisRanges();

        return {};
    }

    void widgetLeaveEvent( QEvent* ) override
    {
        if ( !m_summaryPlot ) return;

        auto* timeAxisProps = m_summaryPlot->timeAxisProperties();
        if ( timeAxisProps && removeTrackingAnnotation( timeAxisProps ) )
        {
            m_summaryPlot->updateAnnotationsInPlotWidget();
            m_summaryPlot->updatePlotWidgetFromAxisRanges();
        }
    }

private:
    static bool removeTrackingAnnotation( RimSummaryTimeAxisProperties* timeAxisProps )
    {
        if ( !timeAxisProps ) return false;
        for ( auto* anno : timeAxisProps->annotations() )
        {
            // Identify the tracking annotation by its pen style, which is unique among time axis annotations. This way we don't interfere
            // with the selected-time annotation, which is also on the time axis.
            if ( anno->penStyle() == Qt::DashLine )
            {
                timeAxisProps->removeAnnotation( dynamic_cast<RimTimeAxisAnnotation*>( anno ) );
                return true;
            }
        }
        return false;
    }

    caf::PdmPointer<RimSummaryPlot> m_summaryPlot;
};

namespace
{
// Ensures the correlation report plot is selected in CAF whenever a context menu event is dispatched
// anywhere within the dock manager — so feature availability checks in RiuContextMenuLauncher
// see the correct item (mirrors what RiuMultiPlotPage::contextMenuEvent did explicitly).
class SelectionFixerOnContextMenu : public QObject
{
public:
    SelectionFixerOnContextMenu( caf::PdmObject* item, QObject* parent )
        : QObject( parent )
        , m_item( item )
    {
    }

    bool eventFilter( QObject*, QEvent* event ) override
    {
        if ( event->type() == QEvent::ContextMenu ) caf::SelectionManager::instance()->setSelectedItem( m_item );
        return false; // never consume the event
    }

private:
    caf::PdmObject* m_item;
};
} // namespace

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

    CAF_PDM_InitField( &m_showDockTitleBars, "ShowDockTitleBars", false, "Show Title Bars" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showDockTitleBars );
    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot" );
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

    auto* curveSet = new RimEnsembleCurveSet();
    curveSet->setColorMode( RimEnsembleCurveSet::ColorMode::SINGLE_COLOR );
    curveSet->setColor( TRACKING_ANNOTATION_COLOR );
    curveSet->statisticsOptions()->enableCurveLabels( false );
    m_summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );

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
    return m_viewWidget;
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

    // Install selection fixer on each plot widget.
    // Filter runs first (LIFO order) so RiuContextMenuLauncher sees the correct CAF selection.
    delete m_contextMenuFilter;
    m_contextMenuFilter = new SelectionFixerOnContextMenu( this, this );
    for ( RiuPlotWidget* w : std::initializer_list<RiuPlotWidget*>{ m_correlationMatrixPlot->viewer(),
                                                                    m_correlationPlot->viewer(),
                                                                    m_parameterResultCrossPlot->viewer(),
                                                                    m_summaryPlot->plotWidget() } )
    {
        if ( w ) w->installEventFilter( m_contextMenuFilter );
    }

    // Wrap each plot in a dock widget
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

    m_matrixDockWidget      = makeDockWidget( "Matrix Plot", m_correlationMatrixPlot(), m_correlationMatrixPlot->viewer() );
    m_correlationDockWidget = makeDockWidget( "Correlation Plot", m_correlationPlot(), m_correlationPlot->viewer() );
    m_crossPlotDockWidget   = makeDockWidget( "Cross Plot", m_parameterResultCrossPlot(), m_parameterResultCrossPlot->viewer() );
    m_summaryDockWidget     = makeDockWidget( "Summary Plot", m_summaryPlot(), m_summaryPlot->plotWidget() );

    // Disable zoom — clicks are used for time step selection, not zooming
    if ( m_summaryPlot->summaryPlotWidget() ) m_summaryPlot->summaryPlotWidget()->setZoomEnabled( false );

    // Connect summary plot click → time step update; add time tracking readout
    auto* summaryWidget = dynamic_cast<RiuQwtPlotWidget*>( m_summaryPlot->plotWidget() );
    if ( summaryWidget )
    {
        connect( summaryWidget, &RiuQwtPlotWidget::plotMousePressedAt, this, &RimCorrelationReportPlot::onSummaryPlotMousePressed );
        new TimeReadoutPicker( m_summaryPlot(), summaryWidget->qwtPlot() );
    }

    // Restore saved dock state: project state takes priority, then registry default, then hard-coded layout
    QByteArray stateToRestore;
    if ( !m_dockState().isEmpty() )
        stateToRestore = QByteArray::fromBase64( m_dockState().toLatin1() );
    else
    {
        QSettings settings;
        QVariant  v = settings.value( DOCK_LAYOUT_REGISTRY_KEY );
        if ( v.isValid() ) stateToRestore = v.toByteArray();
    }

    if ( !stateToRestore.isEmpty() )
    {
        // Add all widgets to the manager first (required before restoreState)
        m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_matrixDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_correlationDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_crossPlotDockWidget );
        m_dockManager->addDockWidget( ads::RightDockWidgetArea, m_summaryDockWidget );
        m_dockManager->restoreState( stateToRestore, 1 );
    }
    else
    {
        // Hard-coded default: matrix left (full height), corr top-right, cross bottom-right,
        // summary spanning full width at bottom
        auto* corrArea = m_dockManager->addDockWidget( ads::CenterDockWidgetArea, m_correlationDockWidget );
        m_dockManager->addDockWidget( ads::LeftDockWidgetArea, m_matrixDockWidget );
        m_dockManager->addDockWidget( ads::BottomDockWidgetArea, m_crossPlotDockWidget, corrArea );
        m_dockManager->addDockWidget( ads::BottomDockWidgetArea, m_summaryDockWidget );
    }

    // Sync initial dock visibility from subplot enabled state
    m_matrixDockWidget->toggleView( m_correlationMatrixPlot->showWindow() );
    m_correlationDockWidget->toggleView( m_correlationPlot->showWindow() );
    m_crossPlotDockWidget->toggleView( m_parameterResultCrossPlot->showWindow() );
    m_summaryDockWidget->toggleView( m_summaryPlot()->showWindow() );

    updateDockTitleBarsVisibility();
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
    if ( m_viewWidget )
    {
        m_viewWidget->render( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationReportPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    auto* wrapper = new RiuCorrelationReportPlotWidget( this, mainWindowParent );
    m_viewWidget  = wrapper;
    m_dockManager = new ads::CDockManager( wrapper );
    m_dockManager->setStyleSheet( "ads--CDockSplitter::handle { width: 1px; height: 1px; }" );
    wrapper->layout()->addWidget( m_dockManager );
    recreatePlotWidgets();

    return m_viewWidget;
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
        m_correlationPlot->loadDataAndUpdate();
        m_parameterResultCrossPlot->loadDataAndUpdate();

        if ( m_summaryPlot->showWindow() )
        {
            auto curveSets = m_summaryPlot->ensembleCurveSetCollection()->curveSets();
            if ( !curveSets.empty() && !curveSets[0]->summaryEnsemble() )
            {
                auto curveDefs = m_correlationMatrixPlot->curveDefinitions();
                if ( !curveDefs.empty() )
                {
                    curveSets[0]->setSummaryEnsemble( curveDefs.front().ensemble() );
                    curveSets[0]->setSummaryAddressY( curveDefs.front().summaryAddressY() );
                }
            }

            m_summaryPlot->loadDataAndUpdate();
            if ( m_summaryPlot->plotWidget() ) m_summaryPlot->plotWidget()->setPlotTitleEnabled( true );

            // Add static line for the currently selected time step.
            // Must be done after loadDataAndUpdate() since that clears all annotations internally.
            auto* timeAxisProps = m_summaryPlot->timeAxisProperties();
            if ( timeAxisProps )
            {
                timeAxisProps->appendAnnotation( RimTimeAxisAnnotation::createTimeAnnotation( timeStep, TRACKING_ANNOTATION_COLOR ) );
                m_summaryPlot->updateAnnotationsInPlotWidget();
            }
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

    auto plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->setCollapsedByDefault();
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_subTitleFontSize );
    plotGroup->add( &m_labelFontSize );
    plotGroup->add( &m_legendFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );
    m_correlationMatrixPlot->legendConfig()->uiOrdering( "ColorsOnly", *plotGroup );

    auto layoutGroup = uiOrdering.addNewGroup( "Dock Layout" );
    layoutGroup->setCollapsedByDefault();
    layoutGroup->add( &m_showDockTitleBars );
    layoutGroup->addNewButton( "Save as Default Layout", [this]() { onSaveDefaultDockLayout(); } );
    layoutGroup->addNewButton( "Restore Default Layout", [this]() { onRestoreDefaultDockLayout(); }, { .newRow = false } );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::updateDockTitleBarsVisibility()
{
    if ( !m_dockManager ) return;
    for ( auto* area : m_dockManager->openedDockAreas() )
        area->titleBar()->setVisible( m_showDockTitleBars() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( m_correlationMatrixPlot() );
    uiTreeOrdering.add( m_correlationPlot() );
    uiTreeOrdering.add( m_parameterResultCrossPlot() );
    uiTreeOrdering.add( m_summaryPlot() );
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
void RimCorrelationReportPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    // childFieldChangedByUi receives the parent field (the PdmChildField in this class),
    // not the specific field that changed inside the child. Sync dock visibility unconditionally
    // so toggling showWindow() in the tree view is always reflected immediately.
    if ( m_matrixDockWidget && changedChildField == &m_correlationMatrixPlot )
        m_matrixDockWidget->toggleView( m_correlationMatrixPlot->showWindow() );
    else if ( m_correlationDockWidget && changedChildField == &m_correlationPlot )
        m_correlationDockWidget->toggleView( m_correlationPlot->showWindow() );
    else if ( m_crossPlotDockWidget && changedChildField == &m_parameterResultCrossPlot )
        m_crossPlotDockWidget->toggleView( m_parameterResultCrossPlot->showWindow() );
    else if ( m_summaryDockWidget && changedChildField == &m_summaryPlot )
        m_summaryDockWidget->toggleView( m_summaryPlot->showWindow() );

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
    m_correlationPlot->setSelectedParameter( paramName );
    m_correlationPlot->loadDataAndUpdate();
    m_parameterResultCrossPlot->setCurveDefinitions( { curveDef } );
    m_parameterResultCrossPlot->setEnsembleParameter( paramName );
    m_parameterResultCrossPlot->loadDataAndUpdate();

    if ( m_summaryPlot->showWindow() && curveDef.ensemble() )
    {
        auto curveSets = m_summaryPlot->ensembleCurveSetCollection()->curveSets();
        if ( !curveSets.empty() )
        {
            curveSets[0]->setSummaryEnsemble( curveDef.ensemble() );
            curveSets[0]->setSummaryAddressY( curveDef.summaryAddressY() );
        }
        m_summaryPlot->loadDataAndUpdate();
        if ( m_summaryPlot->plotWidget() ) m_summaryPlot->plotWidget()->setPlotTitleEnabled( true );
    }

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
void RimCorrelationReportPlot::onSaveDefaultDockLayout()
{
    if ( m_dockManager )
    {
        QSettings settings;
        settings.setValue( DOCK_LAYOUT_REGISTRY_KEY, m_dockManager->saveState( 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onRestoreDefaultDockLayout()
{
    if ( !m_dockManager ) return;

    QSettings settings;
    QVariant  v = settings.value( DOCK_LAYOUT_REGISTRY_KEY );
    if ( v.isValid() ) m_dockManager->restoreState( v.toByteArray(), 1 );
}
