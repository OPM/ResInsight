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

#include <QWidget>

#include <cmath>
#include <memory>

class RiuPvtPlotUpdater;
class QDockWidget;
class QwtPlot;
class QComboBox;
class QwtPlotMarker;


//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtPlotPanel : public QWidget
{
    Q_OBJECT

public:
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

    void                setPlotData(const std::vector<RigFlowDiagSolverInterface::PvtCurve>& fvfCurveArr, const std::vector<RigFlowDiagSolverInterface::PvtCurve>& viscosityCurveArr, FvfDynProps fvfDynProps, ViscosityDynProps viscosityDynProps, double pressure);
    void                clearPlot();
    RiuPvtPlotUpdater*  plotUpdater();

private:
    void            plotUiSelectedCurves();
    static void     setPlotDefaults(QwtPlot* plot);
    static void     plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::PvtCurve>& curveArr, double pressure, double pointMarkerYValue, QString plotTitle, QString yAxisTitle, QwtPlot* plot, std::vector<QwtPlotMarker*>* myPlotMarkers);

private slots:
    void            slotPhaseComboCurrentIndexChanged(int);
    
private:
    std::vector<RigFlowDiagSolverInterface::PvtCurve>   m_allFvfCurvesArr;
    std::vector<RigFlowDiagSolverInterface::PvtCurve>   m_allViscosityCurvesArr;
    FvfDynProps                                         m_fvfDynProps;
    ViscosityDynProps                                   m_viscosityDynProps;
    double                                              m_pressure;

    QComboBox*                                          m_phaseComboBox;

    QwtPlot*                                            m_fvfPlot;
    QwtPlot*                                            m_viscosityPlot;
    std::vector<QwtPlotMarker*>                         m_fvfPlotMarkers;
    std::vector<QwtPlotMarker*>                         m_viscosityPlotMarkers;

    std::unique_ptr<RiuPvtPlotUpdater>                  m_plotUpdater;
};

