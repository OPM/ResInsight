/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"

#include "RigFlowDiagSolverInterface.h"

#include "cvfBase.h"
#include "cvfAssert.h"
//#include "cvfTrace.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_legend.h"
#include "qwt_symbol.h"
#include "qwt_plot_marker.h"

#include <QDockWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>

#include <algorithm>
#include <cmath>



//==================================================================================================
//
//
//
//==================================================================================================
class RelPermQwtPlot : public QwtPlot
{
public:
    RelPermQwtPlot(QWidget* parent) : QwtPlot(parent) {}
    virtual QSize sizeHint() const { return QSize(100, 100); }
    virtual QSize minimumSizeHint() const { return QSize(0, 0); }
};





//==================================================================================================
///
/// \class RiuRelativePermeabilityPlotPanel
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::RiuRelativePermeabilityPlotPanel(QDockWidget* parent)
:   QWidget(parent),
    m_swat(HUGE_VAL),
    m_sgas(HUGE_VAL),
    m_plotUpdater(new RiuRelativePermeabilityPlotUpdater(this))
{
    m_qwtPlot = new RelPermQwtPlot(this);
    setPlotDefaults(m_qwtPlot);

    m_selectedCurvesButtonGroup = new QButtonGroup(this);
    m_selectedCurvesButtonGroup->setExclusive(false);

    m_selectedCurvesButtonGroup->addButton(new QCheckBox("KRW"),  RigFlowDiagSolverInterface::RelPermCurve::KRW);
    m_selectedCurvesButtonGroup->addButton(new QCheckBox("KRG"),  RigFlowDiagSolverInterface::RelPermCurve::KRG);
    m_selectedCurvesButtonGroup->addButton(new QCheckBox("KROW"), RigFlowDiagSolverInterface::RelPermCurve::KROW);
    m_selectedCurvesButtonGroup->addButton(new QCheckBox("KROG"), RigFlowDiagSolverInterface::RelPermCurve::KROG);
    m_selectedCurvesButtonGroup->addButton(new QCheckBox("PCOW"), RigFlowDiagSolverInterface::RelPermCurve::PCOW);
    m_selectedCurvesButtonGroup->addButton(new QCheckBox("PCOG"), RigFlowDiagSolverInterface::RelPermCurve::PCOG);

    QGroupBox* groupBox = new QGroupBox("Curves");
    QVBoxLayout* groupBoxLayout = new QVBoxLayout;
    groupBox->setLayout(groupBoxLayout);

    QList<QAbstractButton*> checkButtonList = m_selectedCurvesButtonGroup->buttons();
    for (int i = 0; i < checkButtonList.size(); i++)
    {
        checkButtonList[i]->setChecked(true);
        groupBoxLayout->addWidget(checkButtonList[i]);
    }

    m_showUnscaledCheckBox = new QCheckBox("Show Unscaled");

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(groupBox);
    leftLayout->addWidget(m_showUnscaledCheckBox);
    leftLayout->addStretch(1);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(m_qwtPlot);
    mainLayout->setContentsMargins(5, 0, 0, 0);

    setLayout(mainLayout);

    connect(m_selectedCurvesButtonGroup, SIGNAL(buttonClicked(int)), SLOT(slotButtonInButtonGroupClicked(int)));
    connect(m_showUnscaledCheckBox, SIGNAL(stateChanged(int)), SLOT(slotUnscaledCheckBoxStateChanged(int)));

    plotUiSelectedCurves();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::~RiuRelativePermeabilityPlotPanel()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::setPlotDefaults(QwtPlot* plot)
{
    RiuSummaryQwtPlot::setCommonPlotBehaviour(plot);

    plot->enableAxis(QwtPlot::xBottom, true);
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->enableAxis(QwtPlot::xTop, false);
    plot->enableAxis(QwtPlot::yRight, false);

    plot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 3);

    QwtLegend* legend = new QwtLegend(plot);
    plot->insertLegend(legend, QwtPlot::BottomLegend);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::setPlotData(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves, double swat, double sgas, QString cellReferenceText)
{
    //cvf::Trace::show("Set RelPerm plot data");

    m_allCurvesArr = relPermCurves;
    m_swat = swat;
    m_sgas = sgas;
    m_cellReferenceText = cellReferenceText;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::clearPlot()
{
    //cvf::Trace::show("Clear RelPerm plot data");

    if (m_allCurvesArr.empty() && m_cellReferenceText.isEmpty())
    {
        return;
    }

    m_allCurvesArr.clear();
    m_swat = HUGE_VAL;
    m_sgas = HUGE_VAL;
    m_cellReferenceText.clear();

    plotCurvesInQwt(m_allCurvesArr, m_swat, m_sgas, m_cellReferenceText, m_qwtPlot, &m_myPlotMarkers);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotUpdater* RiuRelativePermeabilityPlotPanel::plotUpdater()
{
    return m_plotUpdater.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotUiSelectedCurves()
{
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> selectedCurves;

    // Determine which curves to actually plot based on selection in GUI
    const RigFlowDiagSolverInterface::RelPermCurve::EpsMode epsModeToShow = m_showUnscaledCheckBox->isChecked() ? RigFlowDiagSolverInterface::RelPermCurve::EPS_OFF : RigFlowDiagSolverInterface::RelPermCurve::EPS_ON;

    for (size_t i = 0; i < m_allCurvesArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::RelPermCurve::Ident curveIdent = m_allCurvesArr[i].ident;
        const RigFlowDiagSolverInterface::RelPermCurve::EpsMode curveEpsMode = m_allCurvesArr[i].epsMode;

        if (curveEpsMode == epsModeToShow) {
            if (m_selectedCurvesButtonGroup->button(curveIdent) && m_selectedCurvesButtonGroup->button(curveIdent)->isChecked())
            {
                selectedCurves.push_back(m_allCurvesArr[i]);
            }
        }
    }

    plotCurvesInQwt(selectedCurves, m_swat, m_sgas, m_cellReferenceText, m_qwtPlot, &m_myPlotMarkers);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr, double swat, double sgas, QString cellReferenceText, QwtPlot* plot, std::vector<QwtPlotMarker*>* myPlotMarkers)
{
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    
    // Workaround for detaching only plot markers that we have added
    // Needed as long as the curve point tracker is also using plot markers for its marking
    //plot->detachItems(QwtPlotItem::Rtti_PlotMarker);
    for (QwtPlotMarker* marker : *myPlotMarkers)
    {
        marker->detach();
        delete marker;
    }
    myPlotMarkers->clear();


    bool shouldEableRightYAxis = false;

    for (size_t i = 0; i < curveArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::RelPermCurve& curve = curveArr[i];
        QwtPlotCurve* qwtCurve = new QwtPlotCurve(curve.name.c_str());

        CVF_ASSERT(curve.xVals.size() == curve.yVals.size());
        qwtCurve->setSamples(curve.xVals.data(), curve.yVals.data(), static_cast<int>(curve.xVals.size()));

        qwtCurve->setTitle(curve.name.c_str());

        qwtCurve->setStyle(QwtPlotCurve::Lines);

        bool plotCurveOnRightAxis = false;
        Qt::PenStyle penStyle = Qt::SolidLine;
        QColor clr = Qt::magenta;
        switch (curve.ident)
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:   clr = QColor(0, 0, 200); break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:  clr = QColor(0, 0, 200); break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:  clr = QColor(0, 130, 175); penStyle = Qt::DashLine; plotCurveOnRightAxis = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KRG:   clr = QColor(200, 0, 0); break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:  clr = QColor(200, 0, 0); break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:  clr = QColor(225, 110, 0); penStyle = Qt::DashLine; plotCurveOnRightAxis = true; break;
        }

        const QPen curvePen(clr, 1, penStyle);
        qwtCurve->setPen(curvePen);

        QwtSymbol* curveSymbol = new QwtSymbol(QwtSymbol::Ellipse);
        curveSymbol->setSize(6, 6);
        curveSymbol->setPen(clr);
        curveSymbol->setBrush(Qt::NoBrush);
        qwtCurve->setSymbol(curveSymbol);

        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowBrush, true);

        qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        if (plotCurveOnRightAxis)
        {
            qwtCurve->setYAxis(QwtPlot::yRight);
            shouldEableRightYAxis = true;
        }

        qwtCurve->attach(plot);


        // Add markers to indicate where SWAT and/or SGAS saturation intersects the respective curves
        if (swat != HUGE_VAL)
        {
            if (curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KRW ||
                curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KROW ||
                curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOW)
            {
                addCurveConstSaturationIntersectionMarker(curve, swat, Qt::blue, plotCurveOnRightAxis, plot, myPlotMarkers);
            }
        }
        if (sgas != HUGE_VAL)
        {
            if (curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KRG ||
                curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KROG ||
                curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOG)
            {
                addCurveConstSaturationIntersectionMarker(curve, sgas, Qt::red, plotCurveOnRightAxis, plot, myPlotMarkers);
            }
        }
    }


    // Add vertical marker lines to indicate cell SWAT and/or SGAS saturations
    if (swat != HUGE_VAL)
    {
        addVerticalSaturationMarkerLine(swat, "SWAT", Qt::blue, plot, myPlotMarkers);
    }
    if (sgas != HUGE_VAL)
    {
        addVerticalSaturationMarkerLine(sgas, "SGAS", Qt::red, plot, myPlotMarkers);
    }


    QString titleStr = "Relative Permeability";
    if (!cellReferenceText.isEmpty())
    {
        titleStr += ", " + cellReferenceText;
    }
    plot->setTitle(titleStr);

    plot->setAxisTitle(QwtPlot::xBottom, determineXAxisTitleFromCurveCollection(curveArr));
    plot->setAxisTitle(QwtPlot::yLeft, "Kr");
    plot->setAxisTitle(QwtPlot::yRight, "Pc");

    plot->enableAxis(QwtPlot::yRight, shouldEableRightYAxis);

    plot->replot();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotPanel::determineXAxisTitleFromCurveCollection(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr)
{
    bool sawWater = false;
    bool sawGas = false;

    for (RigFlowDiagSolverInterface::RelPermCurve curve : curveArr)
    {
        switch (curve.ident)
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:   sawWater = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:  sawWater = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:  sawWater = true; break;

            case RigFlowDiagSolverInterface::RelPermCurve::KRG:   sawGas = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:  sawGas = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:  sawGas = true; break;
        }
    }

    QString title = "";
    if      (sawWater && sawGas) title = "Water/Gas ";
    else if (sawWater)           title = "Water ";
    else if (sawGas)             title = "Gas ";

    title += "Saturation";

    return title;
}

//--------------------------------------------------------------------------------------------------
/// Add a vertical labeled marker line at the specified saturation value
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::addVerticalSaturationMarkerLine(double saturationValue, QString label, QColor color, QwtPlot* plot, std::vector<QwtPlotMarker*>* myPlotMarkers)
{
    QwtPlotMarker* lineMarker = new QwtPlotMarker;
    lineMarker->setXValue(saturationValue);
    lineMarker->setLineStyle(QwtPlotMarker::VLine);
    lineMarker->setLinePen(QPen(color, 1, Qt::DotLine));
    lineMarker->setLabel(label);
    lineMarker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
    lineMarker->setLabelOrientation(Qt::Vertical);

    lineMarker->attach(plot);
    myPlotMarkers->push_back(lineMarker);
}

//--------------------------------------------------------------------------------------------------
/// Add a marker at the intersection of the passed curve and the constant saturation value
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::addCurveConstSaturationIntersectionMarker(const RigFlowDiagSolverInterface::RelPermCurve& curve, double saturationValue, QColor markerColor, bool plotCurveOnRightAxis, QwtPlot* plot, std::vector<QwtPlotMarker*>* myPlotMarkers)
{
    const double yVal = interpolatedCurveYValue(curve.xVals, curve.yVals, saturationValue);
    if (yVal != HUGE_VAL)
    {
        QwtPlotMarker* pointMarker = new QwtPlotMarker;
        pointMarker->setValue(saturationValue, yVal);

        QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse);
        symbol->setSize(13, 13);
        symbol->setPen(QPen(markerColor, 2));
        symbol->setBrush(Qt::NoBrush);
        pointMarker->setSymbol(symbol);
        pointMarker->attach(plot);

        if (plotCurveOnRightAxis)
        {
            pointMarker->setYAxis(QwtPlot::yRight);
        }

        myPlotMarkers->push_back(pointMarker);
    }
}

//--------------------------------------------------------------------------------------------------
/// Assumes that all the x-values are ordered in increasing order
//--------------------------------------------------------------------------------------------------
double RiuRelativePermeabilityPlotPanel::interpolatedCurveYValue(const std::vector<double>& xVals, const std::vector<double>& yVals, double x)
{
    if (xVals.size() == 0) return HUGE_VAL;
    if (x < xVals.front()) return HUGE_VAL;
    if (x > xVals.back()) return HUGE_VAL;

    // Find first element greater or equal to the passed x-value
    std::vector<double>::const_iterator it = std::upper_bound(xVals.begin(), xVals.end(), x);

    // Due to checks above, we should never come up empty, but to safeguard against NaNs etc
    if (it == xVals.end())
    {
        return HUGE_VAL;
    }

    // Corner case - exact match on first element
    if (it == xVals.begin())
    {
        return yVals.front();
    }

    const size_t idx1 = it - xVals.begin();
    CVF_ASSERT(idx1 > 0);
    const size_t idx0 = idx1 - 1;

    const double x0 = xVals[idx0];
    const double y0 = yVals[idx0];
    const double x1 = xVals[idx1];
    const double y1 = yVals[idx1];
    CVF_ASSERT(x1 > x0);

    const double t = (x1 - x0) > 0 ? (x - x0)/(x1 - x0) : 0;
    const double y = y0 + t*(y1 - y0);

    return y;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotButtonInButtonGroupClicked(int)
{
    plotUiSelectedCurves();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotUnscaledCheckBoxStateChanged(int)
{
    plotUiSelectedCurves();
}

