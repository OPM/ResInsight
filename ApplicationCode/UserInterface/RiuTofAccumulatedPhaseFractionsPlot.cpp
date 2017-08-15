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

#include "RiuTofAccumulatedPhaseFractionsPlot.h"

#include "RimContextCommandBuilder.h"
#include "RimTofAccumulatedPhaseFractionsPlot.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_grid.h"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTofAccumulatedPhaseFractionsPlot::RiuTofAccumulatedPhaseFractionsPlot(RimTofAccumulatedPhaseFractionsPlot* plotDefinition, QWidget* parent)
    : QwtPlot(parent), m_watCurve(nullptr), m_oilCurve(nullptr), m_gasCurve(nullptr)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    setPalette(newPalette);

    setAutoFillBackground(true);
    setDefaults();
    QwtText title("Cumulative Saturation by Time of Flight");
    QFont titleFont = title.font();
    titleFont.setPixelSize(12);
    title.setFont(titleFont);
    setTitle(title);

    m_watCurve = new QwtPlotCurve;
    setCurveColor(m_watCurve, QColor(0, 0, 169));
    m_watCurve->setZ(0.9);
    m_watCurve->setTitle("Water");
    m_watCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    m_oilCurve = new QwtPlotCurve;
    setCurveColor(m_oilCurve, QColor(169, 0, 0));
    m_oilCurve->setZ(0.8);
    m_oilCurve->setTitle("Oil");
    m_oilCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    m_gasCurve = new QwtPlotCurve;
    setCurveColor(m_gasCurve, QColor(0, 100, 0));
    m_gasCurve->setZ(0.7);
    m_gasCurve->setTitle("Gas");
    m_gasCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    m_watCurve->attach(this);
    m_oilCurve->attach(this);
    m_gasCurve->attach(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTofAccumulatedPhaseFractionsPlot::~RiuTofAccumulatedPhaseFractionsPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleMdiWindowClosed();
    }

    if (m_watCurve)
    {
        m_watCurve->detach();
        delete m_watCurve;
        m_watCurve = nullptr;
    }

    if (m_oilCurve)
    {
        m_oilCurve->detach();
        delete m_oilCurve;
        m_oilCurve = nullptr;
    }

    if (m_gasCurve)
    {
        m_gasCurve->detach();
        delete m_gasCurve;
        m_gasCurve = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuTofAccumulatedPhaseFractionsPlot::sizeHint() const
{
    return QSize(350, 250);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuTofAccumulatedPhaseFractionsPlot::heightForWidth(int w) const
{
    return static_cast<int>(static_cast<double>(w) / 1.1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTofAccumulatedPhaseFractionsPlot* RiuTofAccumulatedPhaseFractionsPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuTofAccumulatedPhaseFractionsPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTofAccumulatedPhaseFractionsPlot::setSamples(std::vector<double> xSamples,
                                                     std::vector<double> watValues,
                                                     std::vector<double> oilValues,
                                                     std::vector<double> gasValues)
{
    m_xValues.clear();
    m_watValues.clear();
    m_oilValues.clear();
    m_gasValues.clear();

    m_watValues.swap(watValues);
    for (size_t i = 0; i < xSamples.size(); ++i)
    {
        m_xValues.push_back(xSamples[i] / 365.2425);
        m_oilValues.push_back(oilValues[i] + m_watValues[i]);
        m_gasValues.push_back(gasValues[i] + m_oilValues[i]);
    }
    m_watCurve->setSamples(m_xValues.data(), m_watValues.data(), static_cast<int>(m_xValues.size()));
    m_oilCurve->setSamples(m_xValues.data(), m_oilValues.data(), static_cast<int>(m_xValues.size()));
    m_gasCurve->setSamples(m_xValues.data(), m_gasValues.data(), static_cast<int>(m_xValues.size()));


    if (!m_xValues.empty())
    {
        double maxVal = 0;
        for (double val : m_xValues)
        {
            maxVal = std::max(val, maxVal);
        }
        setAxisScale(QwtPlot::xBottom, 0, maxVal);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTofAccumulatedPhaseFractionsPlot::setDefaults()
{
    setCommonPlotBehaviour(this);
    setAxisTitle(QwtPlot::xBottom, "Years");

    enableAxis(QwtPlot::xBottom, true);
    enableAxis(QwtPlot::yLeft, true);
    setAxisScale(QwtPlot::yLeft, 0, 1, 0.2);
    enableAxis(QwtPlot::xTop, false);
    enableAxis(QwtPlot::yRight, false);

    setAxisMaxMinor(QwtPlot::xBottom, 2);
    setAxisMaxMinor(QwtPlot::yLeft, 3);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHeightForWidth(true);
    setSizePolicy(sizePolicy);
    updateGeometry();

    // The legend will be deleted in the destructor of the plot or when 
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend(this);
    this->insertLegend(legend, BottomLegend);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTofAccumulatedPhaseFractionsPlot::setCommonPlotBehaviour(QwtPlot* plot)
{
    // Plot background and frame look

    QPalette newPalette(plot->palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    plot->setPalette(newPalette);

    plot->setAutoFillBackground(true);
    plot->setCanvasBackground(Qt::white);

    QFrame* canvasFrame = dynamic_cast<QFrame*>(plot->canvas());
    if (canvasFrame)
    {
        canvasFrame->setFrameShape(QFrame::NoFrame);
    }

    // Grid

    QwtPlotGrid* grid = new QwtPlotGrid;
    grid->attach(plot);
    QPen gridPen(Qt::SolidLine);
    gridPen.setColor(Qt::lightGray);
    grid->setPen(gridPen);

    // Axis number font
    QFont axisFont =  plot->axisFont(QwtPlot::xBottom);
    axisFont.setPixelSize(11);

    plot->setAxisFont(QwtPlot::xBottom, axisFont);
    plot->setAxisFont(QwtPlot::xTop, axisFont);
    plot->setAxisFont(QwtPlot::yLeft, axisFont);
    plot->setAxisFont(QwtPlot::yRight, axisFont);

    // Axis title font
    QwtText axisTitle = plot->axisTitle(QwtPlot::xBottom);
    QFont axisTitleFont = axisTitle.font();
    axisTitleFont.setPixelSize(11);
    axisTitleFont.setBold(false);
    axisTitle.setFont(axisTitleFont);
    axisTitle.setRenderFlags(Qt::AlignRight);

    plot->setAxisTitle(QwtPlot::xBottom, axisTitle);
    plot->setAxisTitle(QwtPlot::xTop,    axisTitle);
    plot->setAxisTitle(QwtPlot::yLeft,   axisTitle);
    plot->setAxisTitle(QwtPlot::yRight,  axisTitle);

    // Enable mouse tracking and event filter

    plot->canvas()->setMouseTracking(true);
    plot->canvas()->installEventFilter(plot);
//    plot->plotLayout()->setAlignCanvasToScales(true);

//    new RiuQwtCurvePointTracker(plot, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTofAccumulatedPhaseFractionsPlot::setCurveColor(QwtPlotCurve* curve, QColor color)
{
    curve->setBrush(QBrush(color));

    QLinearGradient gradient; 
    gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    gradient.setColorAt(0,    color.darker(110));
    gradient.setColorAt(0.15, color);
    gradient.setColorAt(0.25, color);
    gradient.setColorAt(0.4,  color.darker(110));
    gradient.setColorAt(0.6,  color);
    gradient.setColorAt(0.8,  color.darker(110));
    gradient.setColorAt(1,    color);
    curve->setBrush(gradient);
}
