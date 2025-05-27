/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 equinor ASA
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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <vector>

namespace cvf
{
class BoundingBox;
class Part;
class ModelBasicList;
class Transform;
class Font;
class Camera;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;
class RimPolylinesDataInterface;
class RigPolyLinesData;

class RivPolylinePartMgr : public cvf::Object
{
public:
    RivPolylinePartMgr( Rim3dView* view, RimPolylinesDataInterface* polylines, caf::PdmObject* collection );
    ~RivPolylinePartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            const caf::DisplayCoordTransform* displayXf,
                                            const cvf::BoundingBox&           boundingBox );

private:
    bool isPolylinesInBoundingBox( std::vector<std::vector<cvf::Vec3d>> polyline, const cvf::BoundingBox& boundingBox );
    void buildPolylineParts( const cvf::Camera* camera, const caf::DisplayCoordTransform* displayXf, const cvf::BoundingBox& boundingBox );

    std::vector<std::vector<cvf::Vec3d>> getPolylinesPointsInDomain( RigPolyLinesData* lineDef );
    std::vector<std::vector<cvf::Vec3d>> transformPolylinesPointsToDisplay( const std::vector<std::vector<cvf::Vec3d>>& pointsInDomain,
                                                                            const caf::DisplayCoordTransform*           displayXf );

    bool collectionVisible();

    void clearAllGeometry();

    RimPolylinesDataInterface* m_polylineInterface;
    caf::PdmObject*            m_viewCollection;
    caf::PdmPointer<Rim3dView> m_rimView;
    cvf::ref<cvf::Part>        m_linePart;
    cvf::ref<cvf::Part>        m_spherePart;
    cvf::ref<cvf::Part>        m_labelPart;
};
