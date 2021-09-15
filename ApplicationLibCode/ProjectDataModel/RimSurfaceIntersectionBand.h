////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimCheckableObject.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h" // Include to make Pdm work for cvf::Color
#include "cafPdmPtrArrayField.h"

#include <array>

class RimSurface;
class RimAnnotationLineAppearance;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSurfaceIntersectionBand : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceIntersectionBand();

    RimAnnotationLineAppearance* lineAppearance() const;
    cvf::Color3f                 bandColor() const;
    float                        bandOpacity() const;
    std::array<RimSurface*, 2>   surfaces() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    caf::PdmChildField<RimAnnotationLineAppearance*> m_lineAppearance;
    caf::PdmField<cvf::Color3f>                      m_bandColor;
    caf::PdmField<float>                             m_bandOpacity;
    caf::PdmPtrArrayField<RimSurface*>               m_surfaces;
};
