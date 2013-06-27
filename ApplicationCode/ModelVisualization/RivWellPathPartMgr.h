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

#include "cafPdmPointer.h"
#include "cvfBoundingBox.h"
#include <list>

namespace cvf
{
    class Part;
    class ModelBasicList;
    class Transform;
    class Effect;
}

class RivPipeGeometryGenerator;
class RimProject;
class RimWellPath;
class RimWellPathCollection;

class RivWellPathPartMgr : public cvf::Object
{
public:
    RivWellPathPartMgr(RimWellPathCollection* wellPathCollection, RimWellPath* wellPath);
    ~RivWellPathPartMgr();

    void                                    setScaleTransform(cvf::Transform * scaleTransform);

    void                                    scheduleGeometryRegen() { m_needsTransformUpdate = true; }//printf("R"); }

    void                                    appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, cvf::Vec3d displayModelOffset, double characteristicCellSize, cvf::BoundingBox boundingBox);

private:
    caf::PdmPointer<RimWellPathCollection>  m_wellPathCollection;
    caf::PdmPointer<RimWellPath>            m_rimWellPath;
    
    cvf::ref<cvf::Transform>                m_scaleTransform; 
    bool                                    m_needsTransformUpdate;

    void                                    buildWellPathParts(cvf::Vec3d displayModelOffset, double characteristicCellSize, cvf::BoundingBox boundingBox);

    struct RivPipeBranchData
    {
        cvf::ref<RivPipeGeometryGenerator>  m_pipeGeomGenerator;
        cvf::ref<cvf::Part>                 m_surfacePart;
        cvf::ref<cvf::DrawableGeo>          m_surfaceDrawable;
        cvf::ref<cvf::Part>                 m_centerLinePart;
        cvf::ref<cvf::DrawableGeo>          m_centerLineDrawable;
    };

    RivPipeBranchData                       m_pipeBranchData;
    std::list<RivPipeBranchData>            m_wellBranches;
    cvf::ref<cvf::Part>                     m_wellLabelPart;

    cvf::ref<cvf::ScalarMapper>             m_scalarMapper;
    cvf::ref<cvf::Effect>                   m_scalarMapperSurfaceEffect; 
    cvf::ref<cvf::Effect>                   m_scalarMapperMeshEffect; 
};
