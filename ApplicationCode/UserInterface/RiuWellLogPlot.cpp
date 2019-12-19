#include "RiuWellLogPlot.h"

#include "RimPlotWindow.h"
#include "RimWellLogPlot.h"

#include "RiuQwtPlotWidget.h"
#include "RiuWellLogTrack.h"

#include "cafAssert.h"
#include "cafPdmPointer.h"

#include <QScrollBar>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot( RimWellLogPlot* plotDefinition, QWidget* parent )
    : RiuMultiPlotWindow( plotDefinition, parent )
{
    m_trackScrollBar = new QScrollBar( nullptr );
    m_trackScrollBar->setOrientation( Qt::Vertical );
    m_trackScrollBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

    m_trackScrollBarLayout = new QVBoxLayout;
    m_trackScrollBarLayout->addWidget( m_trackScrollBar, 0 );

    connect( m_trackScrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RiuWellLogPlot::wellLogPlotDefinition()
{
    RimWellLogPlot* wellLogPlot = dynamic_cast<RimWellLogPlot*>( m_plotDefinition.p() );
    CAF_ASSERT( wellLogPlot );
    return wellLogPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::doRenderTo( QPaintDevice* paintDevice )
{
    m_trackScrollBar->setVisible( false );
    RiuMultiPlotWindow::doRenderTo( paintDevice );
    m_trackScrollBar->setVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateVerticalScrollBar( double minVisible, double maxVisible, double minAvailable, double maxAvailable )
{
    maxAvailable += 0.01 * ( maxAvailable - minAvailable );

    double visibleRange = maxVisible - minVisible;

    m_trackScrollBar->blockSignals( true );
    {
        m_trackScrollBar->setRange( (int)minAvailable, (int)( ( maxAvailable - visibleRange ) ) );
        m_trackScrollBar->setPageStep( (int)visibleRange );
        m_trackScrollBar->setValue( (int)minVisible );
        m_trackScrollBar->setVisible( true );
    }
    m_trackScrollBar->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::keyPressEvent( QKeyEvent* event )
{
    wellLogPlotDefinition()->handleKeyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellLogPlot::willAcceptDroppedPlot( const RiuQwtPlotWidget* plotWidget ) const
{
    return dynamic_cast<const RiuWellLogTrack*>( plotWidget ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellLogPlot::showYAxis( int row, int column ) const
{
    return column == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::reinsertScrollbar()
{
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = this->visiblePlotWidgets();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->legendsForVisiblePlots();
    int                               rowCount    = this->m_gridLayout->rowCount();
    int                               colCount    = this->m_gridLayout->columnCount();

    m_trackScrollBar->setVisible( !plotWidgets.empty() );

    m_gridLayout->addLayout( m_trackScrollBarLayout, 2, colCount, rowCount * 2 - 1, 1 );
    m_gridLayout->setColumnStretch( colCount, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::alignScrollbar( int offset )
{
    m_trackScrollBarLayout->setContentsMargins( 0, offset, 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth( int value )
{
    double minimumDepth;
    double maximumDepth;
    wellLogPlotDefinition()->visibleDepthRange( &minimumDepth, &maximumDepth );

    double delta = value - minimumDepth;
    wellLogPlotDefinition()->setDepthAxisRange( minimumDepth + delta, maximumDepth + delta );
    wellLogPlotDefinition()->setAutoScaleYEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::performUpdate()
{
    reinsertPlotWidgets();
    reinsertScrollbar();
    int axisShift = alignCanvasTops();
    alignScrollbar( axisShift );
}
