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

#include "RimFracture.h"

namespace caf {
    class PdmFieldHandle;
    class PdmUiEditorAttribute;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathFracture : public RimFracture
{
     CAF_PDM_HEADER_INIT;

public:
    RimWellPathFracture(void);
    virtual ~RimWellPathFracture(void);

    double                          measuredDepth() const;
    void                            setMeasuredDepth(double mdValue);

    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                    updateAzimuthFromFractureDefinition() override;

    double                          wellAzimuthAtFracturePosition() override;

protected:
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

private:
    void                            updatePositionFromMeasuredDepth();

private:
    caf::PdmField<float>            m_measuredDepth;
};
