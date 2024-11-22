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

#include "RimPlotCurve.h"

#include "cafAppEnum.h"

#include <QDateTime>

class RifReaderEclipseSummary;
class RimSummaryCase;
class RiuQwtPlotCurve;
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
    ~RimAsciiDataCurve() override;

    std::vector<double>        yValues() const;
    const std::vector<time_t>& timeSteps() const;

    void        setYAxis( RiaDefines::PlotAxis plotAxis );
    RiuPlotAxis yAxis() const;
    void        updateQwtPlotAxis();

    void setTimeSteps( const std::vector<QDateTime>& timeSteps );
    void setValues( const std::vector<double>& values );

private:
    void updateZoomInParentPlot() override;
    void onLoadDataAndUpdate( bool updateParentPlot ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

    bool curveData( std::vector<QDateTime>* timeSteps, std::vector<double>* values ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    // Fields
    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis;

    caf::PdmField<std::vector<QDateTime>> m_timeSteps;
    caf::PdmField<std::vector<double>>    m_values;
    caf::PdmField<QString>                m_title_OBSOLETE;
};
