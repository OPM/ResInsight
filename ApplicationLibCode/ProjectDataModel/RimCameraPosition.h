/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "cafPdmObject.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;

//==================================================================================================
///
///
//==================================================================================================
class RimCameraPosition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCameraPosition();

    RimEclipseCase* eclipseCase() const;
    void            setEclipseCase( RimEclipseCase* eclipseCase );

    cvf::Mat4d cameraPosition() const;
    void       setCameraPosition( const cvf::Mat4d& cameraPosition );

    cvf::Vec3d cameraPointOfInterest() const;
    void       setCameraPointOfInterest( const cvf::Vec3d& cameraPointOfInterest );

private:
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;
    caf::PdmField<cvf::Mat4d>         m_cameraPosition;
    caf::PdmField<cvf::Vec3d>         m_cameraPointOfInterest;
};
