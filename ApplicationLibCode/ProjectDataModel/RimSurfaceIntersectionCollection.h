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

#include "cafPdmChildArrayField.h"

class RimSurfaceIntersectionBand;
class RimSurfaceIntersectionCurve;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSurfaceIntersectionCollection : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimSurfaceIntersectionCollection();

    RimSurfaceIntersectionCurve* addIntersectionCurve();
    RimSurfaceIntersectionBand*  addIntersectionBand();

    std::vector<RimSurfaceIntersectionCurve*> surfaceIntersectionCurves() const;
    std::vector<RimSurfaceIntersectionBand*>  surfaceIntersectionBands() const;

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void onObjectChanged( const caf::SignalEmitter* emitter );
    void initAfterRead() override;

private:
    caf::PdmChildArrayField<RimSurfaceIntersectionBand*>  m_intersectionBands;
    caf::PdmChildArrayField<RimSurfaceIntersectionCurve*> m_intersectionCurves;
};
