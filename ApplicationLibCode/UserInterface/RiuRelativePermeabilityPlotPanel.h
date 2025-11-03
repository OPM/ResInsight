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

#include "RigFlowDiagDefines.h"

#include <QPointer>
#include <QWidget>

#include <memory>

class RiuDockedQwtPlot;
class RiuRelativePermeabilityPlotUpdater;
class RiuRelPermQwtPicker;
class QButtonGroup;
class QCheckBox;
class QwtPlot;
class QwtPlotMarker;
class QwtPlotCurve;
class QPointF;
class QGroupBox;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuRelativePermeabilityPlotPanel : public QWidget
{
    Q_OBJECT

public:
    RiuRelativePermeabilityPlotPanel( QWidget* parent );
    ~RiuRelativePermeabilityPlotPanel() override;

    void setPlotData( RiaDefines::EclipseUnitSystem                        unitSystem,
                      const std::vector<RigFlowDiagDefines::RelPermCurve>& relPermCurves,
                      double                                               swat,
                      double                                               sgas,
                      const QString&                                       caseName,
                      const QString&                                       cellReferenceText );

    void enableImbibitionCurveSelection( bool enable );

    void                                clearPlot();
    RiuRelativePermeabilityPlotUpdater* plotUpdater();
    void                                applyFontSizes( bool replot );

private:
    enum WhichYAxis
    {
        LEFT_YAXIS,
        RIGHT_YAXIS
    };

    void        plotUiSelectedCurves();
    static void setPlotDefaults( QwtPlot* plot );
    static void plotCurvesInQwt( RiaDefines::EclipseUnitSystem                        unitSystem,
                                 const std::vector<RigFlowDiagDefines::RelPermCurve>& curveArr,
                                 double                                               swat,
                                 double                                               sgas,
                                 QString                                              cellReferenceText,
                                 bool                                                 logScaleLeftAxis,
                                 bool                                                 fixedXAxis,
                                 bool                                                 fixedLeftYAxis,
                                 QwtPlot*                                             plot,
                                 std::vector<QwtPlotMarker*>*                         myPlotMarkers,
                                 bool                                                 showScaled,
                                 bool                                                 showUnscaled );

    static QwtPlotCurve* getLegendCurve( QString title, bool scaled );

    static QString determineXAxisTitleFromCurveCollection( const std::vector<RigFlowDiagDefines::RelPermCurve>& curveArr );

    static void addVerticalSaturationMarkerLine( double                       saturationValue,
                                                 QString                      label,
                                                 QColor                       color,
                                                 QwtPlot*                     plot,
                                                 std::vector<QwtPlotMarker*>* myPlotMarkers );

    static void addCurveConstSaturationIntersectionMarker( const RigFlowDiagDefines::RelPermCurve& curve,
                                                           double                                  saturationValue,
                                                           QColor                                  markerColor,
                                                           WhichYAxis                              whichYAxis,
                                                           QwtPlot*                                plot,
                                                           std::vector<QwtPlotMarker*>*            myPlotMarkers,
                                                           std::vector<QPointF>*                   points,
                                                           std::vector<WhichYAxis>*                axes );

    static void
        addTransparentCurve( QwtPlot* plot, const std::vector<QPointF>& points, const std::vector<WhichYAxis>& axes, bool logScaleLeftAxis );

    std::vector<RigFlowDiagDefines::RelPermCurve> gatherUiSelectedCurves() const;
    QString                                       asciiDataForUiSelectedCurves() const;

    const QwtPlotCurve* closestCurveSample( const QPoint& cursorPosition, int* closestSampleIndex ) const;
    void                updateTrackerPlotMarkerAndLabelFromPicker();

    void contextMenuEvent( QContextMenuEvent* event ) override;

private slots:
    void slotButtonInButtonGroupClicked( int );
    void slotSomeCheckBoxStateChanged( int );
    void slotCurrentPlotDataInTextDialog();
    void slotShowCurveSelectionWidgets( int state );
    void slotPickerActivated( bool );
    void slotPickerPointChanged( const QPoint& pt );
    void showEvent( QShowEvent* event ) override;

private:
    RiaDefines::EclipseUnitSystem                 m_unitSystem;
    std::vector<RigFlowDiagDefines::RelPermCurve> m_allCurvesArr;
    double                                        m_swat;
    double                                        m_sgas;
    QString                                       m_caseName;
    QString                                       m_cellReferenceText;
    QPointer<RiuDockedQwtPlot>                    m_qwtPlot;
    std::vector<QwtPlotMarker*>                   m_myPlotMarkers;

    QPointer<RiuRelPermQwtPicker> m_qwtPicker;
    QString                       m_trackerLabel;
    QwtPlotMarker*                m_trackerPlotMarker;

    QGroupBox* m_curveSetGroupBox;
    QCheckBox* m_showDrainageCheckBox;
    QCheckBox* m_showImbibitionCheckBox;

    QGroupBox*    m_groupBox;
    QButtonGroup* m_selectedCurvesButtonGroup;
    QCheckBox*    m_showUnscaledCheckBox;
    QCheckBox*    m_showScaledCheckBox;
    QCheckBox*    m_logarithmicScaleKrAxisCheckBox;
    QCheckBox*    m_fixedXAxisCheckBox;
    QCheckBox*    m_fixedLeftYAxisCheckBox;

    std::unique_ptr<RiuRelativePermeabilityPlotUpdater> m_plotUpdater;
};
