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
#include "cvfMath.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_legend.h"
#include "qwt_symbol.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"

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
//
//
//
//==================================================================================================
class PvtQwtPicker : public QwtPicker
{
public:
    PvtQwtPicker(QwtPlot* plot, RiuPvtTrackerTextProvider* trackerTextProvider)
     :  QwtPicker(QwtPicker::NoRubberBand, QwtPicker::AlwaysOn, plot->canvas()),
        m_trackerTextProvider(trackerTextProvider)
    {
        setStateMachine(new QwtPickerTrackerMachine);
    }

    virtual QwtText trackerText(const QPoint&) const
    {
        QwtText text(m_trackerTextProvider->trackerText());
        text.setRenderFlags(Qt::AlignLeft);
        return text;
    }

private:
    const RiuPvtTrackerTextProvider* m_trackerTextProvider;
};



//==================================================================================================
///
/// \class RiuPvtPlotWidget
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotWidget::RiuPvtPlotWidget(RiuPvtPlotPanel* parent)
:   QWidget(parent),
    m_trackerPlotMarker(NULL)
{
    m_qwtPlot = new PvtQwtPlot(this);
    setPlotDefaults(m_qwtPlot);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(m_qwtPlot);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    m_qwtPicker = new PvtQwtPicker(m_qwtPlot, this);
    connect(m_qwtPicker, SIGNAL(activated(bool)), this, SLOT(slotPickerActivated(bool)));
    connect(m_qwtPicker, SIGNAL(moved(const QPoint&)), this, SLOT(slotPickerPointChanged(const QPoint&)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::setPlotDefaults(QwtPlot* plot)
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
    {
        QwtPlotGrid* grid = new QwtPlotGrid;
        grid->attach(plot);
        QPen gridPen(Qt::SolidLine);
        gridPen.setColor(Qt::lightGray);
        grid->setPen(gridPen);
    }

    // Axis number font
    {
        QFont axisFont = plot->axisFont(QwtPlot::xBottom);
        axisFont.setPixelSize(11);
        plot->setAxisFont(QwtPlot::xBottom, axisFont);
        plot->setAxisFont(QwtPlot::yLeft, axisFont);
    }

    // Axis title font
    {
        QwtText axisTitle = plot->axisTitle(QwtPlot::xBottom);
        QFont axisTitleFont = axisTitle.font();
        axisTitleFont.setPixelSize(11);
        axisTitleFont.setBold(false);
        axisTitle.setFont(axisTitleFont);
        axisTitle.setRenderFlags(Qt::AlignRight);
        plot->setAxisTitle(QwtPlot::xBottom, axisTitle);
        plot->setAxisTitle(QwtPlot::yLeft, axisTitle);
    }

    // Title font
    {
        QwtText plotTitle = plot->title();
        QFont titleFont = plotTitle.font();
        titleFont.setPixelSize(14);
        plotTitle.setFont(titleFont);
        plot->setTitle(plotTitle);
    }


    plot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 3);

    plot->plotLayout()->setAlignCanvasToScales(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::plotCurves(RiaEclipseUnitTools::UnitSystem unitSystem, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& curveArr, double pressure, double pointMarkerYValue, QString plotTitle, QString yAxisTitle)
{
    m_qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    m_qwtPlot->detachItems(QwtPlotItem::Rtti_PlotMarker);
    m_qwtCurveArr.clear();
    m_pvtCurveArr.clear();
    m_trackerPlotMarker = NULL;


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

        qwtCurve->attach(m_qwtPlot);

        m_qwtCurveArr.push_back(qwtCurve);
    }

    m_pvtCurveArr = curveArr;
    CVF_ASSERT(m_pvtCurveArr.size() == m_qwtCurveArr.size());


    // Add vertical marker line to indicate cell pressure
    if (pressure != HUGE_VAL)
    {
        QwtPlotMarker* lineMarker = new QwtPlotMarker;
        lineMarker->setXValue(pressure);
        lineMarker->setLineStyle(QwtPlotMarker::VLine);
        lineMarker->setLinePen(QPen(QColor(128, 0, 255), 1, Qt::DashLine));
        lineMarker->setLabel(QString("PRESSURE"));
        lineMarker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
        lineMarker->setLabelOrientation(Qt::Vertical);
        lineMarker->attach(m_qwtPlot);
    }

    // Then point marker
    if (pressure != HUGE_VAL && pointMarkerYValue != HUGE_VAL)
    {
        QwtPlotMarker* pointMarker = new QwtPlotMarker;
        pointMarker->setValue(pressure, pointMarkerYValue);

        QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse);
        symbol->setSize(13, 13);
        symbol->setPen(QPen(QColor(128, 128, 255), 2));
        symbol->setBrush(Qt::NoBrush);
        pointMarker->setSymbol(symbol);
        pointMarker->attach(m_qwtPlot);
    }

    m_qwtPlot->setTitle(plotTitle);

    m_qwtPlot->setAxisTitle(QwtPlot::xBottom, QString("Pressure [%1]").arg(RiaEclipseUnitTools::unitStringPressure(unitSystem)));
    m_qwtPlot->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    updateTrackerPlotMarkerAndLabelFromPicker();

    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::updateTrackerPlotMarkerAndLabelFromPicker()
{
    bool hasValidSamplePoint = false;
    QPointF samplePoint;
    QString mixRatioText = "";
    double mixRat = HUGE_VAL;

    if (m_qwtPicker && m_qwtPicker->isActive())
    {
        const QPoint trackerPos = m_qwtPicker->trackerPosition();

        int pointSampleIdx = -1;
        const QwtPlotCurve* closestQwtCurve = closestCurveSample(trackerPos, *m_qwtPlot, &pointSampleIdx);
        if (closestQwtCurve && pointSampleIdx >= 0)
        {
            samplePoint = closestQwtCurve->sample(pointSampleIdx);
            hasValidSamplePoint = true;

            size_t curveIdx = indexOfQwtCurve(closestQwtCurve);
            if (curveIdx < m_pvtCurveArr.size())
            {
                const RigFlowDiagSolverInterface::PvtCurve& pvtCurve = m_pvtCurveArr[curveIdx];
                if (static_cast<size_t>(pointSampleIdx) < pvtCurve.mixRatVals.size())
                {
                    mixRat = pvtCurve.mixRatVals[pointSampleIdx];

                    // The text is Rs or Rv depending on phase
                    mixRatioText = (pvtCurve.phase == RigFlowDiagSolverInterface::PvtCurve::GAS) ? "Rv" : "Rs";
                }
            }
        }
    }


    m_trackerLabel = "";

    bool needsReplot = false;

    if (hasValidSamplePoint)
    {
        if (!m_trackerPlotMarker)
        {
            m_trackerPlotMarker = new QwtPlotMarker;

            QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse);
            symbol->setSize(13, 13);
            symbol->setPen(QPen(QColor(0, 0, 0), 2));
            symbol->setBrush(Qt::NoBrush);
            m_trackerPlotMarker->setSymbol(symbol);
            m_trackerPlotMarker->attach(m_qwtPlot);

            needsReplot = true;
        }

        if (m_trackerPlotMarker->value() != samplePoint)
        {
            m_trackerPlotMarker->setValue(samplePoint);
            needsReplot = true;
        }

        m_trackerLabel = QString("%1 (%2)").arg(samplePoint.y()).arg(samplePoint.x());
        if (mixRat != HUGE_VAL)
        {
            m_trackerLabel += QString("\n%1 = %2").arg(mixRatioText).arg(mixRat);
        }
    }
    else
    {
        if (m_trackerPlotMarker)
        {
            m_trackerPlotMarker->detach();
            delete m_trackerPlotMarker;
            m_trackerPlotMarker = NULL;

            needsReplot = true;
        }
    }

    if (needsReplot)
    {
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QwtPlotCurve* RiuPvtPlotWidget::closestCurveSample(const QPoint& cursorPosition, const QwtPlot& plot, int* closestSampleIndex)
{
    if (closestSampleIndex) *closestSampleIndex = -1;

    const QwtPlotCurve* closestCurve = NULL;
    double distMin = HUGE_VAL;
    int closestPointSampleIndex = -1;

    const QwtPlotItemList& itemList = plot.itemList();
    for (QwtPlotItemIterator it = itemList.begin(); it != itemList.end(); it++)
    {
        if ((*it)->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            const QwtPlotCurve* candidateCurve = static_cast<const QwtPlotCurve*>(*it);
            double dist = HUGE_VAL;
            int candidateSampleIndex = candidateCurve->closestPoint(cursorPosition, &dist);
            if (dist < distMin)
            {
                closestCurve = candidateCurve;
                closestPointSampleIndex = candidateSampleIndex;
                distMin = dist;
            }
        }
    }

    if (closestCurve && closestPointSampleIndex >= 0 && distMin < 50)
    {
        if (closestSampleIndex) *closestSampleIndex = closestPointSampleIndex;
        return closestCurve;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RiuPvtPlotWidget::indexOfQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for (size_t i = 0; i < m_qwtCurveArr.size();  i++)
    {
        if (m_qwtCurveArr[i] == qwtCurve)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// Implements the RiuPvtTrackerTextProvider interface
//--------------------------------------------------------------------------------------------------
QString RiuPvtPlotWidget::trackerText() const
{
    return m_trackerLabel;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::slotPickerPointChanged(const QPoint& pt)
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::slotPickerActivated(bool on)
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}



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

    m_fvfPlot = new RiuPvtPlotWidget(this);
    m_viscosityPlot = new RiuPvtPlotWidget(this);

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
        const QString yAxisTitle = QString("%1 Formation Volume Factor [%2]").arg(phaseString).arg(unitLabelFromCurveIdent(m_unitSystem, curveIdentToPlot));
        m_fvfPlot->plotCurves(m_unitSystem, selectedFvfCurves, m_pressure, fvfPointMarkerYValue, plotTitle, yAxisTitle);
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
        const QString yAxisTitle = QString("%1 Viscosity [%2]").arg(phaseString).arg(unitLabelFromCurveIdent(m_unitSystem, curveIdentToPlot));
        m_viscosityPlot->plotCurves(m_unitSystem, selectedViscosityCurves, m_pressure, viscosityPointMarkerYValue, plotTitle, yAxisTitle);
    }
}

//--------------------------------------------------------------------------------------------------
/// Static helper to get unit labels
//--------------------------------------------------------------------------------------------------
QString RiuPvtPlotPanel::unitLabelFromCurveIdent(RiaEclipseUnitTools::UnitSystem unitSystem, RigFlowDiagSolverInterface::PvtCurve::Ident curveIdent)
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

