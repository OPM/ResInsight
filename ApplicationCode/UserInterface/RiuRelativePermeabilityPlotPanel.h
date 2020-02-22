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

#include <QPointer>
#include <QWidget>

#include <memory>

class RiuDockedQwtPlot;
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
    RiuRelativePermeabilityPlotPanel( QDockWidget* parent );
    ~RiuRelativePermeabilityPlotPanel() override;

    void                                setPlotData( RiaEclipseUnitTools::UnitSystem                              unitSystem,
                                                     const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves,
                                                     double                                                       swat,
                                                     double                                                       sgas,
                                                     const QString&                                               caseName,
                                                     const QString&                                               cellReferenceText );
    void                                clearPlot();
    RiuRelativePermeabilityPlotUpdater* plotUpdater();
    void                                applyFontSizes( bool replot );

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
        void add( const ValueRange& range );

    public:
        double min;
        double max;
    };

    void        plotUiSelectedCurves();
    static void setPlotDefaults( QwtPlot* plot );
    static void plotCurvesInQwt( RiaEclipseUnitTools::UnitSystem                              unitSystem,
                                 const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr,
                                 double                                                       swat,
                                 double                                                       sgas,
                                 QString                                                      cellReferenceText,
                                 bool                                                         logScaleLeftAxis,
                                 bool                                                         fixedXAxis,
                                 bool                                                         fixedLeftYAxis,
                                 QwtPlot*                                                     plot,
                                 std::vector<QwtPlotMarker*>*                                 myPlotMarkers );

    static QString
        determineXAxisTitleFromCurveCollection( const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr );

    static void addVerticalSaturationMarkerLine( double                       saturationValue,
                                                 QString                      label,
                                                 QColor                       color,
                                                 QwtPlot*                     plot,
                                                 std::vector<QwtPlotMarker*>* myPlotMarkers );

    static void addCurveConstSaturationIntersectionMarker( const RigFlowDiagSolverInterface::RelPermCurve& curve,
                                                           double                       saturationValue,
                                                           QColor                       markerColor,
                                                           WhichYAxis                   whichYAxis,
                                                           QwtPlot*                     plot,
                                                           std::vector<QwtPlotMarker*>* myPlotMarkers,
                                                           std::vector<QPointF>*        points,
                                                           std::vector<WhichYAxis>*     axes );

    static double interpolatedCurveYValue( const std::vector<double>& xVals, const std::vector<double>& yVals, double x );

    static void addTransparentCurve( QwtPlot*                       plot,
                                     const std::vector<QPointF>&    points,
                                     const std::vector<WhichYAxis>& axes,
                                     bool                           logScaleLeftAxis );

    std::vector<RigFlowDiagSolverInterface::RelPermCurve> gatherUiSelectedCurves() const;
    QString                                               asciiDataForUiSelectedCurves() const;

    void contextMenuEvent( QContextMenuEvent* event ) override;

private slots:
    void slotButtonInButtonGroupClicked( int );
    void slotSomeCheckBoxStateChanged( int );
    void slotCurrentPlotDataInTextDialog();

private:
    RiaEclipseUnitTools::UnitSystem                       m_unitSystem;
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> m_allCurvesArr;
    double                                                m_swat;
    double                                                m_sgas;
    QString                                               m_caseName;
    QString                                               m_cellReferenceText;
    QPointer<RiuDockedQwtPlot>                            m_qwtPlot;
    std::vector<QwtPlotMarker*>                           m_myPlotMarkers;

    QButtonGroup* m_selectedCurvesButtonGroup;
    QCheckBox*    m_showUnscaledCheckBox;
    QCheckBox*    m_logarithmicScaleKrAxisCheckBox;
    QCheckBox*    m_fixedXAxisCheckBox;
    QCheckBox*    m_fixedLeftYAxisCheckBox;

    std::unique_ptr<RiuRelativePermeabilityPlotUpdater> m_plotUpdater;
};
