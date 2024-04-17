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

#include "RiuMultiPlotPage.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlot.h"
#include "RimPlotCurve.h"
#include "RimPlotWindow.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuGuiTheme.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotObjectPicker.h"
#include "RiuPlotWidget.h"
#include "RiuQwtLegendOverlayContentFrame.h"
#include "RiuQwtPlotLegend.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"
#ifdef USE_QTCHARTS
#include "RiuQtChartsPlotWidget.h"
#endif

#include "cafCmdFeatureMenuBuilder.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "qwt_axis.h"
#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"

#include <QDebug>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QWidget>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage::RiuMultiPlotPage( RimPlotWindow* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_previewMode( false )
    , m_showSubTitles( false )
    , m_autoAlignAxes( true )
    , m_titleFontPixelSize( 12 )
    , m_subTitleFontPixelSize( 11 )
    , m_legendFontPixelSize( 8 )
    , m_axisTitleFontSize( 8 )
    , m_axisValueFontSize( 8 )

{
    CAF_ASSERT( m_plotDefinition );

    m_layout = new QVBoxLayout( this );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_layout->setSpacing( 4 );

    m_plotTitle = createTitleLabel();
    m_layout->addWidget( m_plotTitle );
    m_plotTitle->setVisible( true );

    m_plotLayout = new QHBoxLayout;
    m_layout->addLayout( m_plotLayout );

    m_plotWidgetFrame = new QFrame;
    m_plotLayout->addWidget( m_plotWidgetFrame, 1 );
    m_plotWidgetFrame->setVisible( true );

    m_gridLayout = new QGridLayout( m_plotWidgetFrame );
    m_gridLayout->setContentsMargins( 0, 0, 0, 0 );
    m_gridLayout->setSpacing( 0 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    setFocusPolicy( Qt::StrongFocus );
    setAcceptDrops( true );

    setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );

    applyLook();

    updateMarginsFromPageLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage::~RiuMultiPlotPage()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuMultiPlotPage::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow* RiuMultiPlotPage::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::addPlot( RiuPlotWidget* plotWidget )
{
    // Insert the plot to the left of the scroll bar
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::insertPlot( RiuPlotWidget* plotWidget, size_t index )
{
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    plotWidget->setVisible( false );

    QString subTitleText = plotWidget->plotTitle();
    QLabel* subTitle     = new QLabel( subTitleText );
    subTitle->setAlignment( Qt::AlignHCenter | Qt::AlignBottom );
    subTitle->setWordWrap( true );
    subTitle->setVisible( false );
    m_subTitles.insert( static_cast<int>( index ), subTitle );

    // Remove legend overlays already attached to the plot widget
    auto matcher = []( RiuDraggableOverlayFrame* p )
    { return dynamic_cast<RiuQwtLegendOverlayContentFrame*>( p->contentFrame() ) != nullptr; };
    plotWidget->clearOverlayFrames( matcher );

    RiuQwtPlotWidget* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget );

    RiuQwtPlotLegend*         legend      = nullptr;
    RiuDraggableOverlayFrame* legendFrame = new RiuDraggableOverlayFrame( plotWidget->getParentForOverlay(), plotWidget->overlayMargins() );

    if ( m_plotDefinition->legendsVisible() && plotWidget->plotDefinition()->legendsVisible() )
    {
        legend            = new RiuQwtPlotLegend( this );
        int legendColumns = 1;
        if ( m_plotDefinition->legendsHorizontal() )
        {
            legendColumns = 0; // unlimited
        }
        legend->setMaxColumns( legendColumns );
        legend->horizontalScrollBar()->setVisible( false );
        legend->verticalScrollBar()->setVisible( false );

        // The legend item mode must be set before the widget is created
        // See https://qwt.sourceforge.io/class_qwt_legend.html#af977ff3e749f8281ee8ad4b926542b50
        auto legendItemMode = m_plotDefinition->legendItemsClickable() ? QwtLegendData::Clickable : QwtLegendData::ReadOnly;
        legend->setDefaultItemMode( legendItemMode );

        if ( qwtPlotWidget )
        {
            legend->connect( qwtPlotWidget->qwtPlot(),
                             SIGNAL( legendDataChanged( const QVariant&, const QList<QwtLegendData>& ) ),
                             SLOT( updateLegend( const QVariant&, const QList<QwtLegendData>& ) ) );
            qwtPlotWidget->connect( legend, SIGNAL( clicked( const QVariant&, int ) ), SLOT( onLegendClicked( const QVariant&, int ) ) );
        }
        else
        {
#ifdef USE_QTCHARTS
            auto qtchartPlotWidget = dynamic_cast<RiuQtChartsPlotWidget*>( plotWidget );
            legend->connect( qtchartPlotWidget,
                             SIGNAL( legendDataChanged( const QList<QwtLegendData>& ) ),
                             SLOT( updateLegend( const QList<QwtLegendData>& ) ) );
#endif
        }

        QObject::connect( legend, SIGNAL( legendUpdated() ), this, SLOT( onLegendUpdated() ) );

        legend->contentsWidget()->layout()->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
        legend->setVisible( false );

        legendFrame->setVisible( false );

        plotWidget->updateLegend();
    }
    m_legends.insert( static_cast<int>( index ), legend );
    m_legendFrames.insert( static_cast<int>( index ), legendFrame );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removePlot( RiuPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    int plotWidgetIdx = m_plotWidgets.indexOf( plotWidget );
    if ( plotWidgetIdx < 0 ) return;

    m_plotWidgets.removeAt( plotWidgetIdx );
    plotWidget->setParent( nullptr );

    RiuQwtPlotLegend* legend = m_legends[plotWidgetIdx];
    if ( legend )
    {
        legend->setParent( nullptr );
        delete legend;
    }
    m_legends.removeAt( plotWidgetIdx );

    QLabel* subTitle = m_subTitles[plotWidgetIdx];
    subTitle->setParent( nullptr );
    m_subTitles.removeAt( plotWidgetIdx );
    delete subTitle;

    m_childCountForAdjustSizeOperation.clear();

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removeAllPlots()
{
    auto plotWidgets = m_plotWidgets;
    for ( RiuPlotWidget* plotWidget : plotWidgets )
    {
        removePlot( plotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle->setText( plotTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setTitleFontSizes( int titleFontSize, int subTitleFontSize )
{
    m_titleFontPixelSize    = caf::FontTools::pointSizeToPixelSize( titleFontSize );
    m_subTitleFontPixelSize = caf::FontTools::pointSizeToPixelSize( subTitleFontSize );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setLegendFontSize( int legendFontSize )
{
    m_legendFontPixelSize = caf::FontTools::pointSizeToPixelSize( legendFontSize );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize )
{
    m_axisTitleFontSize = axisTitleFontSize;
    m_axisValueFontSize = axisValueFontSize;

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setSubTitlesVisible( bool visible )
{
    m_showSubTitles = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::previewModeEnabled() const
{
    return m_previewMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setPagePreviewModeEnabled( bool previewMode )
{
    m_previewMode = previewMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::indexOfPlotWidget( RiuPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate )
{
    CAF_ASSERT( m_plotDefinition );
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotPageUpdate( this, whatToUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleReplotOfAllPlots()
{
    for ( RiuPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateSubTitles()
{
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            m_subTitles[i]->setText( m_plotWidgets[i]->plotTitle() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::renderTo( QPaintDevice* paintDevice )
{
    auto scaling = RiaDefines::scalingFactor( paintDevice );

    QPainter painter( paintDevice );
    renderTo( &painter, scaling );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::renderTo( QPainter* painter, double scalingFactor )
{
    painter->fillRect( painter->viewport(), Qt::white );
    m_plotTitle->render( painter );

    // Subtract margins because we are rendering into part inside the margins
    QPoint marginOffset( m_layout->contentsMargins().left(), m_layout->contentsMargins().top() );

    for ( auto subTitle : subTitlesForVisiblePlots() )
    {
        if ( subTitle->isVisible() )
        {
            QPoint renderOffset = m_plotWidgetFrame->mapToParent( subTitle->frameGeometry().topLeft() ) - marginOffset;
            subTitle->render( painter, renderOffset );
        }
    }

    for ( auto legend : legendsForVisiblePlots() )
    {
        if ( legend )
        {
            QPoint renderOffset = m_plotWidgetFrame->mapToParent( legend->frameGeometry().topLeft() ) - marginOffset;
            legend->render( painter, renderOffset );
        }
    }

    for ( auto plotWidget : visiblePlotWidgets() )
    {
        QRect  plotWidgetGeometry     = plotWidget->frameGeometry();
        QPoint plotWidgetTopLeft      = plotWidgetGeometry.topLeft();
        QPoint plotWidgetFrameTopLeft = m_plotWidgetFrame->frameGeometry().topLeft();
        plotWidgetGeometry.moveTo( plotWidgetTopLeft + plotWidgetFrameTopLeft - marginOffset );

        plotWidget->renderTo( painter, plotWidgetGeometry, scalingFactor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem( ownerPlotDefinition() );

    menuBuilder << "RicShowPlotDataFeature";
    menuBuilder << "RicShowContributingWellsFromPlotFeature";
    menuBuilder << "RicNewDefaultSummaryPlotFeature";

    menuBuilder.appendToMenu( &menu );

    if ( !menu.actions().empty() )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* RiuMultiPlotPage::createTitleLabel() const
{
    QLabel* plotTitle = new QLabel( "", nullptr );
    plotTitle->setAlignment( Qt::AlignHCenter );
    plotTitle->setWordWrap( true );
    plotTitle->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    return plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    performUpdate( RiaDefines::MultiPlotPageUpdateType::ALL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::hasHeightForWidth() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::heightForWidth( int width ) const
{
    QPageLayout pageLayout  = m_plotDefinition->pageLayout();
    QRectF      rect        = pageLayout.fullRectPoints();
    double      aspectRatio = rect.height() / rect.width();
    int         height      = static_cast<int>( width * aspectRatio );
    return height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateMarginsFromPageLayout()
{
    if ( m_previewMode )
    {
        QPageLayout pageLayout = m_plotDefinition->pageLayout();
        const int   resolution = RiaGuiApplication::applicationResolution();
        QMargins    margins    = pageLayout.marginsPixels( resolution );
        m_layout->setContentsMargins( margins );
    }
    else
    {
        m_layout->setContentsMargins( 0, 0, 0, 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMultiPlotPage::sizeHint() const
{
    if ( m_previewMode )
    {
        QPageLayout pageLayout = RiaPreferences::current()->defaultPageLayout();
        if ( m_plotDefinition )
        {
            pageLayout = m_plotDefinition->pageLayout();
        }

        const int resolution = RiaGuiApplication::applicationResolution();
        QRect     rect       = pageLayout.fullRectPixels( resolution );
        return rect.size();
    }
    return QWidget::sizeHint();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMultiPlotPage::minimumSizeHint() const
{
    QSize fullSizeHint = sizeHint();
    return QSize( fullSizeHint.width() / 2, fullSizeHint.height() / 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuMultiPlotPage::rowAndColumnCount( int plotWidgetCount ) const
{
    if ( plotWidgetCount == 0 )
    {
        return std::make_pair( 0, 0 );
    }

    int columnCount = std::max( 1, m_plotDefinition->columnCount() );
    int rowCount    = static_cast<int>( std::ceil( plotWidgetCount / static_cast<double>( columnCount ) ) );
    return std::make_pair( rowCount, columnCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    if ( !m_plotDefinition ) return;

    for ( RiuPlotWidget* plotWidget : m_plotWidgets )
    {
        if ( !plotWidget ) continue;
        RimPlot* plot = plotWidget->plotDefinition();
        if ( !plot ) continue;

        bool isSelected = false;
        for ( int changedLevel : changedSelectionLevels )
        {
            isSelected = isSelected || caf::SelectionManager::instance()->isSelected( plot, changedLevel );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::showYAxis( int row, int column ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::performUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate )
{
    auto multiPlot = dynamic_cast<RimMultiPlot*>( m_plotDefinition.p() );
    if ( multiPlot && !multiPlot->isValid() ) return;

    if ( whatToUpdate == RiaDefines::MultiPlotPageUpdateType::ALL )
    {
        applyLook();
        updateTitleFont();
        updateMarginsFromPageLayout();

        reinsertPlotWidgets();
        alignCanvasTops();
        if ( m_autoAlignAxes ) alignAxes();
        updatePlotLayouts();
        return;
    }

    if ( ( whatToUpdate & RiaDefines::MultiPlotPageUpdateType::LEGEND ) == RiaDefines::MultiPlotPageUpdateType::LEGEND )
    {
        refreshLegends();
        alignCanvasTops();
        if ( m_autoAlignAxes ) alignAxes();
    }

    if ( ( whatToUpdate & RiaDefines::MultiPlotPageUpdateType::TITLE ) == RiaDefines::MultiPlotPageUpdateType::TITLE )
    {
        updateTitleFont();
        updateSubTitles();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::onLegendUpdated()
{
    scheduleUpdate( RiaDefines::MultiPlotPageUpdateType::LEGEND );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::refreshLegends()
{
    QList<QPointer<RiuQwtPlotLegend>> legends = legendsForVisiblePlots();
    for ( const auto& l : legends )
    {
        l->setVisible( !l->isEmpty() );
        updateLegendFont( l );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updatePlotLayouts()
{
    QList<QPointer<RiuPlotWidget>> plotWidgets = visiblePlotWidgets();
    for ( auto p : plotWidgets )
        p->updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::reinsertPlotWidgets()
{
    clearGridLayout();

    QList<QPointer<QLabel>>                   subTitles    = subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>>         legends      = legendsForVisiblePlots();
    QList<QPointer<RiuDraggableOverlayFrame>> legendFrames = legendFramesForVisiblePlots();
    QList<QPointer<RiuPlotWidget>>            plotWidgets  = visiblePlotWidgets();

    m_visibleIndexToPositionMapping.clear();

    if ( !plotWidgets.empty() )
    {
        auto [rowCount, columnCount] = rowAndColumnCount( plotWidgets.size() );

        int row    = 0;
        int column = 0;
        for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
        {
            auto plotWidget  = plotWidgets[visibleIndex];
            auto legend      = legends[visibleIndex];
            auto legendFrame = legendFrames[visibleIndex];
            auto subTitle    = subTitles[visibleIndex];

            int expectedColSpan = plotWidget->colSpan();
            int colSpan         = std::min( expectedColSpan, columnCount );
            int rowSpan         = plotWidget->rowSpan();

            auto position           = findAvailableRowAndColumn( row, column, colSpan, columnCount );
            std::tie( row, column ) = position;

            m_visibleIndexToPositionMapping[visibleIndex] = position;

            reinsertPlotWidget( plotWidget, legend, legendFrame, subTitle, row, column, rowSpan, colSpan );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::reinsertPlotWidget( RiuPlotWidget*            plotWidget,
                                           RiuQwtPlotLegend*         legend,
                                           RiuDraggableOverlayFrame* legendFrame,
                                           QLabel*                   subTitle,
                                           int                       row,
                                           int                       column,
                                           int                       rowSpan,
                                           int                       colSpan )
{
    m_gridLayout->addWidget( subTitle, 3 * row, column, 1, colSpan );

    addLegendWidget( plotWidget, legend, legendFrame, 3 * row + 1, column, colSpan );

    m_gridLayout->addWidget( plotWidget, 3 * row + 2, column, 1 + ( rowSpan - 1 ) * 3, colSpan );

    updateSubTitleVisibility( subTitle );

    setDefaultAxisProperties( plotWidget, row, column );

    adjustHeadingSpacing( plotWidget );

    plotWidget->show();

    updateLegendVisibility( plotWidget, legend, legendFrame );

    setRowAndColumnStretches( row, column, rowSpan, colSpan );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::addLegendWidget( RiuPlotWidget*            plotWidget,
                                        RiuQwtPlotLegend*         legend,
                                        RiuDraggableOverlayFrame* legendFrame,
                                        int                       row,
                                        int                       column,
                                        int                       colSpan )
{
    if ( !legend ) return;

    if ( m_plotDefinition->legendPosition() == RimPlotWindow::LegendPosition::ABOVE )
    {
        m_gridLayout->addWidget( legend, row, column, 1, colSpan, Qt::AlignHCenter | Qt::AlignBottom );
    }
    else
    {
        auto anchor = RiuDraggableOverlayFrame::AnchorCorner::TopRight;

        if ( m_plotDefinition->legendPosition() == RimPlotWindow::LegendPosition::INSIDE_UPPER_RIGHT )
            anchor = RiuDraggableOverlayFrame::AnchorCorner::TopRight;
        else if ( m_plotDefinition->legendPosition() == RimPlotWindow::LegendPosition::INSIDE_UPPER_LEFT )
            anchor = RiuDraggableOverlayFrame::AnchorCorner::TopLeft;

        auto overlayFrame = new RiuQwtLegendOverlayContentFrame;
        overlayFrame->setLegend( legend );
        legendFrame->setContentFrame( overlayFrame );
        legendFrame->setAnchorCorner( anchor );
        plotWidget->removeOverlayFrame( legendFrame );
        plotWidget->addOverlayFrame( legendFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateLegendVisibility( RiuPlotWidget* plotWidget, RiuQwtPlotLegend* legend, RiuDraggableOverlayFrame* legendFrame )
{
    if ( !legend ) return;

    if ( m_plotDefinition->legendsVisible() && !legend->isEmpty() )
    {
        updateLegendColumns( legend );
        updateLegendFont( legend );
        legend->show();

        if ( m_plotDefinition->legendPosition() == RimPlotWindow::LegendPosition::INSIDE_UPPER_LEFT ||
             m_plotDefinition->legendPosition() == RimPlotWindow::LegendPosition::INSIDE_UPPER_RIGHT )
        {
            plotWidget->addOverlayFrame( legendFrame );
            legendFrame->show();
        }
        else
        {
            plotWidget->removeOverlayFrame( legendFrame );
            legendFrame->hide();
        }
    }
    else
    {
        legend->hide();
        legendFrame->hide();
        plotWidget->removeOverlayFrame( legendFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateSubTitleVisibility( QLabel* subTitle )
{
    bool isSubTitleVisible = m_showSubTitles && !subTitle->text().isEmpty();
    subTitle->setVisible( isSubTitleVisible );
    QFont subTitleFont = subTitle->font();
    subTitleFont.setPixelSize( m_subTitleFontPixelSize );
    subTitle->setFont( subTitleFont );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setDefaultAxisProperties( RiuPlotWidget* plotWidget, int row, int column )
{
    plotWidget->setAxisLabelsAndTicksEnabled( RiuPlotAxis::defaultLeft(), showYAxis( row, column ), showYAxis( row, column ) );
    plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), showYAxis( row, column ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::adjustHeadingSpacing( RiuPlotWidget* plotWidget )
{
    // Adjust the space below a graph to make sure the heading of the row below is closest to the
    // corresponding graph

    if ( !m_plotDefinition ) return;

    int bottomMargin = m_plotDefinition->bottomMargin();
    if ( bottomMargin < 0 ) return;

    auto margins = plotWidget->contentsMargins();

    margins.setBottom( bottomMargin );
    plotWidget->setContentsMargins( margins );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setRowAndColumnStretches( int row, int column, int rowSpan, int colSpan )
{
    // Set basic row and column stretches
    for ( int r = row; r < row + rowSpan; ++r )
    {
        m_gridLayout->setRowStretch( 3 * r + 2, 1 );
    }

    for ( int c = column; c < column + colSpan; ++c )
    {
        int colStretch = 6; // Empirically chosen to try to counter the width of the axis on the first track
        if ( showYAxis( row, column ) ) colStretch += 1;
        m_gridLayout->setColumnStretch( c, std::max( colStretch, m_gridLayout->columnStretch( c ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateLegendFont( RiuQwtPlotLegend* legend )
{
    QFont legendFont = legend->font();
    legendFont.setPixelSize( m_legendFontPixelSize );

    // Set font size for all existing labels
    QList<QwtLegendLabel*> labels = legend->findChildren<QwtLegendLabel*>();
    for ( QwtLegendLabel* label : labels )
    {
        label->setFont( legendFont );
        label->setMargin( 0 );
        label->setSpacing( 3 );
    }

    legend->setFont( legendFont );
    legend->setContentsMargins( 0, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateLegendColumns( RiuQwtPlotLegend* legend )
{
    int legendColumns = 1;
    if ( m_plotDefinition->legendsHorizontal() )
    {
        legendColumns = 0; // unlimited
    }

    legend->setMaxColumns( legendColumns );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_plotWidgets.size() );

    QList<QPointer<RiuPlotWidget>>    plotWidgets = visiblePlotWidgets();
    QList<QPointer<RiuQwtPlotLegend>> legends     = legendsForVisiblePlots();
    if ( plotWidgets.empty() ) return 0;

    auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

    std::vector<double> maxExtents( rowAndColumnCount.first, 0.0 );

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        RiuQwtPlotWidget* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidgets[visibleIndex].data() );
        if ( qwtPlotWidget )
        {
            int row = visibleIndex / rowAndColumnCount.second;
            if ( plotWidgets[visibleIndex]->axisEnabled( RiuPlotAxis::defaultTop() ) )
            {
                QFont font      = qwtPlotWidget->qwtPlot()->axisFont( QwtAxis::XTop );
                maxExtents[row] = std::max( maxExtents[row], qwtPlotWidget->qwtPlot()->axisScaleDraw( QwtAxis::XTop )->extent( font ) );
            }
        }
    }

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        RiuQwtPlotWidget* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidgets[visibleIndex].data() );
        if ( qwtPlotWidget )
        {
            int row = visibleIndex / rowAndColumnCount.second;
            qwtPlotWidget->qwtPlot()->axisScaleDraw( QwtAxis::XTop )->setMinimumExtent( maxExtents[row] );
            auto legend = legends[visibleIndex];
            if ( legend )
            {
                int previousChildCount = -1;

                auto it = m_childCountForAdjustSizeOperation.find( legend );
                if ( it != m_childCountForAdjustSizeOperation.end() )
                {
                    previousChildCount = it->second;
                }

                auto legendItemCount = legend->contentsWidget()->children().size();
                if ( previousChildCount != legendItemCount )
                {
                    legends[visibleIndex]->adjustSize();
                    m_childCountForAdjustSizeOperation[legend] = legendItemCount;
                }
            }
        }
    }
    return maxExtents[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::clearGridLayout()
{
    if ( m_gridLayout )
    {
        for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
        {
            m_gridLayout->removeWidget( m_subTitles[tIdx] );
            if ( m_legends[tIdx] )
            {
                m_gridLayout->removeWidget( m_legends[tIdx] );
            }
            m_gridLayout->removeWidget( m_plotWidgets[tIdx] );
        }

        QLayoutItem* item;
        while ( ( item = m_gridLayout->takeAt( 0 ) ) != nullptr )
        {
        }
        QWidget().setLayout( m_gridLayout );
        delete m_gridLayout;
        m_gridLayout = new QGridLayout( m_plotWidgetFrame );
    }

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        if ( m_plotWidgets[tIdx] )
        {
            m_plotWidgets[tIdx]->removeOverlayFrame( m_legendFrames[tIdx] );
            m_plotWidgets[tIdx]->hide();
        }
        if ( m_legends[tIdx] )
        {
            m_legends[tIdx]->hide();
        }
        if ( m_subTitles[tIdx] )
        {
            m_subTitles[tIdx]->hide();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuPlotWidget>> RiuMultiPlotPage::visiblePlotWidgets() const
{
    QList<QPointer<RiuPlotWidget>> plotWidgets;
    for ( QPointer<RiuPlotWidget> plotWidget : m_plotWidgets )
    {
        if ( plotWidget->isChecked() )
        {
            plotWidgets.push_back( plotWidget );
        }
    }
    return plotWidgets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotLegend>> RiuMultiPlotPage::legendsForVisiblePlots() const
{
    QList<QPointer<RiuQwtPlotLegend>> legends;
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            legends.push_back( m_legends[i] );
        }
    }
    return legends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuDraggableOverlayFrame>> RiuMultiPlotPage::legendFramesForVisiblePlots() const
{
    QList<QPointer<RiuDraggableOverlayFrame>> legendFrames;
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            legendFrames.push_back( m_legendFrames[i] );
        }
    }
    return legendFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<QLabel>> RiuMultiPlotPage::subTitlesForVisiblePlots() const
{
    QList<QPointer<QLabel>> subTitles;
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            m_subTitles[i]->setText( m_plotWidgets[i]->plotTitle() );
            subTitles.push_back( m_subTitles[i] );
        }
    }
    return subTitles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuMultiPlotPage::findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const
{
    int availableRow    = startRow;
    int availableColumn = startColumn;
    while ( true )
    {
        for ( ; availableColumn < columnCount; ++availableColumn )
        {
            bool fits = true;
            for ( int c = availableColumn; ( c < availableColumn + columnSpan ) && fits; ++c )
            {
                if ( c >= columnCount )
                {
                    fits = false;
                }

                if ( m_gridLayout->itemAtPosition( 3 * availableRow, c ) != nullptr )
                {
                    fits = false;
                }
            }
            if ( fits )
            {
                return std::make_pair( availableRow, availableColumn );
            }
        }
        availableColumn = 0;
        availableRow++;
    }
    return std::make_pair( availableRow, availableColumn );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::applyLook()
{
    QPalette newPalette( palette() );
    auto     backgroundColor = RiuGuiTheme::getColorByVariableName( "mainBackgroundColor" );
    newPalette.setColor( QPalette::Window, backgroundColor );
    setPalette( newPalette );

    setAutoFillBackground( true );
    setBackgroundRole( QPalette::Window );

    if ( m_previewMode )
    {
        setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

        QGraphicsDropShadowEffect* dropShadowEffect = new QGraphicsDropShadowEffect( this );
        dropShadowEffect->setOffset( 4.0, 4.0 );
        dropShadowEffect->setBlurRadius( 4.0 );
        dropShadowEffect->setColor( QColor( 40, 40, 40, 40 ) );
        setGraphicsEffect( dropShadowEffect );
    }
    else
    {
        setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
        setGraphicsEffect( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateTitleFont()
{
    auto titleFont = m_plotTitle->font();
    titleFont.setPixelSize( m_titleFontPixelSize );
    m_plotTitle->setFont( titleFont );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::alignAxes()
{
    auto [rowCount, columnCount] = rowAndColumnCount( visiblePlotWidgets().size() );

    auto matchRow = []( int row, int column, int targetRow ) { return row == targetRow; };

    for ( int row = 0; row < rowCount; row++ )
    {
        alignAxis( QwtAxisId( QwtAxis::Position::XTop, 0 ), row, matchRow );
        alignAxis( QwtAxisId( QwtAxis::Position::XBottom, 0 ), row, matchRow );
    }

    auto matchColumn = []( int row, int column, int targetColumn ) { return column == targetColumn; };

    for ( int column = 0; column < columnCount; column++ )
    {
        alignAxis( QwtAxisId( QwtAxis::Position::YLeft, 0 ), column, matchColumn );
        alignAxis( QwtAxisId( QwtAxis::Position::YRight, 0 ), column, matchColumn );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::alignAxis( QwtAxisId axis, int targetRowOrColumn, std::function<bool( int, int, int )> matchPosition )
{
    auto rowAndColumnFromIdx = [this]( int idx )
    {
        auto hit = m_visibleIndexToPositionMapping.find( idx );
        CAF_ASSERT( hit != m_visibleIndexToPositionMapping.end() );
        return hit->second;
    };

    QList<QPointer<RiuPlotWidget>> plotWidgets = visiblePlotWidgets();

    // Find the max extent of the "scale draws" for the given axis
    double maxExtent = 0;
    for ( int tIdx = 0; tIdx < plotWidgets.size(); ++tIdx )
    {
        RiuPlotWidget* plotWidget = plotWidgets[tIdx];
        auto [row, column]        = rowAndColumnFromIdx( tIdx );
        bool matchesRowOrColumn   = matchPosition( row, column, targetRowOrColumn );
        if ( plotWidget && matchesRowOrColumn )
        {
            RiuQwtPlotWidget* riuQwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget );

            if ( riuQwtPlotWidget )
            {
                QwtPlot* p = riuQwtPlotWidget->qwtPlot();
                if ( p )
                {
                    QwtScaleWidget* scaleWidget = p->axisWidget( axis );
                    QwtScaleDraw*   sd          = scaleWidget->scaleDraw();
                    sd->setMinimumExtent( 0.0 );
                    maxExtent = std::max( sd->extent( scaleWidget->font() ), maxExtent );
                }
            }
        }
    }

    // Set minimum extent for all "scale draws" for the given axis.
    for ( int tIdx = 0; tIdx < plotWidgets.size(); ++tIdx )
    {
        RiuPlotWidget* plotWidget = plotWidgets[tIdx];
        auto [row, column]        = rowAndColumnFromIdx( tIdx );
        bool matchesRowOrColumn   = matchPosition( row, column, targetRowOrColumn );
        if ( plotWidget && matchesRowOrColumn )
        {
            RiuQwtPlotWidget* riuQwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget );
            if ( riuQwtPlotWidget )
            {
                QwtPlot* p = riuQwtPlotWidget->qwtPlot();
                if ( p )
                {
                    QwtScaleWidget* scaleWidget = p->axisWidget( axis );
                    scaleWidget->scaleDraw()->setMinimumExtent( maxExtent );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setAutoAlignAxes( bool autoAlignAxes )
{
    m_autoAlignAxes = autoAlignAxes;
}
