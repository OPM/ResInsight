#include "RiuWellLogPlot.h"

#include "RimPlotWindow.h"
#include "RimWellLogPlot.h"

#include "cafAssert.h"
#include "cafPdmPointer.h"

#include <QScrollBar>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot( RimWellLogPlot* plotDefinition, QWidget* parent )
    : RiuGridPlotWindow( plotDefinition, parent )
{
    connect( m_scrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );
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
void RiuWellLogPlot::updateVerticalScrollBar( double minVisible, double maxVisible, double minAvailable, double maxAvailable )
{
    maxAvailable += 0.01 * ( maxAvailable - minAvailable );

    double visibleRange = maxVisible - minVisible;

    m_scrollBar->blockSignals( true );
    {
        m_scrollBar->setRange( (int)minAvailable, (int)( ( maxAvailable - visibleRange ) ) );
        m_scrollBar->setPageStep( (int)visibleRange );
        m_scrollBar->setValue( (int)minVisible );
        m_scrollBar->setVisible( true );
    }
    m_scrollBar->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth( int value )
{
    double minimumDepth;
    double maximumDepth;
    wellLogPlotDefinition()->availableDepthRange( &minimumDepth, &maximumDepth );

    double delta = value - minimumDepth;
    wellLogPlotDefinition()->setDepthAxisRange( minimumDepth + delta, maximumDepth + delta );
    wellLogPlotDefinition()->setAutoScaleYEnabled( false );
}
