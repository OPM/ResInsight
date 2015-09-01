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

#define RIU_SCROLLWHEEL_ZOOMFACTOR 1.05

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot(RimWellLogPlot* dataModelPlot, QWidget* parent)
    : QWidget(parent)
{
    Q_ASSERT(dataModelPlot);
    m_dataModelPlot = dataModelPlot;

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);

    setLayout(m_layout);
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

    m_layout->addWidget(tracePlot);
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
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::wheelEvent(QWheelEvent* event)
{
     if (!m_dataModelPlot)
     {
         QWidget::wheelEvent(event);
         return;
     }

     if (event->delta() > 0)
     {
         m_dataModelPlot->zoomDepth(RIU_SCROLLWHEEL_ZOOMFACTOR);
     }
     else
     {
         m_dataModelPlot->zoomDepth(1.0/RIU_SCROLLWHEEL_ZOOMFACTOR);
     }

     event->accept();
}
