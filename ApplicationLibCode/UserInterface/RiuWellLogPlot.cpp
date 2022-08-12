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
    m_verticalTrackScrollBar = new QScrollBar( nullptr );
    m_verticalTrackScrollBar->setOrientation( Qt::Vertical );
    m_verticalTrackScrollBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

    m_verticalTrackScrollBarLayout = new QVBoxLayout;
    m_verticalTrackScrollBarLayout->addWidget( m_verticalTrackScrollBar, 0 );

    connect( m_verticalTrackScrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );

    m_horizontalTrackScrollBar = new QScrollBar( nullptr );
    m_horizontalTrackScrollBar->setOrientation( Qt::Horizontal );
    m_horizontalTrackScrollBar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    m_horizontalTrackScrollBarLayout = new QHBoxLayout;
    m_horizontalTrackScrollBarLayout->addWidget( m_horizontalTrackScrollBar, 0 );

    connect( m_horizontalTrackScrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot* RiuWellLogPlot::depthTrackPlot()
{
    auto* wellLogPlot = dynamic_cast<RimDepthTrackPlot*>( m_plotDefinition.p() );
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

    m_verticalTrackScrollBar->blockSignals( true );
    {
        m_verticalTrackScrollBar->setRange( (int)minAvailable, (int)( ( maxAvailable - visibleRange ) ) );
        m_verticalTrackScrollBar->setPageStep( (int)visibleRange );
        m_verticalTrackScrollBar->setValue( (int)minVisible );
    }
    m_verticalTrackScrollBar->blockSignals( false );

    m_horizontalTrackScrollBar->blockSignals( true );
    {
        m_horizontalTrackScrollBar->setRange( (int)minAvailable, (int)( ( maxAvailable - visibleRange ) ) );
        m_horizontalTrackScrollBar->setPageStep( (int)visibleRange );
        m_horizontalTrackScrollBar->setValue( (int)minVisible );
    }
    m_horizontalTrackScrollBar->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::renderTo( QPaintDevice* paintDevice )
{
    m_verticalTrackScrollBar->setVisible( false );
    m_horizontalTrackScrollBar->setVisible( false );

    RiuMultiPlotPage::renderTo( paintDevice );

    if ( depthTrackPlot() && depthTrackPlot()->depthOrientation() == RimDepthTrackPlot::DepthOrientation::HORIZONTAL )
        m_horizontalTrackScrollBar->setVisible( true );
    else
        m_verticalTrackScrollBar->setVisible( true );
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
    QList<QPointer<RiuPlotWidget>> plotWidgets = this->visiblePlotWidgets();
    int                            colCount    = this->m_gridLayout->columnCount();
    int                            rowCount    = this->m_gridLayout->rowCount();

    if ( depthTrackPlot() && depthTrackPlot()->depthOrientation() == RimDepthTrackPlot::DepthOrientation::HORIZONTAL )
    {
        m_gridLayout->addLayout( m_horizontalTrackScrollBarLayout, rowCount, 0, 1, colCount );
        m_horizontalTrackScrollBar->setVisible( !plotWidgets.empty() );
    }
    else
    {
        m_gridLayout->addLayout( m_verticalTrackScrollBarLayout, 2, colCount, 1, 1 );
        m_verticalTrackScrollBar->setVisible( !plotWidgets.empty() );
        m_gridLayout->setColumnStretch( colCount, 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::alignScrollbar( int offset )
{
    m_verticalTrackScrollBarLayout->setContentsMargins( 0, offset, 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth( int value )
{
    double minimumDepth;
    double maximumDepth;
    depthTrackPlot()->visibleDepthRange( &minimumDepth, &maximumDepth );

    double delta = value - minimumDepth;
    depthTrackPlot()->setDepthAxisRange( minimumDepth + delta, maximumDepth + delta );
    depthTrackPlot()->setAutoScaleDepthValuesEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::performUpdate( RiaDefines::MultiPlotPageUpdateType /* whatToUpdate */ )
{
    m_horizontalTrackScrollBar->setVisible( false );
    m_verticalTrackScrollBar->setVisible( false );

    reinsertPlotWidgets();
    reinsertScrollbar();
    updateTitleFont();
    int axisShift = alignCanvasTops();
    alignScrollbar( axisShift );
    alignAxes();
}
