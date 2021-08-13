/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "cafPdmPointer.h"

#include <gsl/gsl>

class RimWellPathGeometryDef;
class RigWellPath;

//==================================================================================================
///
//==================================================================================================
class RicCreateWellTargetsPickEventHandler : public Ric3dViewPickEventHandler
{
public:
    RicCreateWellTargetsPickEventHandler( gsl::not_null<RimWellPathGeometryDef*> wellGeometryDef );
    ~RicCreateWellTargetsPickEventHandler();

    void registerAsPickEventHandler() override;

protected:
    bool handle3dPickEvent( const Ric3dPickEvent& eventObject ) override;
    void notifyUnregistered() override;

private:
    static bool       isGridSourceObject( const cvf::Object* object );
    static cvf::Vec3d findHexElementIntersection( gsl::not_null<Rim3dView*> view,
                                                  const RiuPickItemInfo&    pickItem,
                                                  const cvf::Vec3d&         domainRayOrigin,
                                                  const cvf::Vec3d&         domainRayEnd );

private:
    caf::PdmPointer<RimWellPathGeometryDef> m_geometryToAddTargetsTo;
};
