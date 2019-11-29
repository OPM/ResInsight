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
#include "RimPlotCurve.h"

#include "cvfObject.h"

class RigWellLogCurveData;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
public:
    RimWellLogCurve();
    ~RimWellLogCurve() override;

    bool xValueRangeInData( double* minimumValue, double* maximumValue ) const;
    bool yValueRangeInData( double* minimumValue, double* maximumValue ) const;

    void setValuesAndDepths( const std::vector<double>& xValues,
                             const std::vector<double>& depths,
                             RiaDefines::DepthTypeEnum  depthType,
                             RiaDefines::DepthUnitType  depthUnit,
                             bool                       isExtractionCurve );
    void setValuesWithTVD( const std::vector<double>& xValues,
                           const std::vector<double>& measuredDepths,
                           const std::vector<double>& tvDepths,
                           RiaDefines::DepthUnitType  depthUnit,
                           bool                       isExtractionCurve );
    void setValuesAndDepths( const std::vector<double>&                                      xValues,
                             const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                             RiaDefines::DepthUnitType                                       depthUnit,
                             bool                                                            isExtractionCurve );

    const RigWellLogCurveData* curveData() const;

    virtual QString wellName() const           = 0;
    virtual QString wellLogChannelName() const = 0;
    virtual QString wellDate() const
    {
        return "";
    };

protected:
    void updateZoomInParentPlot() override;
    void updateLegendsInPlot() override;
    void setOverrideCurveDataXRange( double minimumValue, double maximumValue );
    void calculateCurveDataXRange();

private:
    cvf::ref<RigWellLogCurveData> m_curveData;
    std::pair<double, double>     m_curveDataXRange;
};
