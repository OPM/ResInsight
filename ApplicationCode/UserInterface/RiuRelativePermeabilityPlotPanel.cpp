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
class MyQwtPlot : public QwtPlot
{
public:
    MyQwtPlot(QWidget* parent) : QwtPlot(parent) {}
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
:   QWidget(parent)
{
    m_qwtPlot = new MyQwtPlot(this);
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

    // Uncheck and disable PCOW and PCOG until we have data for these
    checkButtonList[4]->setDisabled(true);
    checkButtonList[4]->setChecked(false);
    checkButtonList[5]->setDisabled(true);
    checkButtonList[5]->setChecked(false);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(groupBox);
    leftLayout->addStretch(1);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(m_qwtPlot);

    setLayout(mainLayout);

    connect(m_selectedCurvesButtonGroup, SIGNAL(buttonClicked(int)), SLOT(slotButtonInButtonGroupClicked(int)));

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
void RiuRelativePermeabilityPlotPanel::setPlotData(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves, QString cellReferenceText)
{
    //cvf::Trace::show("Set RelPerm plot data");

    m_allCurvesArr = relPermCurves;
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
    m_cellReferenceText.clear();

    plotCurvesInQwt(m_allCurvesArr, m_cellReferenceText, m_qwtPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotUiSelectedCurves()
{
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> selectedCurves;

    for (size_t i = 0; i < m_allCurvesArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::RelPermCurve::Ident curveIdent = m_allCurvesArr[i].ident;
        if (m_selectedCurvesButtonGroup->button(curveIdent) && m_selectedCurvesButtonGroup->button(curveIdent)->isChecked())
        {
            selectedCurves.push_back(m_allCurvesArr[i]);
        }
    }

    plotCurvesInQwt(selectedCurves, m_cellReferenceText, m_qwtPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr, QString cellReferenceText, QwtPlot* plot)
{
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);

    bool enableRightYAxis = false;

    for (size_t i = 0; i < curveArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::RelPermCurve& curve = curveArr[i];
        QwtPlotCurve* qwtCurve = new QwtPlotCurve(curve.name.c_str());

        CVF_ASSERT(curve.xVals.size() == curve.yVals.size());
        qwtCurve->setSamples(curve.xVals.data(), curve.yVals.data(), static_cast<int>(curve.xVals.size()));

        qwtCurve->setTitle(curve.name.c_str());

        qwtCurve->setStyle(QwtPlotCurve::Lines);

        QwtPlot::Axis yAxis = QwtPlot::yLeft;
        bool plotOnRightAxis = false;
        Qt::GlobalColor curveClr = Qt::magenta;
        switch (curve.ident)
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:   curveClr = Qt::blue; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KRG:   curveClr = Qt::red; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:  curveClr = Qt::darkGreen; break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:  curveClr = Qt::green; break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:  curveClr = Qt::darkGreen;   plotOnRightAxis = true; break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:  curveClr = Qt::green;       plotOnRightAxis = true; break;
        }

        QPen curvePen;
        curvePen.setColor(curveClr);
        qwtCurve->setPen(curvePen);

        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        qwtCurve->setLegendAttribute(QwtPlotCurve::LegendShowBrush, true);

        qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        if (plotOnRightAxis)
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

    QString title = "Relative Permeability";
    if (!cellReferenceText.isEmpty())
    {
        title += ", " + cellReferenceText;
    }
    plot->setTitle(title);

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

