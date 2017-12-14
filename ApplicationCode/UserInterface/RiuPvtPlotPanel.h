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

#pragma once

#include "RigFlowDiagSolverInterface.h"
#include "RiaEclipseUnitTools.h"

#include <QWidget>
#include <QPointer>

#include <cmath>
#include <memory>

class RiuPvtPlotUpdater;
class QDockWidget;
class QwtPlot;
class QComboBox;
class QwtPlotMarker;
class QwtPlotCurve;

class RiuPvtQwtPicker;
class RiuPvtPlotPanel;



// Interface for providing our custom picker with a tracker text
class RiuPvtTrackerTextProvider
{
public:
    virtual QString trackerText() const = 0;
};


//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtPlotWidget : public QWidget, public RiuPvtTrackerTextProvider
{
    Q_OBJECT

public:
    RiuPvtPlotWidget(RiuPvtPlotPanel* parent);

    void    plotCurves(RiaEclipseUnitTools::UnitSystem unitSystem, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& curveArr, double pressure, double pointMarkerYValue, QString pointMarkerLabel, QString plotTitle, QString yAxisTitle);

private:
    static void                 setPlotDefaults(QwtPlot* plot);
    const QwtPlotCurve*         closestCurveSample(const QPoint& cursorPosition, int* closestSampleIndex) const;
    size_t                      indexOfQwtCurve(const QwtPlotCurve* qwtCurve) const;
    void                        updateTrackerPlotMarkerAndLabelFromPicker();
    virtual QString             trackerText() const override;

    private slots:
    void            slotPickerActivated(bool);
    void            slotPickerPointChanged(const QPoint& pt);

private:
    QPointer<QwtPlot>                                   m_qwtPlot;

    std::vector<RigFlowDiagSolverInterface::PvtCurve>   m_pvtCurveArr;  // Array of source Pvt curves currently being plotted
    std::vector<const QwtPlotCurve*>                    m_qwtCurveArr;  // Array of corresponding qwt curves used for mapping to Pvt curve when doing tracking

    QPointer<RiuPvtQwtPicker>                           m_qwtPicker;
    QString                                             m_trackerLabel;
    QwtPlotMarker*                                      m_trackerPlotMarker;
};



//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtPlotPanel : public QWidget
{
    Q_OBJECT

public:
    struct CellValues
    {
        double pressure = HUGE_VAL;
        double rs = HUGE_VAL;
        double rv = HUGE_VAL;
    };

    struct FvfDynProps
    {
        double bo = HUGE_VAL;
        double bg = HUGE_VAL;
    };

    struct ViscosityDynProps
    {
        double mu_o = HUGE_VAL;
        double mu_g = HUGE_VAL;
    };

public:
    RiuPvtPlotPanel(QDockWidget* parent);
    virtual ~RiuPvtPlotPanel();

    void                setPlotData(RiaEclipseUnitTools::UnitSystem unitSystem, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& fvfCurveArr, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& viscosityCurveArr, FvfDynProps fvfDynProps, ViscosityDynProps viscosityDynProps, CellValues cellValues);
    void                clearPlot();
    RiuPvtPlotUpdater*  plotUpdater();

private:
    void                plotUiSelectedCurves();
    static QString      unitLabelFromCurveIdent(RiaEclipseUnitTools::UnitSystem unitSystem, RigFlowDiagSolverInterface::PvtCurve::Ident curveIdent);

private slots:
    void                slotPhaseComboCurrentIndexChanged(int);

private:
    RiaEclipseUnitTools::UnitSystem                     m_unitSystem;
    std::vector<RigFlowDiagSolverInterface::PvtCurve>   m_allFvfCurvesArr;
    std::vector<RigFlowDiagSolverInterface::PvtCurve>   m_allViscosityCurvesArr;
    FvfDynProps                                         m_fvfDynProps;
    ViscosityDynProps                                   m_viscosityDynProps;
    CellValues                                          m_cellValues;

    QComboBox*                                          m_phaseComboBox;

    RiuPvtPlotWidget*                                   m_fvfPlot;
    RiuPvtPlotWidget*                                   m_viscosityPlot;

    std::unique_ptr<RiuPvtPlotUpdater>                  m_plotUpdater;
};


