/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RiuMohrsCirclePlot.h"

#include "RiuSelectionManager.h"

#include "RiaColorTables.h"

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "cvfAssert.h"

#include <QPainterPath>
#include <QWidget>

#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_shapeitem.h"

//==================================================================================================
///
/// \class RiuMohrsCirclePlot
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::RiuMohrsCirclePlot(QWidget* parent)
    : QwtPlot(parent)
{
    setDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::~RiuMohrsCirclePlot()
{
    deleteCircles();

    delete m_rescaler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setPrincipals(double p1, double p2, double p3)
{
    CVF_ASSERT(p1 > p2);
    CVF_ASSERT(p2 > p3);

    m_principal1 = p1;
    m_principal2 = p2;
    m_principal3 = p3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setPrincipalsAndRedrawCircles(double p1, double p2, double p3)
{
    setPrincipals(p1, p2, p3);

    redrawCircles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::updateOnSelectionChanged(const RiuSelectionItem* selectionItem)
{
    const RiuGeoMechSelectionItem* geoMechSelectionItem = dynamic_cast<const RiuGeoMechSelectionItem*>(selectionItem);

    if (!geoMechSelectionItem)
    {
        this->clearPlot();
        return;
    }
    
    RimGeoMechView* geoMechView = geoMechSelectionItem->m_view;
    CVF_ASSERT(geoMechView);

    if (this->isVisible())
    {
        const size_t gridIndex = geoMechSelectionItem->m_gridIndex;
        const size_t cellIndex = geoMechSelectionItem->m_cellIndex;
        
        queryDataAndUpdatePlot(geoMechView, gridIndex, cellIndex);
    }
    else
    {
        this->clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::clearPlot()
{
    deleteCircles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::sizeHint() const
{
    return QSize(100, 100);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::minimumSizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::redrawCircles()
{
    deleteCircles();
    createMohrCircles();

    caf::ColorTable colors = RiaColorTables::mohrsCirclePaletteColors();

    for (size_t i = 0; i < m_mohrCircles.size(); i++)
    {
        MohrCircle*       circle   = &m_mohrCircles[i];
        QwtPlotShapeItem* plotItem = new QwtPlotShapeItem("Circle");

        QPainterPath* circleDrawing = new QPainterPath();
        QPointF       center(circle->centerX, 0);
        circleDrawing->addEllipse(center, circle->radius, circle->radius);

        plotItem->setPen(QPen(colors.cycledQColor(i)));
        plotItem->setShape(*circleDrawing);
        plotItem->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        m_circlePlotItems.push_back(plotItem);
        plotItem->attach(this);
    }

    double yHeight = 0.6 * (m_principal1 - m_principal3);
    this->setAxisScale(QwtPlot::yLeft, -yHeight, yHeight);

    // Scale the x-axis to show the y-axis if the largest circle's leftmost intersection of the
    // x-axis (principal 3) is to the right of the y-axis

    //The following examples shows the largest of the three Mohr circles

    // Ex 1: xMin will be set to 1.1 * m_principal3 to be able to see the whole circle
    //      |y
    //     _|____
    //    / |     \
    //   /  |      \
    //--|---|-------|------- x
    //   \  |      /
    //    \_|_____/
    //      |
    //      |

    // Ex 2: xMin will be set to -1 to be able to see the y-axis
    //  |y              
    //  |                _______
    //  |               /       \
    //  |              /         \
    // -|-------------|-----------|---------- x
    //  |              \         /
    //  |               \_______/
    //  |               
    //  |               

    double xMin;
    if (m_principal3 < 0)
    {
        xMin = 1.1 * m_principal3;
    }
    else
    {
        xMin = -1;
    }
    // When using the rescaler, xMax is ignored
    double xMax = 0;

    this->setAxisScale(QwtPlot::xBottom, xMin, xMax);

    this->replot();
    m_rescaler->rescale();
    this->plotLayout()->setAlignCanvasToScales(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deleteCircles()
{
    for (size_t i = 0; i < m_circlePlotItems.size(); i++)
    {
        m_circlePlotItems[i]->detach();
        delete m_circlePlotItems[i];
    }

    m_circlePlotItems.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::queryDataAndUpdatePlot(RimGeoMechView* geoMechView, size_t gridIndex, size_t cellIndex)
{
    CVF_ASSERT(geoMechView);

    RigFemPartResultsCollection* resultCollection = geoMechView->geoMechCase()->geoMechData()->femPartResults();

    int frameIdx = geoMechView->currentTimeStep();

    RigFemResultAddress currentAddress = geoMechView->cellResult->resultAddress();

    // TODO: All tensors are calculated everytime this function is called. FIX
    std::vector<caf::Ten3f> vertexTensors = resultCollection->tensors(currentAddress, 0, frameIdx);
    RigFemPart*             femPart       = geoMechView->geoMechCase()->geoMechData()->femParts()->part(gridIndex);

    caf::Ten3f tensorSumOfElmNodes = vertexTensors[femPart->elementNodeResultIdx((int)cellIndex, 0)];
    for (int i = 1; i < 8; i++)
    {
        tensorSumOfElmNodes = tensorSumOfElmNodes + vertexTensors[femPart->elementNodeResultIdx((int)cellIndex, i)];
    }

    caf::Ten3f elmTensor = tensorSumOfElmNodes * (1.0 / 8.0);

    cvf::Vec3f principalDirs[3];
    cvf::Vec3f elmPrincipals = elmTensor.calculatePrincipals(principalDirs);

    setPrincipalsAndRedrawCircles(elmPrincipals[0], elmPrincipals[1], elmPrincipals[2]);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setDefaults()
{
    m_rescaler = new QwtPlotRescaler(this->canvas());
    m_rescaler->setReferenceAxis(QwtPlot::yLeft);
    m_rescaler->setAspectRatio(QwtPlot::xBottom, 1.0);
    m_rescaler->setRescalePolicy(QwtPlotRescaler::Fixed);
    m_rescaler->setEnabled(true);

    enableAxis(QwtPlot::xBottom, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xTop, false);
    enableAxis(QwtPlot::yRight, false);

    QwtPlotMarker* lineXPlotMarker = new QwtPlotMarker("LineX");
    lineXPlotMarker->setLineStyle(QwtPlotMarker::HLine);
    lineXPlotMarker->setYValue(0);
    lineXPlotMarker->attach(this);

    QwtPlotMarker* lineYPlotMarker = new QwtPlotMarker("LineY");
    lineYPlotMarker->setLineStyle(QwtPlotMarker::VLine);
    lineYPlotMarker->setXValue(0);
    lineYPlotMarker->attach(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::createMohrCircles()
{
    m_mohrCircles[0].component = 2;
    m_mohrCircles[0].radius    = (m_principal1 - m_principal3) / 2.0;
    m_mohrCircles[0].centerX   = (m_principal1 + m_principal3) / 2.0;

    m_mohrCircles[1].component = 1;
    m_mohrCircles[1].radius    = (m_principal2 - m_principal3) / 2.0;
    m_mohrCircles[1].centerX   = (m_principal2 + m_principal3) / 2.0;

    m_mohrCircles[2].component = 3;
    m_mohrCircles[2].radius    = (m_principal1 - m_principal2) / 2.0;
    m_mohrCircles[2].centerX   = (m_principal1 + m_principal2) / 2.0;
}
