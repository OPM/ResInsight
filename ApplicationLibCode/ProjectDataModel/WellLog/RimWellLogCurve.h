/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "RiaWellLogUnitTools.h"
#include "RimStackablePlotCurve.h"

#include "cafSignal.h"
#include "cvfObject.h"

class RigWellLogCurveData;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogCurve : public RimStackablePlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogCurve();
    ~RimWellLogCurve() override;

    void setDepthUnit( RiaDefines::DepthUnitType depthUnit );

    bool xValueRangeInData( double* minimumValue, double* maximumValue ) const;
    bool yValueRangeInData( double* minimumValue, double* maximumValue ) const;

    void setValuesAndDepths( const std::vector<double>& xValues,
                             const std::vector<double>& depths,
                             RiaDefines::DepthTypeEnum  depthType,
                             double                     rkbDiff,
                             RiaDefines::DepthUnitType  depthUnit,
                             bool                       isExtractionCurve,
                             const QString&             xUnits = RiaWellLogUnitTools<double>::noUnitString() );
    void setValuesWithMdAndTVD( const std::vector<double>& xValues,
                                const std::vector<double>& measuredDepths,
                                const std::vector<double>& tvDepths,
                                double                     rkbDiff,
                                RiaDefines::DepthUnitType  depthUnit,
                                bool                       isExtractionCurve,
                                const QString&             xUnits = RiaWellLogUnitTools<double>::noUnitString() );
    void setValuesAndDepths( const std::vector<double>&                                      xValues,
                             const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                             double                                                          rkbDiff,
                             RiaDefines::DepthUnitType                                       depthUnit,
                             bool                                                            isExtractionCurve,
                             const QString& xUnits = RiaWellLogUnitTools<double>::noUnitString() );

    const RigWellLogCurveData* curveData() const;

    void updateCurveAppearance() override;

    virtual QString wellName() const             = 0;
    virtual QString wellLogChannelUiName() const = 0;
    virtual QString wellLogChannelName() const;
    virtual QString wellLogChannelUnits() const = 0;
    virtual QString wellDate() const { return ""; };

    static QString wellLogCurveIconName();

    void setOverrideCurveData( const std::vector<double>&               xValues,
                               const std::vector<double>&               depthValues,
                               const RiaCurveDataTools::CurveIntervals& curveIntervals );

    virtual RiaDefines::PhaseType resultPhase() const;

protected:
    void updateZoomInParentPlot() override;
    void updateLegendsInPlot() override;
    void setOverrideCurveDataXRange( double minimumValue, double maximumValue );
    void calculateCurveDataXRange();
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    cvf::ref<RigWellLogCurveData> m_curveData;
    std::pair<double, double>     m_curveDataXRange;
};
