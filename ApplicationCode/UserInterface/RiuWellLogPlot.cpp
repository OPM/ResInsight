#include "RiuWellLogPlot.h"

#include "RimDepthTrackPlot.h"
#include "RimPlotWindow.h"

#include "RiuQwtPlotWidget.h"
#include "RiuWellLogTrack.h"

#include "cafAssert.h"
#include "cafPdmPointer.h"

#include <QScrollBar>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot( RimDepthTrackPlot* plotDefinition, QWidget* parent )
    : RiuMultiPlotPage( plotDefinition, parent )
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
RimDepthTrackPlot* RiuWellLogPlot::wellLogPlotDefinition()
{
    RimDepthTrackPlot* wellLogPlot = dynamic_cast<RimDepthTrackPlot*>( m_plotDefinition.p() );
    CAF_ASSERT( wellLogPlot );
    return wellLogPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuWellLogPlot::ownerViewWindow() const
{
    return m_plotDefinition;
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
    }
    m_trackScrollBar->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::renderTo( QPaintDevice* paintDevice )
{
    m_trackScrollBar->setVisible( false );
    RiuMultiPlotPage::renderTo( paintDevice );
    m_trackScrollBar->setVisible( true );
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
    int                               colCount    = this->m_gridLayout->columnCount();

    m_gridLayout->addLayout( m_trackScrollBarLayout, 2, colCount, 1, 1 );
    m_trackScrollBar->setVisible( !plotWidgets.empty() );
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
    wellLogPlotDefinition()->setAutoScaleDepthEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::performUpdate()
{
    m_trackScrollBar->setVisible( false );
    reinsertPlotWidgets();
    reinsertScrollbar();
    int axisShift = alignCanvasTops();
    alignScrollbar( axisShift );
}
