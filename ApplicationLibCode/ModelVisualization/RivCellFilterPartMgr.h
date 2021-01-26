/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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
#include "cvfAssert.h"
#include "cvfCollection.h"
#include "cvfObject.h"

namespace cvf
{
class BoundingBox;
class Part;
class ModelBasicList;
class Transform;
class Font;
} // namespace cvf
namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;
class RivPolylinePartMgr;

class RivCellFilterPartMgr : public cvf::Object
{
public:
    RivCellFilterPartMgr( Rim3dView* view );
    ~RivCellFilterPartMgr() override;

    void appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                     const cvf::BoundingBox&           boundingBox );

    void clearGeometryCache();

private:
    void createCellFilterPartManagers();

private:
    caf::PdmPointer<Rim3dView>          m_rimView;
    cvf::Collection<RivPolylinePartMgr> m_cellFilterPartMgrs;
};
