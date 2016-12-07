/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"
#include <list>
#include "RigSingleWellResultsData.h"

namespace cvf
{
    class Part;
    class ModelBasicList;
    class Transform;
    class Effect;
    class DrawableGeo;
    class ScalarMapper;
}

class RivPipeGeometryGenerator;
class RimEclipseView;
class RimEclipseWell;

class RivWellSpheresPartMgr : public cvf::Object
{
public:
    RivWellSpheresPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well);
    ~RivWellSpheresPartMgr();

    void setScaleTransform(cvf::Transform * scaleTransform) { m_scaleTransform = scaleTransform; scheduleGeometryRegen();}

    void scheduleGeometryRegen() { m_needsTransformUpdate = true; }

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);

    
private:
    static cvf::ref<cvf::DrawableGeo> createSphere(double radius, const cvf::Vec3d& pos);
    cvf::ref<cvf::Part> createPart(cvf::DrawableGeo* geo);

private:
    caf::PdmPointer<RimEclipseView>   m_rimReservoirView;
    caf::PdmPointer<RimEclipseWell>            m_rimWell;


    struct RivPipeBranchData
    {
        cvf::ref<cvf::Part>                 m_surfacePart;
        cvf::ref<cvf::DrawableGeo>          m_surfaceDrawable;
    };

    
    cvf::ref<cvf::Transform>    m_scaleTransform; 
    bool                        m_needsTransformUpdate;
};
