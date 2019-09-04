/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "cvfObject.h"

#include "cafPdmPointer.h"

#include <QDateTime>

#include <map>

namespace cvf
{
class BoundingBox;
class Transform;
class ModelBasicList;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;
class RivWellPathPartMgr;
class RimWellPath;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RivWellPathsPartMgr : public cvf::Object
{
public:
    explicit RivWellPathsPartMgr(Rim3dView* view);
    ~RivWellPathsPartMgr() override;

    void appendStaticGeometryPartsToModel(cvf::ModelBasicList*              model,
                                          const caf::DisplayCoordTransform* displayCoordTransform,
                                          double                            characteristicCellSize,
                                          const cvf::BoundingBox&           wellPathClipBoundingBox);


    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList*              model,
                                           size_t                            timeStepIndex,
                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                           double                            characteristicCellSize,
                                           const cvf::BoundingBox&           wellPathClipBoundingBox);

    void appendStaticFracturePartsToModel(cvf::ModelBasicList* model, const cvf::BoundingBox& wellPathClipBoundingBox);

private:
    void                   clearGeometryCache();
    void                   scheduleGeometryRegen();
    void                   createPartManagersIfRequired();
    bool                   isWellPathVisible() const;

private:
    caf::PdmPointer<Rim3dView>                  m_rimView;
    cvf::Collection<RivWellPathPartMgr>         m_wellPathsPartMgrs;
    std::map<RimWellPath*, RivWellPathPartMgr*> m_mapFromViewToIndex;
};
