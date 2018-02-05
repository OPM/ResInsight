/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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
#include "cvfCollection.h"
#include "cvfObject.h"

#include <QDateTime>
#include <map>
#include "cafPdmPointer.h"


namespace cvf
{
    class BoundingBox;
    class Transform;
    class ModelBasicList;
}

namespace caf
{
    class DisplayCoordTransform;
}

class Rim3dView;
class RivWellPathPartMgr;
class RimWellPathCollection;
class RimWellPath;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivWellPathsPartMgr : public cvf::Object
{
public:
    explicit RivWellPathsPartMgr(Rim3dView* view);
    ~RivWellPathsPartMgr();

    void                          appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                   double characteristicCellSize,
                                                                   const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                   const caf::DisplayCoordTransform* displayCoordTransform);

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    void                          appendStaticFracturePartsToModel(cvf::ModelBasicList* model, 
                                                                   const Rim3dView* rimView);
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    void                          appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                    const QDateTime& timeStamp,
                                                                    double characteristicCellSize, 
                                                                    const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                    const caf::DisplayCoordTransform* displayCoordTransform);

    size_t                        segmentIndexFromTriangleIndex(size_t triangleIndex, RimWellPath* wellPath) const;




private:
    void clearGeometryCache();
    void scheduleGeometryRegen();
    void buildPartManagers();
    RimWellPathCollection* wellPathCollection() const;

private:
    caf::PdmPointer<Rim3dView>                m_rimView;

    cvf::Collection<RivWellPathPartMgr>  m_wellPatshsPartMgrs;
    std::map<RimWellPath*, RivWellPathPartMgr*> m_mapFromViewToIndex;
};
