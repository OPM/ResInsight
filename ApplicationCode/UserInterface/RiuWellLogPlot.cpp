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

#include "RimContextCommandBuilder.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
#include "RiuWellLogTrack.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "qwt_legend.h"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot(RimWellLogPlot* plotDefinition, QWidget* parent)
    : QWidget(parent), m_scheduleUpdateChildrenLayoutTimer(NULL)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    setPalette(newPalette);

    setAutoFillBackground(true);

    m_scrollBar = new QScrollBar(this);
    m_scrollBar->setOrientation(Qt::Vertical);
    m_scrollBar->setVisible(true);

    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotSetMinDepth(int)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::~RiuWellLogPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->detachAllCurves();
        m_plotDefinition->handleMdiWindowClosed();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::addTrackPlot(RiuWellLogTrack* trackPlot)
{
    // Insert the plot to the left of the scroll bar
    insertTrackPlot(trackPlot, m_trackPlots.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::insertTrackPlot(RiuWellLogTrack* trackPlot, size_t index)
{
    trackPlot->setParent(this);
    
    m_trackPlots.insert(static_cast<int>(index), trackPlot); 

    QwtLegend* legend = new QwtLegend(this);
    legend->setMaxColumns(1);
    legend->connect(trackPlot, SIGNAL(legendDataChanged(const QVariant &, const QList< QwtLegendData > &)), SLOT(updateLegend(const QVariant &, const QList< QwtLegendData > &)));
    legend->contentsWidget()->layout()->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    m_legends.insert(static_cast<int>(index), legend);

    this->connect(trackPlot,  SIGNAL(legendDataChanged(const QVariant &, const QList< QwtLegendData > &)), SLOT(scheduleUpdateChildrenLayout()));
 
    if (!m_plotDefinition->isTrackLegendsVisible())
    {
        legend->hide();
    }

    trackPlot->updateLegend();

    if (trackPlot->isRimTrackVisible())
    {
        trackPlot->show();
    }
    else
    {
        trackPlot->hide();
    }

    modifyWidthOfContainingMdiWindow(trackPlot->width());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::removeTrackPlot(RiuWellLogTrack* trackPlot)
{
    if (!trackPlot) return;

    int windowWidthChange = - trackPlot->width();

    int trackIdx = m_trackPlots.indexOf(trackPlot);
    CVF_ASSERT(trackIdx >= 0);

    m_trackPlots.removeAt(trackIdx);
    trackPlot->setParent(NULL);

    QwtLegend* legend = m_legends[trackIdx];
    m_legends.removeAt(trackIdx);
    delete legend;

    modifyWidthOfContainingMdiWindow(windowWidthChange);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::modifyWidthOfContainingMdiWindow(int widthChange)
{
    QMdiSubWindow* mdiWindow = RiuMainWindow::instance()->findMdiSubWindow(this);
    if (mdiWindow)
    {
        if (m_trackPlots.size() == 0 && widthChange <= 0) return; // Last track removed. Leave be

        QSize subWindowSize = mdiWindow->size();
        int newWidth = 0;

        if (m_trackPlots.size() == 1 && widthChange > 0) // First track added
        {
            newWidth = widthChange;
        }
        else
        {
            newWidth = subWindowSize.width() + widthChange;
        }

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
QSize RiuWellLogPlot::sizeHint() const
{
    return QSize(1,1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    QStringList commandIds;

    caf::SelectionManager::instance()->setSelectedItem(ownerPlotDefinition());

    commandIds << "RicShowPlotDataFeature";
    commandIds << "RicShowContributingWellsFromPlotFeature";

    RimContextCommandBuilder::appendCommandsToMenu(commandIds, &menu);

    if (menu.actions().size() > 0)
    {
        menu.exec(event->globalPos());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateScrollBar(double minDepth, double maxDepth)
{
    double availableMinDepth;
    double availableMaxDepth;
    m_plotDefinition->availableDepthRange(&availableMinDepth, &availableMaxDepth);
    availableMaxDepth += 0.01*(availableMaxDepth-availableMinDepth);

    double visibleDepth = maxDepth - minDepth;

    m_scrollBar->blockSignals(true);
    {
        m_scrollBar->setRange((int) availableMinDepth, (int) ((availableMaxDepth - visibleDepth)));
        m_scrollBar->setPageStep((int) visibleDepth);
        m_scrollBar->setValue((int) minDepth);

        m_scrollBar->setVisible(true);
    }
    m_scrollBar->blockSignals(false);
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
void RiuWellLogPlot::resizeEvent(QResizeEvent *event)
{
    int height = event->size().height();
    int width  = event->size().width();

    placeChildWidgets(height, width);
    QWidget::resizeEvent(event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::placeChildWidgets(int height, int width)
{
    int trackCount = m_trackPlots.size();
    CVF_ASSERT(m_legends.size() == trackCount);

    int visibleTrackCount = 0;
    for (int tIdx = 0; tIdx < trackCount; ++tIdx)
    {
        if (m_trackPlots[tIdx]->isVisible()) ++visibleTrackCount;
    }

    int scrollBarWidth = 0;
    if (m_scrollBar->isVisible()) scrollBarWidth = m_scrollBar->sizeHint().width();

    int maxLegendHeight = 0;

    if (m_plotDefinition && m_plotDefinition->isTrackLegendsVisible())
    {
        for ( int tIdx = 0; tIdx < trackCount; ++tIdx )
        {
            if ( m_trackPlots[tIdx]->isVisible() )
            {
                int legendHeight = m_legends[tIdx]->sizeHint().height();
                if ( legendHeight > maxLegendHeight ) maxLegendHeight = legendHeight;
            }
        }
    }

    int trackHeight = height - maxLegendHeight;
    int trackX = 0;

    if (visibleTrackCount)
    {
        int trackWidth = (width - scrollBarWidth)/visibleTrackCount;
        int trackWidthExtra = (width-scrollBarWidth)%visibleTrackCount;

        for (int tIdx = 0; tIdx < trackCount; ++tIdx)
        {
            if (m_trackPlots[tIdx]->isVisible())
            {
                int realTrackWidth = trackWidth;
                if (trackWidthExtra > 0)
                {
                    realTrackWidth += 1;
                    --trackWidthExtra;
                }
                int realLegendWidth = std::max(realTrackWidth, m_legends[tIdx]->sizeHint().width()); 
                m_legends[tIdx]->setGeometry(trackX, 0, realLegendWidth, maxLegendHeight);
                m_trackPlots[tIdx]->setGeometry(trackX, maxLegendHeight, realTrackWidth, trackHeight);

                trackX += realTrackWidth;
            }
        }
    }

    if (m_scrollBar->isVisible()) m_scrollBar->setGeometry(trackX, maxLegendHeight, scrollBarWidth, trackHeight);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateChildrenLayout()
{
    int trackCount = m_trackPlots.size();
    for (int tIdx = 0; tIdx < trackCount; ++tIdx)
    {
        if (m_trackPlots[tIdx]->isVisible())
        {
           m_legends[tIdx]->show();
        }
        else
        {
            m_legends[tIdx]->hide();
        }
    }

    placeChildWidgets(this->height(), this->width());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::showEvent(QShowEvent *)
{
    updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        updateChildrenLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// Schedule an update of the widget positions
/// Will happen just a bit after the event loop is entered
/// Used to delay the positioning to after the legend widgets is actually updated.
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::scheduleUpdateChildrenLayout()
{
    if (!m_scheduleUpdateChildrenLayoutTimer) 
    {
        m_scheduleUpdateChildrenLayoutTimer = new QTimer(this);
        connect(m_scheduleUpdateChildrenLayoutTimer, SIGNAL(timeout()), this, SLOT(updateChildrenLayout()));
    }

    if (!m_scheduleUpdateChildrenLayoutTimer->isActive())
    {
        m_scheduleUpdateChildrenLayoutTimer->setSingleShot(true);
        m_scheduleUpdateChildrenLayoutTimer->start(10);
    }
}
