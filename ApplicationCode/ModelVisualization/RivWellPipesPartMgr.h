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
class RimReservoirView;
class RimWell;

class RivWellPipesPartMgr : public cvf::Object
{
public:
    RivWellPipesPartMgr(RimReservoirView* reservoirView, RimWell* well);
    ~RivWellPipesPartMgr();

    void setScaleTransform(cvf::Transform * scaleTransform) { m_scaleTransform = scaleTransform; scheduleGeometryRegen();}

    void scheduleGeometryRegen() { m_needsTransformUpdate = true; }

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);
    void updatePipeResultColor(size_t frameIndex);


private:
    caf::PdmPointer<RimReservoirView>   m_rimReservoirView;
    caf::PdmPointer<RimWell>            m_rimWell;
    
    cvf::ref<cvf::Transform>    m_scaleTransform; 
    bool                        m_needsTransformUpdate;

    void buildWellPipeParts();
 

    //void calculateWellPipeCenterline(std::vector<cvf::Vec3d>& coords) const;

    void calculateWellPipeCenterline(std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                     std::vector< std::vector <RigWellResultCell> >& pipeBranchesCellIds ) const;

    struct RivPipeBranchData
    {
        std::vector <RigWellResultCell>     m_cellIds;
        //std::vector< std::vector<WellCellStatus> > m_cellStatusPrFrame;
        cvf::ref<RivPipeGeometryGenerator>  m_pipeGeomGenerator;

        cvf::ref<cvf::Part>                 m_surfacePart;
        cvf::ref<cvf::DrawableGeo>          m_surfaceDrawable;
        cvf::ref<cvf::Part>                 m_centerLinePart;
        cvf::ref<cvf::DrawableGeo>          m_centerLineDrawable;

    };

    std::list<RivPipeBranchData> m_wellBranches;

    cvf::ref<cvf::ScalarMapper> m_scalarMapper;
    cvf::ref<cvf::Effect>       m_scalarMapperSurfaceEffect; 
    cvf::ref<cvf::Effect>       m_scalarMapperMeshEffect; 
};
