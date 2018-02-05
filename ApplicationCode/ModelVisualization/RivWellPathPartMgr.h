/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include "cvfCollection.h"

namespace cvf
{
    class Part;
    class ModelBasicList;
    class Transform;
    class Effect;
    class DrawableGeo;
    class ScalarMapper;
}

namespace caf
{
    class DisplayCoordTransform;
}

class RivPipeGeometryGenerator;
class RimProject;
class RimWellPath;
class RivFishbonesSubsPartMgr;
class RimWellPathCollection;
class Rim3dView;

class QDateTime;

class RivWellPathPartMgr : public cvf::Object
{
public:
    explicit RivWellPathPartMgr(RimWellPath* wellPath, Rim3dView* view);
    ~RivWellPathPartMgr();

    void                          appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                   double characteristicCellSize,
                                                                   const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                   const caf::DisplayCoordTransform* displayCoordTransform);

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    void                          appendStaticFracturePartsToModel(cvf::ModelBasicList* model, 
                                                                   const Rim3dView* eclView);
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    void                          appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                    const QDateTime& timeStamp,
                                                                    double characteristicCellSize, 
                                                                    const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                    const caf::DisplayCoordTransform* displayCoordTransform);

    size_t                        segmentIndexFromTriangleIndex(size_t triangleIndex);

private:
    void                          appendFishboneSubsPartsToModel(cvf::ModelBasicList* model,
                                                                 const caf::DisplayCoordTransform* displayCoordTransform,
                                                                 double characteristicCellSize);

    void                          appendImportedFishbonesToModel(cvf::ModelBasicList* model,
                                                                 const caf::DisplayCoordTransform* displayCoordTransform,
                                                                 double characteristicCellSize);

    void                          appendPerforationsToModel(const QDateTime& currentViewDate,
                                                            cvf::ModelBasicList* model,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            double characteristicCellSize);


    void                          buildWellPathParts(const caf::DisplayCoordTransform* displayCoordTransform,
                                                     double characteristicCellSize,
                                                     const cvf::BoundingBox& wellPathClipBoundingBox);

    void                          clearAllBranchData();
    inline RimWellPathCollection* wellPathCollection();
    inline double                 wellPathRadius(double characteristicCellSize, RimWellPathCollection* wellPathCollection);

private:
    caf::PdmPointer<RimWellPath>            m_rimWellPath;
    caf::PdmPointer<Rim3dView>              m_rimView;
    
    struct RivPipeBranchData
    {
        cvf::ref<RivPipeGeometryGenerator>  m_pipeGeomGenerator;
        cvf::ref<cvf::Part>                 m_surfacePart;
        cvf::ref<cvf::DrawableGeo>          m_surfaceDrawable;
        cvf::ref<cvf::Part>                 m_centerLinePart;
        cvf::ref<cvf::DrawableGeo>          m_centerLineDrawable;
    };

    RivPipeBranchData                       m_pipeBranchData;
    cvf::ref<cvf::Part>                     m_wellLabelPart;
};
