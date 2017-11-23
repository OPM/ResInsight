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
    m_sgas(HUGE_VAL)
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

    //new RiuQwtPlotWheelZoomer(plot);

    //{
    //    // Rubber-band zoom
    //    RiuQwtPlotZoomer* plotZoomer = new RiuQwtPlotZoomer(plot->canvas());
    //    plotZoomer->setRubberBandPen(QColor(Qt::black));
    //    plotZoomer->setTrackerMode(QwtPicker::AlwaysOff);
    //    plotZoomer->setTrackerPen(QColor(Qt::black));
    //    plotZoomer->initMousePattern(1);
    //}
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

    plotCurvesInQwt(m_allCurvesArr, m_swat, m_sgas, m_cellReferenceText, m_qwtPlot);
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

    plotCurvesInQwt(selectedCurves, m_swat, m_sgas, m_cellReferenceText, m_qwtPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr, double swat, double sgas, QString cellReferenceText, QwtPlot* plot)
{
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    plot->detachItems(QwtPlotItem::Rtti_PlotMarker);

    bool enableRightYAxis = false;

    for (size_t i = 0; i < curveArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::RelPermCurve& curve = curveArr[i];
        QwtPlotCurve* qwtCurve = new QwtPlotCurve(curve.name.c_str());

        CVF_ASSERT(curve.xVals.size() == curve.yVals.size());
        qwtCurve->setSamples(curve.xVals.data(), curve.yVals.data(), static_cast<int>(curve.xVals.size()));

        qwtCurve->setTitle(curve.name.c_str());

        qwtCurve->setStyle(QwtPlotCurve::Lines);

        bool plotCurveOnRightAxis = false;
        QColor curveClr = Qt::magenta;
        switch (curve.ident)
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:   curveClr = Qt::blue; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KRG:   curveClr = Qt::red; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:  curveClr = QColor(0, 130, 175); break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:  curveClr = QColor(225, 110, 0); break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:  curveClr = QColor(0, 130, 175);   plotCurveOnRightAxis = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:  curveClr = QColor(225, 110, 0);   plotCurveOnRightAxis = true; break;
        }

        QPen curvePen;
        curvePen.setColor(curveClr);
        qwtCurve->setPen(curvePen);

        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowBrush, true);

        qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        if (plotCurveOnRightAxis)
        {
            QwtSymbol* curveSymbol = new QwtSymbol(QwtSymbol::Ellipse);
            curveSymbol->setSize(10, 10);
            curveSymbol->setPen(curvePen);
            curveSymbol->setBrush(Qt::NoBrush);
            qwtCurve->setSymbol(curveSymbol);

            qwtCurve->setYAxis(QwtPlot::yRight);
            enableRightYAxis = true;
        }

        qwtCurve->attach(plot);
    }


    if (swat != HUGE_VAL)
    {
        QwtPlotMarker* marker = new QwtPlotMarker("SWAT");
        marker->setXValue(swat);
        marker->setLineStyle(QwtPlotMarker::VLine);
        marker->setLinePen(QPen(Qt::blue, 1, Qt::DashLine));
        marker->attach(plot);
    }

    if (sgas != HUGE_VAL)
    {
        QwtPlotMarker* marker = new QwtPlotMarker("SGAS");
        marker->setXValue(sgas);
        marker->setLineStyle(QwtPlotMarker::VLine);
        marker->setLinePen(QPen(Qt::red, 1, Qt::DashLine));
        marker->attach(plot);
    }


    QString titleStr = "Relative Permeability";
    if (!cellReferenceText.isEmpty())
    {
        titleStr += ", " + cellReferenceText;
    }
    plot->setTitle(titleStr);

    plot->setAxisTitle(QwtPlot::xBottom, "Saturation");
    plot->setAxisTitle(QwtPlot::yLeft, "Kr");
    plot->setAxisTitle(QwtPlot::yRight, "Pc");

    plot->enableAxis(QwtPlot::yRight, enableRightYAxis);

    plot->replot();

    //plot->setAxisScale(QwtPlot::xBottom, 0, 1);
    //plot->setAxisScale(QwtPlot::yLeft, 0, 1);
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

