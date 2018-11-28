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

#include "RimAnnotationLineAppearance.h"
#include "RimLineBasedAnnotation.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include "cafPdmUiOrdering.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class QString;
class RimGridView;


//==================================================================================================
///
///
//==================================================================================================
class RimReachCircleAnnotation : public RimLineBasedAnnotation
{
    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;

public:
    RimReachCircleAnnotation();

    Vec3d           centerPoint() const;
    double          radius() const;
    QString         name() const;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    caf::PdmField<Vec3d>    m_centerPointXyd;
    caf::PdmField<double>   m_radius;
    caf::PdmField<QString>  m_name;
};
