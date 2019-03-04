/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuGridCrossQwtPlot.h"

#include "RiuCvfOverlayItemWidget.h"
#include "RiuWidgetDragger.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RimRegularLegendConfig.h"

#include "cafTitledOverlayFrame.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGridCrossQwtPlot::RiuGridCrossQwtPlot(RimViewWindow* ownerViewWindow, QWidget* parent /*= nullptr*/)
    : RiuQwtPlot(ownerViewWindow, parent)
{    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::addOrUpdateCurveSetLegend(RimGridCrossPlotCurveSet* curveSetToShowLegendFor)
{
    RiuCvfOverlayItemWidget* overlayWidget = nullptr;

    auto it = m_legendWidgets.find(curveSetToShowLegendFor);
    if (it == m_legendWidgets.end() || it->second == nullptr)
    {
        overlayWidget = new RiuCvfOverlayItemWidget(this);

        new RiuWidgetDragger(overlayWidget);

        m_legendWidgets[curveSetToShowLegendFor] = overlayWidget;
    }
    else
    {
        overlayWidget = it->second;
    }

    if (overlayWidget)
    {
        caf::TitledOverlayFrame* overlayItem = curveSetToShowLegendFor->legendConfig()->titledOverlayFrame();
        overlayItem->setRenderSize(overlayItem->preferredSize());

        overlayWidget->updateFromOverlayItem(curveSetToShowLegendFor->legendConfig()->titledOverlayFrame());
    }

    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::removeCurveSetLegend(RimGridCrossPlotCurveSet* curveSetToShowLegendFor)
{
    auto it = m_legendWidgets.find(curveSetToShowLegendFor);
    if (it != m_legendWidgets.end())
    {
        if (it->second != nullptr) it->second->deleteLater();

        m_legendWidgets.erase(it);
    }

    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateLayout()
{
    QwtPlot::updateLayout();
    updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateLegendLayout()
{
    const int spacing      = 5;
    int       startMarginX = this->canvas()->pos().x() + spacing;
    int       startMarginY = this->canvas()->pos().y() + spacing;

    int xpos           = startMarginX;
    int ypos           = startMarginY;
    int maxColumnWidth = 0;

    RimGridCrossPlot* crossPlot = dynamic_cast<RimGridCrossPlot*>(ownerPlotDefinition());

    if (!crossPlot) return;

    std::set<QString> legendTypes;

    for (RimGridCrossPlotCurveSet* curveSet : crossPlot->curveSets())
    {
        if (!curveSet->isChecked() || !curveSet->legendConfig()->showLegend()) continue;

        auto pairIt = m_legendWidgets.find(curveSet);
        if (pairIt != m_legendWidgets.end())
        {
            RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;
            // Show only one copy of each legend type
            if (!legendTypes.count(curveSet->categoryTitle()))
            {
                if (ypos + overlayWidget->height() + spacing > this->canvas()->height())
                {
                    xpos += spacing + maxColumnWidth;
                    ypos = startMarginY;
                    maxColumnWidth = 0;
                }

                overlayWidget->show();
                overlayWidget->move(xpos, ypos);

                ypos += pairIt->second->height() + spacing;
                maxColumnWidth = std::max(maxColumnWidth, pairIt->second->width());
                legendTypes.insert(curveSet->categoryTitle());
            }
        }
    }
}
