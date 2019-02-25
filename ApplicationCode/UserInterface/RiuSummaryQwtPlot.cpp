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
#include "RiuSummaryQwtPlot.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuCvfOverlayItemWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuRimQwtPlotCurve.h"
#include "RiuWidgetDragger.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtScalePicker.h"

#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafSelectionManager.h"
#include "cafTitledOverlayFrame.h"

#include "qwt_interval.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"

#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QWheelEvent>



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class EnsembleCurveInfoTextProvider : public IPlotCurveInfoTextProvider
{
public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QString curveInfoText(QwtPlotCurve* curve) override
    {
        RiuRimQwtPlotCurve* riuCurve = dynamic_cast<RiuRimQwtPlotCurve*>(curve);
        RimSummaryCurve*    sumCurve = nullptr;
        if (riuCurve)
        {
            sumCurve = dynamic_cast<RimSummaryCurve*>(riuCurve->ownerRimCurve());
        }

        return sumCurve && sumCurve->summaryCaseY() ? sumCurve->summaryCaseY()->caseName() : "";
    }
};
static EnsembleCurveInfoTextProvider ensembleCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::RiuSummaryQwtPlot(RimViewWindow* viewWindow, QWidget* parent /*= nullptr*/)
    : RiuQwtPlot(viewWindow, parent)
{
    setDefaults();
    new RiuQwtCurvePointTracker(this, true, &ensembleCurveInfoTextProvider);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useDateBasedTimeAxis()
{
    RiuQwtPlotTools::enableDateBasedBottomXAxis(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());
    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::addOrUpdateEnsembleCurveSetLegend(RimEnsembleCurveSet* curveSetToShowLegendFor)
{
    RiuCvfOverlayItemWidget* overlayWidget = nullptr;

    auto it = m_ensembleLegendWidgets.find(curveSetToShowLegendFor);
    if (it == m_ensembleLegendWidgets.end() || it->second == nullptr)
    {
        overlayWidget = new RiuCvfOverlayItemWidget(this);

        new RiuWidgetDragger(overlayWidget);

        m_ensembleLegendWidgets[curveSetToShowLegendFor] = overlayWidget;
    }
    else
    {
        overlayWidget = it->second;
    }

    if (overlayWidget)
    {
        caf::TitledOverlayFrame* overlyItem = curveSetToShowLegendFor->legendConfig()->titledOverlayFrame();
        overlyItem->setRenderSize(overlyItem->preferredSize());

        overlayWidget->updateFromOverlyItem(curveSetToShowLegendFor->legendConfig()->titledOverlayFrame());
        overlayWidget->show();
    }

    this->updateEnsembleLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::removeEnsembleCurveSetLegend(RimEnsembleCurveSet* curveSetToShowLegendFor)
{
    auto it = m_ensembleLegendWidgets.find(curveSetToShowLegendFor);
    if (it != m_ensembleLegendWidgets.end())
    {
        if (it->second != nullptr) it->second->deleteLater();

        m_ensembleLegendWidgets.erase(it);
    }

    this->updateEnsembleLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::keyPressEvent(QKeyEvent* keyEvent)
{
    RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>(ownerPlotDefinition());

    if (summaryPlot && summaryPlot->summaryCurveCollection())
    {
        RimSummaryCurveCollection* curveColl = summaryPlot->summaryCurveCollection();
        curveColl->handleKeyPressEvent(keyEvent);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem(ownerViewWindow());

    menuBuilder << "RicShowPlotDataFeature";

    menuBuilder.appendToMenu(&menu);

    if (menu.actions().size() > 0)
    {
        menu.exec(event->globalPos());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setDefaults()
{
    useDateBasedTimeAxis();

    // The legend will be deleted in the destructor of the plot or when
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend(this);
    this->insertLegend(legend, BottomLegend);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateLayout()
{
    QwtPlot::updateLayout();
    updateEnsembleLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateEnsembleLegendLayout()
{
    const int spacing      = 5;
    int       startMarginX = this->canvas()->pos().x() + spacing;
    int       startMarginY = this->canvas()->pos().y() + spacing;

    int xpos           = startMarginX;
    int ypos           = startMarginY;
    int maxColumnWidth = 0;

    RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>(ownerPlotDefinition());

    if (!summaryPlot || !summaryPlot->ensembleCurveSetCollection()) return;

    for (RimEnsembleCurveSet* curveSet : summaryPlot->ensembleCurveSetCollection()->curveSets())
    {
        auto pairIt = m_ensembleLegendWidgets.find(curveSet);
        if (pairIt != m_ensembleLegendWidgets.end())
        {
            if (ypos + pairIt->second->height() + spacing > this->canvas()->height())
            {
                xpos += spacing + maxColumnWidth;
                ypos           = startMarginY;
                maxColumnWidth = 0;
            }

            RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;
            overlayWidget->move(xpos, ypos);

            ypos += pairIt->second->height() + spacing;
            maxColumnWidth = std::max(maxColumnWidth, pairIt->second->width());
        }
    }
}
