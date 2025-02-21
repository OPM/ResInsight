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

#include "RiaDefines.h"
#include "RigFlowDiagSolverInterface.h"

#include <QWidget>

#include <cmath>
#include <memory>

class RiuPvtPlotUpdater;
class RiuPvtPlotWidget;

class QComboBox;
class QLabel;

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
        double rs       = HUGE_VAL;
        double rv       = HUGE_VAL;
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
    RiuPvtPlotPanel( QWidget* parent );
    ~RiuPvtPlotPanel() override;

    void               setPlotData( RiaDefines::EclipseUnitSystem                            unitSystem,
                                    const std::vector<RigFlowDiagSolverInterface::PvtCurve>& fvfCurveArr,
                                    const std::vector<RigFlowDiagSolverInterface::PvtCurve>& viscosityCurveArr,
                                    const FvfDynProps&                                       fvfDynProps,
                                    const ViscosityDynProps&                                 viscosityDynProps,
                                    const CellValues&                                        cellValues,
                                    const QString&                                           cellReferenceText );
    void               clearPlot();
    RiuPvtPlotUpdater* plotUpdater();
    void               applyFontSizes( bool replot );

private:
    void plotUiSelectedCurves();
    static QString unitLabelFromCurveIdent( RiaDefines::EclipseUnitSystem unitSystem, RigFlowDiagSolverInterface::PvtCurve::Ident curveIdent );

private slots:
    void slotPhaseComboCurrentIndexChanged( int );
    void showEvent( QShowEvent* event ) override;

private:
    RiaDefines::EclipseUnitSystem                     m_unitSystem;
    std::vector<RigFlowDiagSolverInterface::PvtCurve> m_allFvfCurvesArr;
    std::vector<RigFlowDiagSolverInterface::PvtCurve> m_allViscosityCurvesArr;
    FvfDynProps                                       m_fvfDynProps;
    ViscosityDynProps                                 m_viscosityDynProps;
    CellValues                                        m_cellValues;
    QString                                           m_cellReferenceText;

    QComboBox* m_phaseComboBox;
    QLabel*    m_titleLabel;

    RiuPvtPlotWidget* m_fvfPlot;
    RiuPvtPlotWidget* m_viscosityPlot;

    std::unique_ptr<RiuPvtPlotUpdater> m_plotUpdater;
};
