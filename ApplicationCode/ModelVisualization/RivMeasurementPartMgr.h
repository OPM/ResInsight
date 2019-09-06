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
#include "cvfAssert.h"
#include "cvfCollection.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf
{
class BoundingBox;
class Camera;
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
class RimMeasurement;

class RivMeasurementPartMgr : public cvf::Object
{
    using Vec3d = cvf::Vec3d;

public:
    RivMeasurementPartMgr( Rim3dView* view );
    ~RivMeasurementPartMgr() override;

    void appendGeometryPartsToModel( const cvf::Camera*                camera,
                                     cvf::ModelBasicList*              model,
                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                     const cvf::BoundingBox&           boundingBox );

    void clearGeometryCache();

private:
    void buildPolyLineParts( const cvf::Camera* camera, const caf::DisplayCoordTransform* displayCoordTransform );

    bool isPolylinesInBoundingBox( const cvf::BoundingBox& boundingBox );

private:
    caf::PdmPointer<Rim3dView>      m_rimView;
    caf::PdmPointer<RimMeasurement> m_measurement;
    cvf::ref<cvf::Part>             m_linePart;
    cvf::ref<cvf::Part>             m_pointPart;
    cvf::ref<cvf::Part>             m_labelPart;
};
