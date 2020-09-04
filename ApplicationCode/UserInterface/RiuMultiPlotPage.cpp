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
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

#include "RiaGuiApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotObjectPicker.h"
#include "RiuQwtPlotLegend.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_draw.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage::RiuMultiPlotPage( RimPlotWindow* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_previewMode( false )
    , m_showSubTitles( false )
    , m_titleFontPixelSize( 12 )
    , m_subTitleFontPixelSize( 11 )
    , m_legendFontPixelSize( 8 )
    , m_axisTitleFontSize( 8 )
    , m_axisValueFontSize( 8 )

{
    CAF_ASSERT( m_plotDefinition );

    m_layout = new QVBoxLayout( this );
    m_layout->setMargin( 0 );
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
    m_gridLayout->setSpacing( 5 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    setFocusPolicy( Qt::StrongFocus );

    this->setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );

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
void RiuMultiPlotPage::addPlot( RiuQwtPlotWidget* plotWidget )
{
    // Insert the plot to the left of the scroll bar
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
{
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    plotWidget->setVisible( false );

    QString subTitleText = plotWidget->plotTitle();
    QLabel* subTitle     = new QLabel( subTitleText );
    subTitle->setAlignment( Qt::AlignHCenter | Qt::AlignBottom );
    subTitle->setWordWrap( true );
    subTitle->setVisible( false );
    m_subTitles.insert( static_cast<int>( index ), subTitle );

    RiuQwtPlotLegend* legend = nullptr;
    if ( m_plotDefinition->legendsVisible() )
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
        legend->connect( plotWidget,
                         SIGNAL( legendDataChanged( const QVariant&, const QList<QwtLegendData>& ) ),
                         SLOT( updateLegend( const QVariant&, const QList<QwtLegendData>& ) ) );
        QObject::connect( legend, SIGNAL( legendUpdated() ), this, SLOT( onLegendUpdated() ) );

        legend->contentsWidget()->layout()->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
        legend->setVisible( false );
        plotWidget->updateLegend();
    }
    m_legends.insert( static_cast<int>( index ), legend );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removePlot( RiuQwtPlotWidget* plotWidget )
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

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removeAllPlots()
{
    auto plotWidgets = m_plotWidgets;
    for ( RiuQwtPlotWidget* plotWidget : plotWidgets )
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
int RiuMultiPlotPage::indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleUpdate()
{
    CAF_ASSERT( m_plotDefinition );
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotPageUpdate( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleReplotOfAllPlots()
{
    for ( RiuQwtPlotWidget* plotWidget : visiblePlotWidgets() )
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
    int    resolution = paintDevice->logicalDpiX();
    double scaling    = resolution / static_cast<double>( RiaGuiApplication::applicationResolution() );

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

    menuBuilder.appendToMenu( &menu );

    if ( menu.actions().size() > 0 )
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
    performUpdate();
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
        QPageLayout pageLayout = RiaApplication::instance()->preferences()->defaultPageLayout();
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

    for ( RiuQwtPlotWidget* plotWidget : m_plotWidgets )
    {
        CAF_ASSERT( plotWidget );
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
void RiuMultiPlotPage::performUpdate()
{
    applyLook();
    updateMarginsFromPageLayout();
    reinsertPlotWidgets();
    alignCanvasTops();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::onLegendUpdated()
{
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::reinsertPlotWidgets()
{
    clearGridLayout();

    auto titleFont = m_plotTitle->font();
    titleFont.setPixelSize( m_titleFontPixelSize );
    m_plotTitle->setFont( titleFont );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        if ( m_plotWidgets[tIdx] )
        {
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

    QList<QPointer<QLabel>>           subTitles   = this->subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->legendsForVisiblePlots();
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = this->visiblePlotWidgets();

    if ( !plotWidgets.empty() )
    {
        auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

        int row    = 0;
        int column = 0;
        for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
        {
            int expectedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
            int colSpan         = std::min( expectedColSpan, rowAndColumnCount.second );
            int rowSpan         = plotWidgets[visibleIndex]->rowSpan();

            std::tie( row, column ) = findAvailableRowAndColumn( row, column, colSpan, rowAndColumnCount.second );

            m_gridLayout->addWidget( subTitles[visibleIndex], 3 * row, column, 1, colSpan );
            if ( legends[visibleIndex] )
            {
                m_gridLayout->addWidget( legends[visibleIndex], 3 * row + 1, column, 1, colSpan, Qt::AlignHCenter | Qt::AlignBottom );
            }
            m_gridLayout->addWidget( plotWidgets[visibleIndex], 3 * row + 2, column, 1 + ( rowSpan - 1 ) * 3, colSpan );

            subTitles[visibleIndex]->setVisible( m_showSubTitles );
            QFont subTitleFont = subTitles[visibleIndex]->font();
            subTitleFont.setPixelSize( m_subTitleFontPixelSize );
            subTitles[visibleIndex]->setFont( subTitleFont );

            plotWidgets[visibleIndex]->setAxisLabelsAndTicksEnabled( QwtPlot::yLeft,
                                                                     showYAxis( row, column ),
                                                                     showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxisTitleEnabled( QwtPlot::yLeft, showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxesFontsAndAlignment( m_axisTitleFontSize, m_axisValueFontSize );

            plotWidgets[visibleIndex]->show();

            if ( legends[visibleIndex] )
            {
                if ( m_plotDefinition->legendsVisible() )
                {
                    int legendColumns = 1;
                    if ( m_plotDefinition->legendsHorizontal() )
                    {
                        legendColumns = 0; // unlimited
                    }
                    legends[visibleIndex]->setMaxColumns( legendColumns );
                    QFont legendFont = legends[visibleIndex]->font();
                    legendFont.setPixelSize( m_legendFontPixelSize );
                    legends[visibleIndex]->setFont( legendFont );
                    legends[visibleIndex]->show();
                }
                else
                {
                    legends[visibleIndex]->hide();
                }
            }
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
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_plotWidgets.size() );

    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = visiblePlotWidgets();
    QList<QPointer<RiuQwtPlotLegend>> legends     = legendsForVisiblePlots();
    if ( plotWidgets.empty() ) return 0;

    auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

    std::vector<double> maxExtents( rowAndColumnCount.first, 0.0 );

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;
        if ( plotWidgets[visibleIndex]->axisEnabled( QwtPlot::xTop ) )
        {
            QFont font = m_plotWidgets[visibleIndex]->axisFont( QwtPlot::xTop );
            maxExtents[row] =
                std::max( maxExtents[row], plotWidgets[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->extent( font ) );
        }
    }

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;
        plotWidgets[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->setMinimumExtent( maxExtents[row] );
        if ( legends[visibleIndex] )
        {
            legends[visibleIndex]->adjustSize();
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotWidget>> RiuMultiPlotPage::visiblePlotWidgets() const
{
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets;
    for ( QPointer<RiuQwtPlotWidget> plotWidget : m_plotWidgets )
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
std::pair<int, int>
    RiuMultiPlotPage::findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const
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
    newPalette.setColor( QPalette::Window, Qt::white );
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
