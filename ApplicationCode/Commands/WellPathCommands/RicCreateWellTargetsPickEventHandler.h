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

class RimWellPathGeometryDefInterface;
class RigWellPath;
class RiuPickItemInfo;
class RivWellPathSourceInfo;
class RimWellPathGeometryDef;
class RimWellPathLateralGeometryDef;

//==================================================================================================
///
//==================================================================================================
class RicCreateWellTargetsPickEventHandler : public Ric3dViewPickEventHandler
{
public:
    RicCreateWellTargetsPickEventHandler( gsl::not_null<RimWellPathGeometryDefInterface*> wellGeometryDef );
    ~RicCreateWellTargetsPickEventHandler();

    void registerAsPickEventHandler() override;

protected:
    bool handle3dPickEvent( const Ric3dPickEvent& eventObject ) override;
    void notifyUnregistered() override;

private:
    bool calculateAzimuthAndInclinationAtMd( double                            measuredDepth,
                                             gsl::not_null<const RigWellPath*> wellPathGeometry,
                                             double*                           azimuth,
                                             double*                           inclination ) const;
    bool calculateWellPathGeometryAtPickPoint( const RiuPickItemInfo&                      pickItem,
                                               gsl::not_null<const RivWellPathSourceInfo*> sourceInfo,
                                               const cvf::Vec3d&                           intersectionPointInDomain,
                                               gsl::not_null<cvf::Vec3d*>                  targetPointInDomain,
                                               gsl::not_null<double*>                      azimuth,
                                               gsl::not_null<double*>                      inclination ) const;

    cvf::Vec3d calculateGridPickPoint( gsl::not_null<const Rim3dView*> rimView,
                                       const RiuPickItemInfo&          pickItem,
                                       const cvf::Vec3d&               intersectionPointInDomain ) const;

    void addNewTargetToModeledWellPath( const RiuPickItemInfo&                 pickItem,
                                        gsl::not_null<RimWellPathGeometryDef*> wellPathGeometryDef,
                                        const cvf::Vec3d&                      intersectionPointInDomain,
                                        const cvf::Vec3d&                      targetPointInDomain,
                                        double                                 azimuth,
                                        double                                 inclination );

    void addNewTargetToModeledWellPathLateral( const RiuPickItemInfo&                        pickItem,
                                               gsl::not_null<RimWellPathLateralGeometryDef*> wellPathLateralGeometryDef,
                                               const cvf::Vec3d&                             intersectionPointInDomain,
                                               const cvf::Vec3d&                             targetPointInDomain,
                                               double                                        azimuth,
                                               double                                        inclination );

    static bool       isGridSourceObject( const cvf::Object* object );
    static bool       isValidWellPathSourceObject( const RivWellPathSourceInfo* sourceInfo );
    static cvf::Vec3d findHexElementIntersection( gsl::not_null<const Rim3dView*> view,
                                                  const RiuPickItemInfo&          pickItem,
                                                  const cvf::Vec3d&               domainRayOrigin,
                                                  const cvf::Vec3d&               domainRayEnd );

private:
    caf::PdmPointer<RimWellPathGeometryDefInterface> m_geometryToAddTargetsTo;
};
