/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 equinor ASA
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
#include "RimPolylinesAnnotation.h"

#include "cafPdmFieldCvfVec3d.h"

//==================================================================================================
///
///
//==================================================================================================

class RimUserDefinedPolylinesAnnotation : public RimPolylinesAnnotation
{
    friend class RimUserDefinedPolylinesAnnotationInView;

    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;
public:
    RimUserDefinedPolylinesAnnotation();
    ~RimUserDefinedPolylinesAnnotation();

    cvf::ref<RigPolyLinesData>  polyLinesData() override;
    virtual bool isEmpty() override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<std::vector<Vec3d>>   m_pointsXyd;
};


