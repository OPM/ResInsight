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

#include "cafPdmObject.h"

#include "cvfObject.h"
#include "cvfVector3.h"

class RimWellPath;
class RimWellPathTarget;
class RicCreateWellTargetsPickEventHandler;

class RiaLineArcWellPathCalculator;
class RigWellPath;

class RimWellPathGeometryDefInterface : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    virtual cvf::ref<RigWellPath> createWellPathGeometry()                                                        = 0;
    virtual cvf::Vec3d            anchorPointXyz() const                                                          = 0;
    virtual void insertTarget( const RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert ) = 0;
    virtual void deleteTarget( RimWellPathTarget* targetTodelete )                                                = 0;
    virtual void deleteAllTargets()                                                                               = 0;
    virtual RimWellPathTarget* appendTarget()                                                                     = 0;

    virtual const RimWellPathTarget* firstActiveTarget() const = 0;
    virtual const RimWellPathTarget* lastActiveTarget() const  = 0;

    virtual void                            enableTargetPointPicking( bool isEnabling )    = 0;
    virtual std::vector<RimWellPathTarget*> activeWellTargets() const                      = 0;
    virtual void                            updateWellPathVisualization( bool fullUpdate ) = 0;

private:
    virtual RiaLineArcWellPathCalculator lineArcWellPathCalculator() const = 0;
};
