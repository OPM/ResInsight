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
#include "RiuPvtPlotUpdater.h"
#include "RiuSummaryQwtPlot.h"

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
:   QWidget(parent),
    m_unitSystem(RiaEclipseUnitTools::UNITS_UNKNOWN),
    m_pressure(HUGE_VAL),
    m_plotUpdater(new RiuPvtPlotUpdater(this))
{
    m_phaseComboBox = new QComboBox(this);
    m_phaseComboBox->setEditable(false);
    m_phaseComboBox->addItem("Oil", QVariant(RigFlowDiagSolverInterface::PvtCurve::OIL));
    m_phaseComboBox->addItem("Gas", QVariant(RigFlowDiagSolverInterface::PvtCurve::GAS));

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
void RiuPvtPlotPanel::setPlotData(RiaEclipseUnitTools::UnitSystem unitSystem, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& fvfCurveArr, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& viscosityCurveArr, FvfDynProps fvfDynProps, ViscosityDynProps viscosityDynProps, double pressure)
{
    //cvf::Trace::show("RiuPvtPlotPanel::setPlotData()");

    m_unitSystem = unitSystem;
    m_allFvfCurvesArr = fvfCurveArr;
    m_allViscosityCurvesArr = viscosityCurveArr;
    m_fvfDynProps = fvfDynProps;
    m_viscosityDynProps = viscosityDynProps;
    m_pressure = pressure;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::clearPlot()
{
    //cvf::Trace::show("RiuPvtPlotPanel::clearPlot()");

    if (m_allFvfCurvesArr.empty() && m_allViscosityCurvesArr.empty())
    {
        return;
    }

    m_unitSystem = RiaEclipseUnitTools::UNITS_UNKNOWN;
    m_allFvfCurvesArr.clear();
    m_allViscosityCurvesArr.clear();
    m_fvfDynProps = FvfDynProps();
    m_viscosityDynProps = ViscosityDynProps();
    m_pressure = HUGE_VAL;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotUpdater* RiuPvtPlotPanel::plotUpdater()
{
    return m_plotUpdater.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::plotUiSelectedCurves()
{

    // Determine which curves (phase) to actually plot based on selection in GUI
    const int currComboIdx = m_phaseComboBox->currentIndex();
    const RigFlowDiagSolverInterface::PvtCurve::Phase phaseToPlot = static_cast<const RigFlowDiagSolverInterface::PvtCurve::Phase>(m_phaseComboBox->itemData(currComboIdx).toInt());

    QString phaseString = "";
    if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::GAS)
    {
        phaseString = "Gas ";
    }
    else if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::OIL)
    {
        phaseString = "Oil ";
    }

    // FVF plot
    {
        RigFlowDiagSolverInterface::PvtCurve::Ident curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Unknown;
        double fvfPointMarkerYValue = HUGE_VAL;

        if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::GAS)
        {
            curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Bg;
            fvfPointMarkerYValue = m_fvfDynProps.bg;
        }
        else if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::OIL)
        {
            curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Bo;
            fvfPointMarkerYValue = m_fvfDynProps.bo;
        }

        std::vector<RigFlowDiagSolverInterface::PvtCurve> selectedFvfCurves;
        for (RigFlowDiagSolverInterface::PvtCurve curve : m_allFvfCurvesArr)
        {
            if (curve.ident == curveIdentToPlot)
            {
                selectedFvfCurves.push_back(curve);
            }
        }

        const QString plotTitle = QString("%1 Formation Volume Factor").arg(phaseString);
        const QString yAxisTitle = QString("%1 Formation Volume Factor [%2]").arg(phaseString).arg(unitStringFromCurveIdent(m_unitSystem, curveIdentToPlot));
        plotCurvesInQwt(m_unitSystem, selectedFvfCurves, m_pressure, fvfPointMarkerYValue, plotTitle, yAxisTitle, m_fvfPlot, &m_fvfPlotMarkers);
    }
    
    // Viscosity plot
    {
        RigFlowDiagSolverInterface::PvtCurve::Ident curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Unknown;
        double viscosityPointMarkerYValue = HUGE_VAL;

        if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::GAS)
        {
            curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Visc_g;
            viscosityPointMarkerYValue = m_viscosityDynProps.mu_g;
        }
        else if (phaseToPlot == RigFlowDiagSolverInterface::PvtCurve::OIL)
        {
            curveIdentToPlot = RigFlowDiagSolverInterface::PvtCurve::Visc_o;
            viscosityPointMarkerYValue = m_viscosityDynProps.mu_o;
        }

        std::vector<RigFlowDiagSolverInterface::PvtCurve> selectedViscosityCurves;
        for (RigFlowDiagSolverInterface::PvtCurve curve : m_allViscosityCurvesArr)
        {
            if (curve.ident == curveIdentToPlot)
            {
                selectedViscosityCurves.push_back(curve);
            }
        }

        const QString plotTitle = QString("%1 Viscosity").arg(phaseString);
        const QString yAxisTitle = QString("%1 Viscosity [%2]").arg(phaseString).arg(unitStringFromCurveIdent(m_unitSystem, curveIdentToPlot));
        plotCurvesInQwt(m_unitSystem, selectedViscosityCurves, m_pressure, viscosityPointMarkerYValue, plotTitle, yAxisTitle, m_viscosityPlot, &m_viscosityPlotMarkers);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::plotCurvesInQwt(RiaEclipseUnitTools::UnitSystem unitSystem, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& curveArr, double pressure, double pointMarkerYValue, QString plotTitle, QString yAxisTitle, QwtPlot* plot, std::vector<QwtPlotMarker*>* myPlotMarkers)
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


    for (size_t i = 0; i < curveArr.size(); i++)
    {
        const RigFlowDiagSolverInterface::PvtCurve& curve = curveArr[i];
        QwtPlotCurve* qwtCurve = new QwtPlotCurve();

        CVF_ASSERT(curve.pressureVals.size() == curve.yVals.size());
        qwtCurve->setSamples(curve.pressureVals.data(), curve.yVals.data(), static_cast<int>(curve.pressureVals.size()));

        qwtCurve->setStyle(QwtPlotCurve::Lines);

        QColor curveClr = Qt::magenta;
        if      (curve.phase == RigFlowDiagSolverInterface::PvtCurve::GAS)   curveClr = QColor(Qt::red);
        else if (curve.phase == RigFlowDiagSolverInterface::PvtCurve::OIL)   curveClr = QColor(Qt::green);
        const QPen curvePen(curveClr);
        qwtCurve->setPen(curvePen);

        qwtCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        QwtSymbol* curveSymbol = new QwtSymbol(QwtSymbol::Ellipse);
        curveSymbol->setSize(6, 6);
        curveSymbol->setPen(curvePen);
        curveSymbol->setBrush(Qt::NoBrush);
        qwtCurve->setSymbol(curveSymbol);

        qwtCurve->attach(plot);
    }

    // Add vertical marker lines to indicate cell pressure
    if (pressure != HUGE_VAL)
    {
        QwtPlotMarker* lineMarker = new QwtPlotMarker;
        lineMarker->setXValue(pressure);
        lineMarker->setLineStyle(QwtPlotMarker::VLine);
        lineMarker->setLinePen(QPen(QColor(128,0,255), 1, Qt::DashLine));
        lineMarker->setLabel(QString("PRESSURE"));
        lineMarker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
        lineMarker->setLabelOrientation(Qt::Vertical);
        lineMarker->attach(plot);
        myPlotMarkers->push_back(lineMarker);
    }

    if (pressure != HUGE_VAL && pointMarkerYValue != HUGE_VAL)
    {
        QwtPlotMarker* pointMarker = new QwtPlotMarker;
        pointMarker->setValue(pressure, pointMarkerYValue);

        QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse);
        symbol->setSize(13, 13);
        symbol->setPen(QPen(QColor(128, 128, 255), 2));
        symbol->setBrush(Qt::NoBrush);
        pointMarker->setSymbol(symbol);
        pointMarker->attach(plot);
        myPlotMarkers->push_back(pointMarker);
    }

    plot->setTitle(plotTitle);

    plot->setAxisTitle(QwtPlot::xBottom, QString("Pressure [%1]").arg(RiaEclipseUnitTools::unitStringPressure(unitSystem)));
    plot->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuPvtPlotPanel::unitStringFromCurveIdent(RiaEclipseUnitTools::UnitSystem unitSystem, RigFlowDiagSolverInterface::PvtCurve::Ident curveIdent)
{
    if (curveIdent == RigFlowDiagSolverInterface::PvtCurve::Bo)
    {
        switch (unitSystem)
        {
            case RiaEclipseUnitTools::UNITS_METRIC:     return "rm3/sm3";
            case RiaEclipseUnitTools::UNITS_FIELD:      return "rb/stb";
            case RiaEclipseUnitTools::UNITS_LAB:        return "rcc/scc";
            default:                                    return "";
        }
    }
    else if (curveIdent == RigFlowDiagSolverInterface::PvtCurve::Bg)
    {
        switch (unitSystem)
        {
            case RiaEclipseUnitTools::UNITS_METRIC:     return "rm3/sm3";
            case RiaEclipseUnitTools::UNITS_FIELD:      return "rb/Mscf";
            case RiaEclipseUnitTools::UNITS_LAB:        return "rcc/scc";
            default:                                    return "";
        }
    }
    else if (curveIdent == RigFlowDiagSolverInterface::PvtCurve::Visc_o ||
             curveIdent == RigFlowDiagSolverInterface::PvtCurve::Visc_g)
    {
        switch (unitSystem)
        {
            case RiaEclipseUnitTools::UNITS_METRIC:     return "cP";
            case RiaEclipseUnitTools::UNITS_FIELD:      return "cP";
            case RiaEclipseUnitTools::UNITS_LAB:        return "cP";
            default:                                    return "";
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::slotPhaseComboCurrentIndexChanged(int)
{
    plotUiSelectedCurves();
}

