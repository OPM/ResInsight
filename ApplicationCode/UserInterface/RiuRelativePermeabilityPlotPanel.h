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

#include "RiaEclipseUnitTools.h"
#include "RigFlowDiagSolverInterface.h"

#include <QWidget>

#include <memory>

class RiuRelativePermeabilityPlotUpdater;
class QDockWidget;
class QButtonGroup;
class QCheckBox;
class QwtPlot;
class QwtPlotMarker;
class QPointF;

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

    void                                setPlotData(RiaEclipseUnitTools::UnitSystem unitSystem,
                                                    const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves, 
                                                    double swat, 
                                                    double sgas,
                                                    QString caseName, 
                                                    QString cellReferenceText);
    void                                clearPlot();
    RiuRelativePermeabilityPlotUpdater* plotUpdater();

private:
    enum WhichYAxis
    {
        LEFT_YAXIS,
        RIGHT_YAXIS
    };

    class ValueRange
    {
    public:
        ValueRange();
        void add(const ValueRange& range);

    public:
        double min;
        double max;
    };

    void           plotUiSelectedCurves();
    static void    setPlotDefaults(QwtPlot* plot);
    static void    plotCurvesInQwt(RiaEclipseUnitTools::UnitSystem unitSystem,
                                   const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr, 
                                   double swat,
                                   double sgas,
                                   QString cellReferenceText,
                                   bool logScaleLeftAxis,
                                   QwtPlot* plot,
                                   std::vector<QwtPlotMarker*>* myPlotMarkers);

    static QString determineXAxisTitleFromCurveCollection(const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr);

    static void    addVerticalSaturationMarkerLine(double   saturationValue, 
                                                   QString  label, 
                                                   QColor   color, 
                                                   QwtPlot* plot,
                                                   std::vector<QwtPlotMarker*>* myPlotMarkers);

    static void    addCurveConstSaturationIntersectionMarker(const RigFlowDiagSolverInterface::RelPermCurve& curve,
                                                             double saturationValue, 
                                                             QColor markerColor, 
                                                             WhichYAxis whichYAxis,
                                                             QwtPlot* plot, 
                                                             std::vector<QwtPlotMarker*>* myPlotMarkers,
                                                             std::vector<QPointF>* points,
                                                             std::vector<WhichYAxis>* axes);

    static double  interpolatedCurveYValue(const std::vector<double>& xVals, const std::vector<double>& yVals, double x);

    static void    addTransparentCurve(QwtPlot* plot, const std::vector<QPointF>& points, const std::vector<WhichYAxis>& axes);

    std::vector<RigFlowDiagSolverInterface::RelPermCurve> gatherUiSelectedCurves() const;
    QString                                               asciiDataForUiSelectedCurves() const;

    virtual void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void slotButtonInButtonGroupClicked(int);
    void slotSomeCheckBoxStateChanged(int);
    void slotCurrentPlotDataInTextDialog();

private:
    RiaEclipseUnitTools::UnitSystem                       m_unitSystem;
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> m_allCurvesArr;
    double                                                m_swat;
    double                                                m_sgas;
    QString                                               m_caseName;
    QString                                               m_cellReferenceText;
    QwtPlot*                                              m_qwtPlot;
    std::vector<QwtPlotMarker*>                           m_myPlotMarkers;

    QButtonGroup* m_selectedCurvesButtonGroup;
    QCheckBox*    m_showUnscaledCheckBox;
    QCheckBox*    m_logarithmicScaleKrAxisCheckBox;

    std::unique_ptr<RiuRelativePermeabilityPlotUpdater> m_plotUpdater;
};
