/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "Ric3dViewPickEventHandler.h"

#include "cafPdmUiCoreVec3d.h"

class Rim3dView;

//==================================================================================================
/// A 3d view pick handler for Vec3d fields
//==================================================================================================
class RicVec3dPickEventHandler : public Ric3dViewPickEventHandler
{
public:
    RicVec3dPickEventHandler( caf::PdmField<cvf::Vec3d>* vectorField, double zOffsetFactor = 0.0 );
    bool handle3dPickEvent( const Ric3dPickEvent& eventObject ) override;

    void registerAsPickEventHandler() override;
    void notifyUnregistered() override;

private:
    caf::PdmField<cvf::Vec3d>* m_vectorField;
    double                     m_zOffsetFactor;
};
