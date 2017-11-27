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

#include "RiuPvtPlotPanel.h"
#include "RiuSummaryQwtPlot.h"

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
#include <QComboBox>
#include <QLabel>



//==================================================================================================
//
//
//
//==================================================================================================
class PvtQwtPlot : public QwtPlot
{
public:
    PvtQwtPlot(QWidget* parent) : QwtPlot(parent) {}
    virtual QSize sizeHint() const { return QSize(100, 100); }
    virtual QSize minimumSizeHint() const { return QSize(0, 0); }
};



//==================================================================================================
///
/// \class RiuPvtPlotPanel
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::RiuPvtPlotPanel(QDockWidget* parent)
:   QWidget(parent)
{
    m_phaseComboBox = new QComboBox(this);
    m_phaseComboBox->setEditable(false);
    m_phaseComboBox->addItem("Oil", QVariant(RigFlowDiagSolverInterface::PvtCurve::OIL));
    m_phaseComboBox->addItem("Gas", QVariant(RigFlowDiagSolverInterface::PvtCurve::GAS));
    m_phaseComboBox->addItem("Water", QVariant(RigFlowDiagSolverInterface::PvtCurve::WATER));

    QHBoxLayout* comboLayout = new QHBoxLayout();
    comboLayout->addWidget(new QLabel("Phase:"));
    comboLayout->addWidget(m_phaseComboBox);
    comboLayout->addStretch(1);
    comboLayout->setContentsMargins(5, 5, 0, 0);

    m_fvfPlot = new PvtQwtPlot(this);
    m_viscosityPlot = new PvtQwtPlot(this);
    setPlotDefaults(m_fvfPlot);
    setPlotDefaults(m_viscosityPlot);

    QHBoxLayout* plotLayout = new QHBoxLayout();
    plotLayout->addWidget(m_fvfPlot);
    plotLayout->addWidget(m_viscosityPlot);
    plotLayout->setSpacing(0);
    plotLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(comboLayout);
    mainLayout->addLayout(plotLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);

    connect(m_phaseComboBox, SIGNAL(currentIndexChanged(int)), SLOT(slotPhaseComboCurrentIndexChanged(int)));

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::~RiuPvtPlotPanel()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::setPlotDefaults(QwtPlot* plot)
{
    RiuSummaryQwtPlot::setCommonPlotBehaviour(plot);

    plot->enableAxis(QwtPlot::xBottom, true);
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->enableAxis(QwtPlot::xTop, false);
    plot->enableAxis(QwtPlot::yRight, false);

    plot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 3);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::setPlotData(const std::vector<RigFlowDiagSolverInterface::PvtCurve>& fvfCurveArr, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& viscosityCurveArr, QString cellReferenceText)
{
    //cvf::Trace::show("RiuPvtPlotPanel::setPlotData()");

    m_allFvfCurvesArr = fvfCurveArr;
    m_allViscosityCurvesArr = viscosityCurveArr;

    m_cellReferenceText = cellReferenceText;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::clearPlot()
{
    //cvf::Trace::show("RiuPvtPlotPanel::clearPlot()");

    if (m_allFvfCurvesArr.empty() && m_allViscosityCurvesArr.empty() && m_cellReferenceText.isEmpty())
    {
        return;
    }

    m_allFvfCurvesArr.clear();
    m_allViscosityCurvesArr.clear();

    m_cellReferenceText.clear();

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::plotUiSelectedCurves()
{
    std::vector<RigFlowDiagSolverInterface::PvtCurve> selectedFvfCurves;
    std::vector<RigFlowDiagSolverInterface::PvtCurve> selectedViscosityCurves;

    // Determine which curves to actually plot based on selection in GUI
    const int currComboIdx = m_phaseComboBox->currentIndex();
    const RigFlowDiagSolverInterface::PvtCurve::Phase phaseToPlot = static_cast<const RigFlowDiagSolverInterface::PvtCurve::Phase>(m_phaseComboBox->itemData(currComboIdx).toInt());

    for (RigFlowDiagSolverInterface::PvtCurve curve : m_allFvfCurvesArr)
    {
        if (curve.phase == phaseToPlot) 
        {
            selectedFvfCurves.push_back(curve);
        }
    }

    for (RigFlowDiagSolverInterface::PvtCurve curve : m_allViscosityCurvesArr)
    {
        if (curve.phase == phaseToPlot)
        {
            selectedViscosityCurves.push_back(curve);
        }
    }

    QString phaseString = "";
    if      (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::GAS)   phaseString = "Gas ";
    else if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::OIL)   phaseString = "Oil ";
    else if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::WATER) phaseString = "Water ";

    {
        const QString plotTitle = phaseString + "Formation Volume Factor";
        const QString yAxisTitle = "";
        plotCurvesInQwt(selectedFvfCurves, plotTitle, yAxisTitle, m_fvfPlot);
    }
    
    {
        const QString plotTitle = phaseString + "Viscosity";
        const QString yAxisTitle = phaseString + "Viscosity";
        plotCurvesInQwt(selectedViscosityCurves, plotTitle, yAxisTitle, m_viscosityPlot);
    }

    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::PvtCurve>& curveArr, QString plotTitle, QString yAxisTitle, QwtPlot* plot)
{
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);

    for (size_t i = 0; i < curveArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::PvtCurve& curve = curveArr[i];
        QwtPlotCurve* qwtCurve = new QwtPlotCurve();

        CVF_ASSERT(curve.xVals.size() == curve.yVals.size());
        qwtCurve->setSamples(curve.xVals.data(), curve.yVals.data(), static_cast<int>(curve.xVals.size()));

        qwtCurve->setStyle(QwtPlotCurve::Lines);

        QColor curveClr = Qt::magenta;
        if      (curve.phase == RigFlowDiagSolverInterface::PvtCurve::GAS)   curveClr = QColor(Qt::red);
        else if (curve.phase == RigFlowDiagSolverInterface::PvtCurve::OIL)   curveClr = QColor(Qt::green);
        else if (curve.phase == RigFlowDiagSolverInterface::PvtCurve::WATER) curveClr = QColor(Qt::blue);
        const QPen curvePen(curveClr);
        qwtCurve->setPen(curvePen);

        qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        qwtCurve->attach(plot);
    }


    plot->setTitle(plotTitle);

    plot->setAxisTitle(QwtPlot::xBottom, "Pressure");
    plot->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::slotPhaseComboCurrentIndexChanged(int)
{
    plotUiSelectedCurves();
}

