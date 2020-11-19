/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "cvfAssert.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <vector>

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
class RimCellFilterCollection;

class RivPolylineSelectionPartMgr : public cvf::Object
{
    using Vec3d = cvf::Vec3d;

public:
    RivPolylineSelectionPartMgr( Rim3dView* view );
    ~RivPolylineSelectionPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            const caf::DisplayCoordTransform* displayXf,
                                            const cvf::BoundingBox&           boundingBox );

    void clearAllGeometry();

private:
    void buildPolylineParts( const caf::DisplayCoordTransform* displayXf );
    bool isPolylinesInBoundingBox( const cvf::BoundingBox& boundingBox );

    RimCellFilterCollection* cellFilterCollection() const;

    caf::PdmPointer<Rim3dView>               m_rimView;
    caf::PdmPointer<RimCellFilterCollection> m_cellfilterCollection;
    cvf::ref<cvf::Part>                      m_linePart;
    cvf::ref<cvf::Part>                      m_spherePart;
};
