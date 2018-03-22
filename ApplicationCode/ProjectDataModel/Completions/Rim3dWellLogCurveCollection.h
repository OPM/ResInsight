/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class Rim3dWellLogCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dWellLogCurveCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    Rim3dWellLogCurveCollection();
    virtual ~Rim3dWellLogCurveCollection();

    bool has3dWellLogCurves() const;
    void add3dWellLogCurve(Rim3dWellLogCurve* curve);

    bool isShowingGrid() const;
    bool isShowingPlot() const;

    std::vector<Rim3dWellLogCurve*> vectorOf3dWellLogCurves() const;

private:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual caf::PdmFieldHandle* objectToggleField() override;

private:
    caf::PdmField<bool>                         m_showPlot;
    caf::PdmField<bool>                         m_showGrid;
    caf::PdmChildArrayField<Rim3dWellLogCurve*> m_3dWellLogCurves;
};
