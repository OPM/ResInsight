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

#include "RigSimWellData.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <list>

namespace cvf
{
    class Part;
    class ModelBasicList;
    class Transform;
    class Effect;
    class DrawableGeo;
}

class RivPipeGeometryGenerator;
class RimEclipseView;
class RimSimWellInView;

class RivSimWellPipesPartMgr : public cvf::Object
{
public:
    RivSimWellPipesPartMgr(RimEclipseView* reservoirView, RimSimWellInView* well, bool isFlattened = false);
    ~RivSimWellPipesPartMgr();

    void setScaleTransform(cvf::Transform * scaleTransform);

    void scheduleGeometryRegen();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);
    void updatePipeResultColor(size_t frameIndex);

private:
    caf::PdmPointer<RimEclipseView>   m_rimReservoirView;
    caf::PdmPointer<RimSimWellInView>            m_rimWell;
    
    cvf::ref<cvf::Transform>    m_scaleTransform; 
    bool                        m_needsTransformUpdate;
    bool                        m_isFlattened; 

    void buildWellPipeParts();

    struct RivPipeBranchData
    {
        std::vector <RigWellResultPoint>     m_cellIds;
        cvf::ref<RivPipeGeometryGenerator>  m_pipeGeomGenerator;

        cvf::ref<cvf::Part>                 m_surfacePart;
        cvf::ref<cvf::DrawableGeo>          m_surfaceDrawable;
        cvf::ref<cvf::DrawableGeo>          m_largeSurfaceDrawable;
        cvf::ref<cvf::Part>                 m_centerLinePart;
        cvf::ref<cvf::DrawableGeo>          m_centerLineDrawable;

    };

    RivPipeBranchData* pipeBranchData(size_t branchIndex);

    std::list<RivPipeBranchData> m_wellBranches;

    std::vector< std::vector <cvf::Vec3d> > m_pipeBranchesCLCoords;
};
