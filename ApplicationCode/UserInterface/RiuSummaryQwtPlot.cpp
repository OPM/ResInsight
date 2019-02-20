/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h" 
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtScalePicker.h"

#include "cafSelectionManager.h"
#include "cafCmdFeatureMenuBuilder.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"

#include <QEvent>
#include <QMenu>
#include <QWheelEvent>


#include "RiuWidgetDragger.h"
#include "RiuCvfOverlayItemWidget.h"
#include "RimEnsembleCurveSet.h"
#include "RimRegularLegendConfig.h"
#include "cafTitledOverlayFrame.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCase.h"
#include "RiuRimQwtPlotCurve.h"
#include "RimSummaryCurve.h"

#include <cfloat>


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
        RiuRimQwtPlotCurve*  riuCurve = dynamic_cast<RiuRimQwtPlotCurve*>(curve);
        RimSummaryCurve* sumCurve = nullptr;
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
RiuSummaryQwtPlot::RiuSummaryQwtPlot(RimSummaryPlot* plotDefinition, QWidget* parent) : QwtPlot(parent)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    setDefaults();

    // LeftButton for the zooming
    m_zoomerLeft = new RiuQwtPlotZoomer(canvas());
    m_zoomerLeft->setRubberBandPen(QColor(Qt::black));
    m_zoomerLeft->setTrackerMode(QwtPicker::AlwaysOff);
    m_zoomerLeft->setTrackerPen(QColor(Qt::black));
    m_zoomerLeft->initMousePattern(1);

    // Attach a zoomer for the right axis
    m_zoomerRight = new RiuQwtPlotZoomer(canvas());
    m_zoomerRight->setAxis(xTop, yRight);
    m_zoomerRight->setTrackerMode(QwtPicker::AlwaysOff);
    m_zoomerRight->initMousePattern(1);

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::MidButton);

    auto wheelZoomer = new RiuQwtPlotWheelZoomer(this);

    connect(wheelZoomer, SIGNAL(zoomUpdated()), SLOT(onZoomedSlot()));
    connect(m_zoomerLeft, SIGNAL(zoomed( const QRectF & )), SLOT(onZoomedSlot()));
    connect(panner, SIGNAL(panned( int , int  )), SLOT(onZoomedSlot()));

    RiuQwtScalePicker* scalePicker = new RiuQwtScalePicker(this);
    connect(scalePicker, SIGNAL(clicked(int, double)), this, SLOT(onAxisClicked(int, double)));

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::~RiuSummaryQwtPlot()
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
RimSummaryPlot* RiuSummaryQwtPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuSummaryQwtPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::currentVisibleWindow(QwtInterval* leftAxis, QwtInterval* rightAxis, QwtInterval* timeAxis) const
{
    *leftAxis  = axisScaleDiv(yLeft).interval();
    *rightAxis = axisScaleDiv(yRight).interval();
    *timeAxis  = axisScaleDiv(xBottom).interval();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateEnsembleLegendLayout()
{
    const int spacing = 5;
    int startMarginX = this->canvas()->pos().x() + spacing;
    int startMarginY = this->canvas()->pos().y() + spacing;

    int xpos = startMarginX;
    int ypos = startMarginY;
    int maxColumnWidth = 0; 

    if (!ownerPlotDefinition() || !ownerPlotDefinition()->ensembleCurveSetCollection()) return;

    for (RimEnsembleCurveSet * curveSet : ownerPlotDefinition()->ensembleCurveSetCollection()->curveSets())
    {
        auto pairIt = m_ensembleLegendWidgets.find(curveSet);
        if (pairIt !=  m_ensembleLegendWidgets.end())
        {
            if (ypos + pairIt->second->height() + spacing > this->canvas()->height())
            {
                xpos += spacing + maxColumnWidth;
                ypos = startMarginY;
                maxColumnWidth  = 0; 
            }

            RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;
            overlayWidget->move(xpos, ypos);

            ypos += pairIt->second->height() + spacing;
            maxColumnWidth = std::max(maxColumnWidth, pairIt->second->width());
        }
    }    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::addOrUpdateEnsembleCurveSetLegend(RimEnsembleCurveSet * curveSetToShowLegendFor)
{
   RiuCvfOverlayItemWidget* overlayWidget = nullptr;

   auto it = m_ensembleLegendWidgets.find(curveSetToShowLegendFor);
   if (it ==  m_ensembleLegendWidgets.end() || it->second == nullptr)
   {
       overlayWidget = new RiuCvfOverlayItemWidget(this);

       new RiuWidgetDragger(overlayWidget);

       m_ensembleLegendWidgets[curveSetToShowLegendFor] = overlayWidget;

   }
   else
   {
        overlayWidget = it->second;
   }

   if ( overlayWidget )
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
void RiuSummaryQwtPlot::removeEnsembleCurveSetLegend(RimEnsembleCurveSet * curveSetToShowLegendFor)
{
    auto it = m_ensembleLegendWidgets.find(curveSetToShowLegendFor);
    if ( it !=  m_ensembleLegendWidgets.end() )
    {
        if ( it->second != nullptr ) it->second->deleteLater();
 
        m_ensembleLegendWidgets.erase(it);
    }

    this->updateEnsembleLegendLayout();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuSummaryQwtPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem(ownerPlotDefinition());

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
void RiuSummaryQwtPlot::keyPressEvent(QKeyEvent* keyEvent)
{
    if (m_plotDefinition && m_plotDefinition->summaryCurveCollection())
    {
        RimSummaryCurveCollection* curveColl = m_plotDefinition->summaryCurveCollection();
        curveColl->handleKeyPressEvent(keyEvent);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuSummaryQwtPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setDefaults()
{
    setCommonPlotBehaviour(this);
    RiuQwtPlotTools::setDefaultAxes(this);

    useDateBasedTimeAxis();

    // The legend will be deleted in the destructor of the plot or when 
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend(this);
    this->insertLegend(legend, BottomLegend);
}

void RiuSummaryQwtPlot::setCommonPlotBehaviour(QwtPlot* plot)
{
    RiuQwtPlotTools::setCommonPlotBehaviour(plot);
  
    new RiuQwtCurvePointTracker(plot, true, &ensembleCurveInfoTextProvider);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useDateBasedTimeAxis()
{
    enableDateBasedBottomXAxis(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::enableDateBasedBottomXAxis(QwtPlot* plot)
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->setDateFormat(QwtDate::Year, QString("dd-MM-yyyy"));

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
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
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());
    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuSummaryQwtPlot::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == canvas())
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(mouseEvent)
        {
            if(mouseEvent->button() == Qt::LeftButton && mouseEvent->type() == QMouseEvent::MouseButtonRelease)
            {
                selectClosestCurve(mouseEvent->pos());
            }
        }
    }

    return QwtPlot::eventFilter(watched, event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::selectClosestCurve(const QPoint& pos)
{
    QwtPlotCurve* closestCurve = nullptr;
    double distMin = DBL_MAX;

    const QwtPlotItemList& itmList = itemList();
    for(QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++)
    {
        if((*it)->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>(*it);
            double dist = DBL_MAX;
            candidateCurve->closestPoint(pos, &dist);
            if(dist < distMin)
            {
                closestCurve = candidateCurve;
                distMin = dist;
            }
        }
    }

    if (closestCurve && distMin < 20)
    {
        caf::PdmObject* selectedPlotObject = m_plotDefinition->findRimPlotObjectFromQwtCurve(closestCurve);

        if (selectedPlotObject)
        {
            RimProject* proj = nullptr;
            selectedPlotObject->firstAncestorOrThisOfType(proj);

            if (proj)
            {
                RiuPlotMainWindowTools::showPlotMainWindow();
                RiuPlotMainWindowTools::selectAsCurrentItem(selectedPlotObject);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::onZoomedSlot()
{
    m_plotDefinition->updateZoomWindowFromQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::onAxisClicked(int axis, double value)
{
    if (!m_plotDefinition) return;

    m_plotDefinition->selectAxisInPropertyEditor(axis);
}
