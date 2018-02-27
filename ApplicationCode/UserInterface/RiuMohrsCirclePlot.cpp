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
#include "RiuSummaryQwtPlot.h"

#include "RiaColorTables.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "cvfAssert.h"

#include <QPainterPath>
#include <QWidget>

#include "qwt_plot_curve.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_shapeitem.h"

#include <cmath>

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
    if (isValidPrincipals(p1, p2, p3))
    {
        m_principal1 = p1;
        m_principal2 = p2;
        m_principal3 = p3;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setPrincipalsAndRedrawPlot(double p1, double p2, double p3)
{
    if (!isValidPrincipals(p1, p2, p3))
    {
        clearPlot();
        return;
    }

    setPrincipals(p1, p2, p3);

    redrawEnvelope();
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
    deleteEnvelope();

    this->replot();
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
        plotItem->attach(this);

        m_circlePlotItems.push_back(plotItem);
    }

    replotAndScaleAxis();
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

    if (m_transparentCurve)
    {
        m_transparentCurve->detach();
        delete m_transparentCurve;
        m_transparentCurve = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::redrawEnvelope()
{
    deleteEnvelope();

    if (m_cohesion == HUGE_VAL || m_frictionAngle == HUGE_VAL)
    {
        this->replot();
        return;
    }

    QwtPlotCurve* qwtCurve = new QwtPlotCurve();
    
    std::vector<double> xVals;
    std::vector<double> yVals;
    
    double tanFrictionAngle = cvf::Math::abs(cvf::Math::tan(cvf::Math::toRadians(m_frictionAngle)));

    if (tanFrictionAngle == 0 || tanFrictionAngle == HUGE_VAL)
    {
        this->replot();
        delete qwtCurve;
        return;
    }

    double x = m_cohesion/tanFrictionAngle;
    if (m_principal1 < 0)
    {
        xVals.push_back(x);
        xVals.push_back(m_principal3*1.1);
    }
    else
    {
        xVals.push_back(-x);
        xVals.push_back(m_principal1*1.1);
    }

    yVals.push_back(0);
    yVals.push_back((x + cvf::Math::abs(m_principal1) * 1.05) * tanFrictionAngle);

    qwtCurve->setSamples(xVals.data(), yVals.data(), 2);

    qwtCurve->setStyle(QwtPlotCurve::Lines);
    qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    const QPen curvePen(QColor(236, 118, 0));
    qwtCurve->setPen(curvePen);

    qwtCurve->attach(this);

    m_envolopePlotItem = qwtCurve;
    
    replotAndScaleAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deleteEnvelope()
{
    if (m_envolopePlotItem)
    {
        m_envolopePlotItem->detach();
        delete m_envolopePlotItem;
        m_envolopePlotItem = nullptr;
    }
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
    if (!(currentAddress.fieldName == "SE" || currentAddress.fieldName == "ST" || currentAddress.fieldName == "NE"))
    {
        clearPlot();
        return;
    }
    // TODO: All tensors are calculated every time this function is called. FIX
    std::vector<caf::Ten3f> vertexTensors = resultCollection->tensors(currentAddress, 0, frameIdx);
    if (vertexTensors.empty())
    {
        clearPlot();
        return;
    }
    
    setCohesion(geoMechView->geoMechCase()->cohesion());
    setFrictionAngle(geoMechView->geoMechCase()->frictionAngleDeg());

    RigFemPart* femPart = geoMechView->geoMechCase()->geoMechData()->femParts()->part(gridIndex);

    size_t i, j, k;
    femPart->structGrid()->ijkFromCellIndex(cellIndex, &i, &j, &k);

    int elmId = femPart->elmId(cellIndex);

    QString title;
    QString resultPos;
    QString fieldName = geoMechView->cellResultResultDefinition()->resultFieldUiName();

    switch (geoMechView->cellResultResultDefinition()->resultPositionType())
    {
    case RIG_ELEMENT_NODAL:
        resultPos = "Element Nodal";
        break;

    case RIG_INTEGRATION_POINT:
        resultPos = "Integration Point";
        break;
    default:
        break;
    }
    
    title += QString("%1, %2").arg(resultPos).arg(fieldName);
    
    title += QString(", Element Id[%1], ijk[%2,%3,%4]").arg(elmId).arg(i).arg(j).arg(k);
    this->setTitle(title);

    caf::Ten3f tensorSumOfElmNodes = vertexTensors[femPart->elementNodeResultIdx((int)cellIndex, 0)];
    for (int i = 1; i < 8; i++)
    {
        tensorSumOfElmNodes = tensorSumOfElmNodes + vertexTensors[femPart->elementNodeResultIdx((int)cellIndex, i)];
    }

    caf::Ten3f elmTensor = tensorSumOfElmNodes * (1.0 / 8.0);

    cvf::Vec3f principalDirs[3];
    cvf::Vec3f elmPrincipals = elmTensor.calculatePrincipals(principalDirs);

    setPrincipalsAndRedrawPlot(elmPrincipals[0], elmPrincipals[1], elmPrincipals[2]);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setDefaults()
{
    RiuSummaryQwtPlot::setCommonPlotBehaviour(this);

    m_rescaler = new QwtPlotRescaler(this->canvas());
    m_rescaler->setReferenceAxis(QwtPlot::yLeft);
    m_rescaler->setAspectRatio(QwtPlot::xBottom, 1.0);
    m_rescaler->setRescalePolicy(QwtPlotRescaler::Fixed);
    m_rescaler->setEnabled(true);

    enableAxis(QwtPlot::xBottom, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xTop, false);
    enableAxis(QwtPlot::yRight, false);

    setAxisTitle(QwtPlot::xBottom, "Effective Normal Stress");
    setAxisTitle(QwtPlot::yLeft, "Shear Stress");

    m_envolopePlotItem = nullptr;
    m_transparentCurve = nullptr;
    m_cohesion = HUGE_VAL;
    m_frictionAngle = HUGE_VAL; 
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setFrictionAngle(double frictionAngle)
{
    m_frictionAngle = frictionAngle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setCohesion(double cohesion)
{
    m_cohesion = cohesion;
}

//--------------------------------------------------------------------------------------------------
/// Add a transparent curve to make tooltip available on principals crossing the x-axis
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::updateTransparentCurveOnPrincipals()
{
    if (m_transparentCurve)
    {
        m_transparentCurve->detach();
        delete m_transparentCurve;
    }

    m_transparentCurve = new QwtPlotCurve();

    QVector<QPointF> qVectorPoints;
    
    qVectorPoints.push_back(QPointF(m_principal1, 0));
    qVectorPoints.push_back(QPointF(m_principal2, 0));
    qVectorPoints.push_back(QPointF(m_principal3, 0));

    m_transparentCurve->setSamples(qVectorPoints);
    m_transparentCurve->setYAxis(QwtPlot::yLeft);
    m_transparentCurve->setStyle(QwtPlotCurve::NoCurve);
    m_transparentCurve->setLegendAttribute(QwtPlotCurve::LegendNoAttribute);

    m_transparentCurve->attach(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::replotAndScaleAxis()
{
    double maxYEnvelope = -HUGE_VAL;
    if (m_envolopePlotItem)
    {
        maxYEnvelope = m_envolopePlotItem->maxYValue();
    }
    
    double yHeight = std::max(maxYEnvelope, 0.6 * (m_principal1 - m_principal3));
    
    this->setAxisScale(QwtPlot::yLeft, 0, yHeight);

    double minXEvelope = 0;

    if (m_envolopePlotItem)
    {
        minXEvelope = m_envolopePlotItem->minXValue();
    }

    double xMin;
    if (minXEvelope < 0)
    {
        xMin = minXEvelope;
    }
    else
    {
        xMin = 1.1 * m_principal3;
    }

    // When using the rescaler, xMax is ignored
    this->setAxisScale(QwtPlot::xBottom, xMin, 0);

    updateTransparentCurveOnPrincipals();

    this->replot();
    m_rescaler->rescale();
    this->plotLayout()->setAlignCanvasToScales(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuMohrsCirclePlot::isValidPrincipals(double p1, double p2, double p3)
{
    //Inf
    if (p1 == HUGE_VAL || p2 == HUGE_VAL || p3 == HUGE_VAL)
    {
        return false;
    }
    
    //Nan
    if ((p1 != p1) || (p2 != p2) || p3 != p3)
    {
        return false;
    }

    //Principal rules:
    if ((p1 < p2) || (p2 < p3))
    {
        return false;
    }

    return true;
}
