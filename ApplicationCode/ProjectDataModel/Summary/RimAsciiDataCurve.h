/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

#include "RifEclipseSummaryAddress.h"
#include "RiaDefines.h"
#include "RimPlotCurve.h"

#include "cafAppEnum.h"

#include <QDateTime>

class RifReaderEclipseSummary;
class RimSummaryCase;
class RimSummaryFilter;
class RiuLineSegmentQwtPlotCurve;
class RimAsciiDataCurveAutoName;

//==================================================================================================
///  
///  
//==================================================================================================
class RimAsciiDataCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimAsciiDataCurve();
    virtual ~RimAsciiDataCurve();

    std::vector<double>                     yValues() const;
    const std::vector<time_t>&              timeSteps() const;

    void                                    setYAxis(RiaDefines::PlotAxis plotAxis);
    RiaDefines::PlotAxis                    yAxis() const;
    void                                    updateQwtPlotAxis();

    void                                    setTimeSteps(const std::vector<QDateTime>& timeSteps);
    void                                    setValues(const std::vector<double>& values);
    void                                    setTitle(const QString& title);

protected:
    // RimPlotCurve overrides

    virtual QString                         createCurveAutoName() override;
    virtual void                            updateZoomInParentPlot() override;
    virtual void                            onLoadDataAndUpdate(bool updateParentPlot) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    bool                                    curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values) const;

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    // Fields
    caf::PdmField< caf::AppEnum< RiaDefines::PlotAxis > > m_plotAxis;

    caf::PdmField< std::vector<QDateTime> > m_timeSteps;
    caf::PdmField< std::vector<double> >    m_values;
    caf::PdmField< QString >                m_title;
};
