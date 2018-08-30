/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimPlotCurve.h"

#include "cafPdmBase.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RigWellLogCurveData;
class RimWellPathAttribute;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathAttributeCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;
    enum CurvePlotPosition
    {
        NegativeSide,
        PositiveSide
    };
    enum CurvePlotItem
    {
        LineCurve,
        MarkerSymbol        
    };
public:
    RimWellPathAttributeCurve(RimWellPathAttribute* wellPathAttribute = nullptr, CurvePlotPosition plotPosition = PositiveSide, CurvePlotItem curvePlotItem = LineCurve);
    ~RimWellPathAttributeCurve();

    void updateZoomInParentPlot();
    QString createCurveAutoName();
    void onLoadDataAndUpdate(bool updateParentPlot);

    virtual bool yValueRange(double* minimumValue, double* maximumValue) const override;

private:

    caf::PdmPtrField<RimWellPathAttribute*> m_wellPathAttribute;
    CurvePlotPosition                       m_curvePlotPosition;
    CurvePlotItem                           m_curvePlotItem;
};
