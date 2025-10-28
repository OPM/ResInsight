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

#include <cmath>
#include <memory>

class RiuDockedQwtPlot;
class RiuPvtPlotUpdater;
class RiuPvtQwtPicker;
class RiuPvtPlotPanel;

class QComboBox;

class QwtPlot;
class QwtPlotMarker;
class QwtPlotCurve;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtPlotWidget : public QWidget
{
    Q_OBJECT

public:
    RiuPvtPlotWidget( RiuPvtPlotPanel* parent );

    void plotCurves( RiaDefines::EclipseUnitSystem                    unitSystem,
                     const std::vector<RigFlowDiagDefines::PvtCurve>& curveArr,
                     double                                           pressure,
                     double                                           pointMarkerYValue,
                     const QString&                                   pointMarkerLabel,
                     const QString&                                   plotTitle,
                     const QString&                                   yAxisTitle );
    void applyFontSizes( bool replot );

private:
    static void         setPlotDefaults( QwtPlot* plot );
    const QwtPlotCurve* closestCurveSample( const QPoint& cursorPosition, int* closestSampleIndex ) const;
    size_t              indexOfQwtCurve( const QwtPlotCurve* qwtCurve ) const;
    void                updateTrackerPlotMarkerAndLabelFromPicker();

private slots:
    void slotPickerActivated( bool );
    void slotPickerPointChanged( const QPoint& pt );

private:
    QPointer<RiuDockedQwtPlot> m_qwtPlot;

    std::vector<RigFlowDiagDefines::PvtCurve> m_pvtCurveArr; // Array of source Pvt curves currently being plotted
    std::vector<const QwtPlotCurve*>          m_qwtCurveArr; // Array of corresponding qwt curves used for mapping to Pvt curve
                                                    // when doing tracking

    QPointer<RiuPvtQwtPicker> m_qwtPicker;
    QString                   m_trackerLabel;
    QwtPlotMarker*            m_trackerPlotMarker;
};
