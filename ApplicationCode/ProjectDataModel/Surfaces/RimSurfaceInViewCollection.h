/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

namespace cvf
{
class ModelBasicList;
class Transform;
class ScalarMapper;
} // namespace cvf

class RimSurfaceInView;
class RimSurface;

class RimSurfaceInViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceInViewCollection();
    ~RimSurfaceInViewCollection() override;

    void updateFromSurfaceCollection();

    void appendPartsToModel( cvf::ModelBasicList* surfaceVizModel, cvf::Transform* scaleTransform );
    void updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex );
    void applySingleColorEffect();

    bool hasAnyActiveSeparateResults();

private:
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    bool hasSurfaceInViewForSurface( const RimSurface* surf ) const;

    caf::PdmField<bool>                        m_isActive;
    caf::PdmChildArrayField<RimSurfaceInView*> m_surfacesInView;
};
