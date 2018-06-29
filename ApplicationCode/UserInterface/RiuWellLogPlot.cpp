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

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuWellLogTrack.h"

#include "cafSelectionManager.h"
#include "cafCmdFeatureMenuBuilder.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_layout.h"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot(RimWellLogPlot* plotDefinition, QWidget* parent)
    : QWidget(parent), m_scheduleUpdateChildrenLayoutTimer(nullptr)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    setPalette(newPalette);

    setAutoFillBackground(true);

    m_plotTitle = new QLabel("PLOT TITLE HERE", this);
    QFont font = m_plotTitle->font();
    font.setPointSize(14);
    font.setBold(true);
    m_plotTitle->setFont(font);
    m_plotTitle->hide();
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
    int legendColumns = 1;
    if (m_plotDefinition->areTrackLegendsHorizontal())
    {
        legendColumns = 0; // unlimited
    }
    legend->setMaxColumns(legendColumns);
    
    legend->horizontalScrollBar()->setVisible(false);
    legend->verticalScrollBar()->setVisible(false);

    legend->connect(trackPlot, SIGNAL(legendDataChanged(const QVariant &, const QList< QwtLegendData > &)), SLOT(updateLegend(const QVariant &, const QList< QwtLegendData > &)));
    legend->contentsWidget()->layout()->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    m_legends.insert(static_cast<int>(index), legend);

    this->connect(trackPlot,  SIGNAL(legendDataChanged(const QVariant &, const QList< QwtLegendData > &)), SLOT(scheduleUpdateChildrenLayout()));
 
    if (!m_plotDefinition->areTrackLegendsVisible())
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
    trackPlot->setParent(nullptr);

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
    RiuPlotMainWindow* plotWindow = RiaApplication::instance()->getOrCreateMainPlotWindow();
    QMdiSubWindow* mdiWindow = plotWindow->findMdiSubWindow(this);
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
void RiuWellLogPlot::setPlotTitle(const QString& plotTitle)
{
    m_plotTitle->setText(plotTitle);
    this->updateChildrenLayout();
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
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem(ownerPlotDefinition());

    menuBuilder << "RicShowPlotDataFeature";
    menuBuilder << "RicShowContributingWellsFromPlotFeature";

    menuBuilder.appendToMenu(&menu);

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
    m_plotDefinition->setDepthAutoZoom(false);
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
std::map<int, int> RiuWellLogPlot::calculateTrackWidths(int frameWidth)
{
    int trackCount = m_trackPlots.size();
    
    int visibleTrackCount = 0;
    int firstTrackAxisOffset = 0; // Account for first track having the y-axis labels and markers
    for (int tIdx = 0; tIdx < trackCount; ++tIdx)
    {
        if (m_trackPlots[tIdx]->isVisible())
        {
            if (visibleTrackCount == 0)
            {
                firstTrackAxisOffset = static_cast<int>(m_trackPlots[tIdx]->plotLayout()->canvasRect().left());
            }
            ++visibleTrackCount;            
        }
    }

    int scrollBarWidth = 0;
    if (m_scrollBar->isVisible()) scrollBarWidth = m_scrollBar->sizeHint().width();
    
    std::map<int, int> trackWidths;

    if (visibleTrackCount)
    {
        int totalTrackWidth = (frameWidth - firstTrackAxisOffset - scrollBarWidth);
        int trackWidthExtra = (frameWidth - firstTrackAxisOffset - scrollBarWidth) % visibleTrackCount;

        int totalWidthWeights = 0;
        for (int tIdx = 0; tIdx < trackCount; ++tIdx)
        {
            if (m_trackPlots[tIdx]->isVisible())
            {
                totalWidthWeights += m_trackPlots[tIdx]->widthScaleFactor();
            }
        }

        bool firstVisible = true;
        for (int tIdx = 0; tIdx < trackCount; ++tIdx)
        {
            if (m_trackPlots[tIdx]->isVisible())
            {
                int realTrackWidth = (totalTrackWidth * m_trackPlots[tIdx]->widthScaleFactor()) / totalWidthWeights;
                
                if (firstVisible)
                {
                    realTrackWidth += firstTrackAxisOffset;
                    firstVisible = false;
                }

                if (trackWidthExtra > 0)
                {
                    realTrackWidth += 1;
                    --trackWidthExtra;
                }

                trackWidths[tIdx] = realTrackWidth;
            }
        }
    }

    return trackWidths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::placeChildWidgets(int frameHeight, int frameWidth)
{
    CVF_ASSERT(m_legends.size() == m_trackPlots.size());

    positionTitle(frameWidth);

    const int trackPadding = 4;

    std::map<int, int> trackWidths = calculateTrackWidths(frameWidth);
    size_t visibleTrackCount = trackWidths.size();

    int maxLegendHeight = 0;

    if (m_plotDefinition && m_plotDefinition->areTrackLegendsVisible())
    {
        for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
        {
            if ( m_trackPlots[tIdx]->isVisible() )
            {
                int legendHeight = m_legends[tIdx]->heightForWidth(trackWidths[tIdx] - 2 * trackPadding);
                if ( legendHeight > maxLegendHeight ) maxLegendHeight = legendHeight;
            }
        }
    }

    int titleHeight = 0;
    if (m_plotTitle && m_plotTitle->isVisible())
    {
        titleHeight = m_plotTitle->height() + 10;
    }

    int trackHeight = frameHeight - maxLegendHeight - titleHeight;
    int trackX = 0;

    if (visibleTrackCount)
    {
 
        int maxCanvasOffset = 0;
        for (int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx)
        {
            if (m_trackPlots[tIdx]->isVisible())
            {
                // Hack to align QWT plots. See below.
                QRectF canvasRect = m_trackPlots[tIdx]->plotLayout()->canvasRect();
                maxCanvasOffset = std::max(maxCanvasOffset, static_cast<int>(canvasRect.top()));
            }
        }


        for (int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx)
        {
            if (m_trackPlots[tIdx]->isVisible())
            {
                int adjustedVerticalPosition = titleHeight + maxLegendHeight + 10;
                int adjustedTrackHeight = trackHeight;
                {
                    // Hack to align QWT plots which doesn't have an x-axis with the other tracks.
                    // Since they are missing the axis, QWT will shift them upwards.
                    // So we shift the plot downwards and resize to match the others.
                    // TODO: Look into subclassing QwtPlotLayout instead.
                    QRectF canvasRect = m_trackPlots[tIdx]->plotLayout()->canvasRect();
                    int myCanvasOffset = static_cast<int>(canvasRect.top());
                    int myMargins = m_trackPlots[tIdx]->plotLayout()->canvasMargin(QwtPlot::xTop);
                    int canvasShift = std::max(0, maxCanvasOffset - myCanvasOffset);
                    adjustedVerticalPosition += canvasShift - myMargins;
                    adjustedTrackHeight -= canvasShift;
                }

                int realTrackWidth = trackWidths[tIdx];
                m_legends[tIdx]->setGeometry(trackX + trackPadding, titleHeight, realTrackWidth - 2 * trackPadding, maxLegendHeight);
                m_trackPlots[tIdx]->setGeometry(trackX + trackPadding, adjustedVerticalPosition, realTrackWidth - 2 * trackPadding, adjustedTrackHeight);

                trackX += realTrackWidth;
            }
        }
    }

    if (m_scrollBar->isVisible())
    {
        m_scrollBar->setGeometry(trackX, titleHeight + maxLegendHeight, m_scrollBar->sizeHint().width(), trackHeight);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::positionTitle(int frameWidth)
{
    if (m_plotDefinition && m_plotDefinition->isPlotTitleVisible())
    {
        int textWidth = m_plotTitle->sizeHint().width();
        m_plotTitle->setGeometry(frameWidth/2 - textWidth/2, 0, textWidth, m_plotTitle->sizeHint().height());
        m_plotTitle->show();
    }
    else
    {
        m_plotTitle->hide();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateChildrenLayout()
{ 
    int trackCount = m_trackPlots.size();
    int numTracksAlreadyShown = 0;
    for (int tIdx = 0; tIdx < trackCount; ++tIdx)
    {
        if (m_trackPlots[tIdx]->isVisible())
        {
            int legendColumns = 1;
            if (m_plotDefinition->areTrackLegendsHorizontal())
            {
                legendColumns = 0; // unlimited
            }
            m_legends[tIdx]->setMaxColumns(legendColumns);
            m_legends[tIdx]->show();

            m_trackPlots[tIdx]->enableVerticalAxisLabelsAndTitle(numTracksAlreadyShown == 0);
            numTracksAlreadyShown++;
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
        m_scheduleUpdateChildrenLayoutTimer->start(100);
    }
}
