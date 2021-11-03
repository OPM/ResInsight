/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RimPlotCurve.h"
#include "RimPlotCurveAppearance.h"

#include "RiaCurveDataTools.h"
#include "RiaPlotDefines.h"

#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtSymbol.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"

#include <QPointer>
#include <Qt>

class QwtPlot;
class QwtPlotCurve;
class QwtPlotIntervalCurve;
class RiuQwtPlotCurve;

//==================================================================================================
///
///
//==================================================================================================
class RimQwtPlotCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimQwtPlotCurve();
    ~RimQwtPlotCurve() override;

    void loadDataAndUpdate( bool updateParentPlot );

    virtual bool xValueRangeInQwt( double* minimumValue, double* maximumValue ) const;
    virtual bool yValueRangeInQwt( double* minimumValue, double* maximumValue ) const;

    void          setParentQwtPlotAndReplot( QwtPlot* plot );
    void          setParentQwtPlotNoReplot( QwtPlot* plot );
    void          detachQwtCurve();
    void          reattachQwtCurve();
    QwtPlotCurve* qwtPlotCurve() const;

    void updateCurveNameAndUpdatePlotLegendAndTitle();
    void updateCurveNameNoLegendUpdate();

    void updateCurveVisibility() override;
    void updateLegendEntryVisibilityAndPlotLegend();
    void updateLegendEntryVisibilityNoPlotUpdate() override;

    bool errorBarsVisible() const;

    void setZOrder( double z ) override;
    void setErrorBarsVisible( bool isVisible );

    void replotParentPlot() override;
    void refreshParentPlot() override;
    void updateUiIconFromPlotSymbol() override;
    bool hasParentPlot() const override;
    void updateCurveAppearance() override;

protected:
    void updateCurvePresentation( bool updatePlotLegendAndTitle );

    void updateOptionSensitivity();
    void updatePlotTitle();
    void updateLegendsInPlot() override;
    void setCurveTitle( const QString& title ) override;

    void setSamplesFromXYErrorValues(
        const std::vector<double>&   xValues,
        const std::vector<double>&   yValues,
        const std::vector<double>&   errorValues,
        bool                         keepOnlyPositiveValues,
        RiaCurveDataTools::ErrorAxis errorAxis = RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS ) override;
    void setSamplesFromXYValues( const std::vector<double>& xValues,
                                 const std::vector<double>& yValues,
                                 bool                       keepOnlyPositiveValues ) override;
    void setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                        const std::vector<double>&    yValues,
                                        bool                          keepOnlyPositiveValues ) override;

    void setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                        const std::vector<double>& yValues,
                                        bool                       keepOnlyPositiveValues ) override;

    void attachCurveAndErrorBars() override;
    void clearErrorBars() override;

    void updateAxisInPlot( RiaDefines::PlotAxis plotAxis ) override;

protected:
    QPointer<QwtPlot> m_parentQwtPlot;

    RiuQwtPlotCurve*      m_qwtPlotCurve;
    QwtPlotIntervalCurve* m_qwtCurveErrorBars;
};
