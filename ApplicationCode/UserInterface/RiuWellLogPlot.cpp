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

#include "RiuWellLogPlot.h"

#include "RiuWellLogTracePlot.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"

#include "cvfAssert.h"

#include <QHBoxLayout>
#include <QWheelEvent>
#include <QScrollBar>

#include <math.h>

#define RIU_SCROLLWHEEL_ZOOMFACTOR 1.05

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot(RimWellLogPlot* plotDefinition, QWidget* parent)
    : QWidget(parent)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    setLayout(m_layout);

    m_scrollBar = new QScrollBar(this);
    m_scrollBar->setOrientation(Qt::Vertical);
    m_scrollBar->setVisible(false);
    m_layout->addWidget(m_scrollBar);
    
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotSetMinDepth(int)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::~RiuWellLogPlot()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTracePlot* RiuWellLogPlot::createTracePlot()
{
    RiuWellLogTracePlot* tracePlot = new RiuWellLogTracePlot(this);

    // Insert the plot to the left of the scroll bar
    m_layout->insertWidget(m_layout->count() - 1, tracePlot);
    m_tracePlots.append(tracePlot);

    return tracePlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setDepthRange(double minDepth, double maxDepth)
{
    for (int tpIdx = 0; tpIdx < m_tracePlots.count(); tpIdx++)
    {
        m_tracePlots[tpIdx]->setDepthRange(minDepth, maxDepth);
    }

    updateScrollBar(minDepth, maxDepth);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateScrollBar(double minDepth, double maxDepth)
{
    double availableMinDepth;
    double availableMaxDepth;
    if (m_plotDefinition->availableDepthRange(&availableMinDepth, &availableMaxDepth))
    {
        double availableDepth = availableMaxDepth - availableMinDepth;
        double visibleDepth = maxDepth - minDepth;

        m_scrollBar->setRange((int) availableMinDepth, (int) (ceil(availableMaxDepth - visibleDepth)));
        m_scrollBar->setPageStep((int) visibleDepth);
        m_scrollBar->setValue((int) minDepth);

        m_scrollBar->setVisible(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::wheelEvent(QWheelEvent* event)
{
     if (!m_plotDefinition)
     {
         QWidget::wheelEvent(event);
         return;
     }

     if (event->modifiers() & Qt::ControlModifier)
     {
         if (event->delta() > 0)
         {
             m_plotDefinition->zoomDepth(RIU_SCROLLWHEEL_ZOOMFACTOR);
         }
         else
         {
             m_plotDefinition->zoomDepth(1.0/RIU_SCROLLWHEEL_ZOOMFACTOR);
         }
     }

     event->accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth(int value)
{
    double minimumDepth;
    double maximumDepth;
    m_plotDefinition->visibleDepthRange(&minimumDepth, &maximumDepth);

    double delta = value - minimumDepth;
    m_plotDefinition->setDepthRange(minimumDepth + delta, maximumDepth + delta);
}
