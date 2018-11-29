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

#include "RimAnnotationCollectionBase.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafTristate.h"

class RimTextAnnotation;

//==================================================================================================
///  
///  
//==================================================================================================
class RimAnnotationInViewCollection : public RimAnnotationCollectionBase
{
    CAF_PDM_HEADER_INIT;
public:

    RimAnnotationInViewCollection();
    ~RimAnnotationInViewCollection() override;

    bool                        isActive() const;
    double                      annotationPlaneZ() const;
    bool                        snapAnnotations() const;

protected:
    void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    caf::PdmFieldHandle*        objectToggleField() override;

private:
    caf::PdmField<bool>         m_isActive;
    caf::PdmField<double>       m_annotationPlaneDepth;
    caf::PdmField<bool>         m_snapAnnotations;
};
