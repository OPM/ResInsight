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

#include "RiuWellLogTrackPlot.h"
#include "RiuMainWindow.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrack.h"

#include "cafPdmUiTreeView.h"
#include "cvfAssert.h"

#include <QHBoxLayout>
#include <QScrollBar>
#include <QFocusEvent>
#include <QMdiSubWindow>

#include <math.h>

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
    if (m_plotDefinition)
    {
        m_plotDefinition->handleViewerDeletion();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::addTrackPlot(RiuWellLogTrackPlot* trackPlot)
{
    // Insert the plot to the left of the scroll bar
    insertTrackPlot(trackPlot, m_layout->count() - 1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::insertTrackPlot(RiuWellLogTrackPlot* trackPlot, size_t index)
{
    m_layout->insertWidget(static_cast<int>(index), trackPlot);
    m_trackPlots.append(trackPlot); // insert?

    modifyWidthOfContainingMdiWindow(trackPlot->width());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::removeTrackPlot(RiuWellLogTrackPlot* trackPlot)
{
    m_layout->removeWidget(trackPlot);
    m_trackPlots.removeAll(trackPlot);

    modifyWidthOfContainingMdiWindow(-trackPlot->width());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::modifyWidthOfContainingMdiWindow(int widthChange)
{
    QMdiSubWindow* mdiWindow = RiuMainWindow::instance()->findMdiSubWindow(this);
    if (mdiWindow)
    {
        QSize subWindowSize = mdiWindow->size();

        int newWidth = subWindowSize.width() + widthChange;
        if (newWidth < 0) newWidth = 100;

        subWindowSize.setWidth(newWidth);
        mdiWindow->resize(subWindowSize);

        if (mdiWindow->isMaximized())
        {
            // Set window temporarily to normal state and back to maximized
            // to redo layout so the whole window canvas is filled
            // Tried to activate layout, did not work as expected
            // Tested code:
            //   m_layout->activate();
            //   mdiWindow->layout()->activate();

            mdiWindow->showNormal();
            mdiWindow->showMaximized();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setDepthZoomAndReplot(double minDepth, double maxDepth)
{
    for (int tpIdx = 0; tpIdx < m_trackPlots.count(); tpIdx++)
    {
        m_trackPlots[tpIdx]->setDepthZoom(minDepth, maxDepth);
        m_trackPlots[tpIdx]->replot();
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
    m_plotDefinition->availableDepthRange(&availableMinDepth, &availableMaxDepth);

    double visibleDepth = maxDepth - minDepth;

    m_scrollBar->setRange((int) availableMinDepth, (int) (ceil(availableMaxDepth - visibleDepth)));
    m_scrollBar->setPageStep((int) visibleDepth);
    m_scrollBar->setValue((int) minDepth);

    m_scrollBar->setVisible(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth(int value)
{
    double minimumDepth;
    double maximumDepth;
    m_plotDefinition->depthZoomMinMax(&minimumDepth, &maximumDepth);

    double delta = value - minimumDepth;
    m_plotDefinition->setDepthZoomMinMax(minimumDepth + delta, maximumDepth + delta);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RiuWellLogPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

