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

class QDockWidget;
class QButtonGroup;
class QwtPlot;


//==================================================================================================
//
//
//
//==================================================================================================
class RiuRelativePermeabilityPlotPanel : public QWidget
{
    Q_OBJECT

public:
    RiuRelativePermeabilityPlotPanel(QDockWidget* parent);
    virtual ~RiuRelativePermeabilityPlotPanel();

    void setPlotData(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves, double swat, double sgas, QString cellReferenceText);
    void clearPlot();

private:
    void            plotUiSelectedCurves();
    static void     setPlotDefaults(QwtPlot* plot);
    static void     plotCurvesInQwt(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr, double swat, double sgas, QString cellReferenceText, QwtPlot* plot);

private slots:
    void            slotButtonInButtonGroupClicked(int);

private:
    std::vector<RigFlowDiagSolverInterface::RelPermCurve>   m_allCurvesArr;
    double                                                  m_swat;
    double                                                  m_sgas;
    QString                                                 m_cellReferenceText;
    QwtPlot*                                                m_qwtPlot;
    QButtonGroup*                                           m_selectedCurvesButtonGroup;
};

