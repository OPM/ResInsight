/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


#include "cvfCollection.h"
#include "cafPdmPointer.h"

class RimWellPathCollection;
class RimProject;
class RivWellPathPartMgr;

class RivWellPathCollectionPartMgr : public cvf::Object
{
public:
    RivWellPathCollectionPartMgr(RimWellPathCollection* wellPathCollection);
    ~RivWellPathCollectionPartMgr();

    void scheduleGeometryRegen();

    void setScaleTransform(cvf::Transform * scaleTransform);

    void appendStaticGeometryPartsToModel(
        cvf::ModelBasicList* model, 
        cvf::Vec3d displayModelOffset, 
        cvf::Transform* scaleTransform, 
        double characteristicCellSize, 
        cvf::BoundingBox boundingBox);

private:
    caf::PdmPointer<RimWellPathCollection>      m_wellPathCollection;
    cvf::ref<cvf::Transform>                    m_scaleTransform; 
};
