/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RiaFontCache.h"
#include "RimViewWindow.h"

namespace caf
{
class PdmObject;
}

class QwtPlotCurve;

class RimRiuQwtPlotOwnerInterface
{
public:
    virtual void detachAllCurves() = 0;
    virtual void updateAxisScaling() = 0;
    virtual void updateAxisDisplay() = 0;
    virtual void updateZoomWindowFromQwt() = 0;
    virtual void selectAxisInPropertyEditor(int axis) = 0;
    virtual void setAutoZoomForAllAxes(bool enableAutoZoom) = 0;
    
    virtual caf::PdmObject* findRimPlotObjectFromQwtCurve(const QwtPlotCurve* curve) const = 0;

};