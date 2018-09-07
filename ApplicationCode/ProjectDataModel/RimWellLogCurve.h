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
    virtual ~RimWellLogCurve();

    bool                            depthRange(double* minimumDepth, double* maximumDepth) const;
    bool                            valueRange(double* minimumValue, double* maximumValue) const;
    
    const RigWellLogCurveData*      curveData() const;
    
    virtual QString                 wellName() const = 0;
    virtual QString                 wellLogChannelName() const = 0;
    virtual QString                 wellDate() const  { return ""; };

protected:
    virtual void                    updateZoomInParentPlot() override;
    virtual void                    updateLegendsInPlot() override;

protected:
    cvf::ref<RigWellLogCurveData>   m_curveData;    
};
