/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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
#include "cvfArray.h"
#include "cvfObject.h"

namespace cvf
{
class ModelBasicList;
class Part;
class DrawableGeo;
class BoundingBox;
class ShaderProgram;
class TextureImage;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;

class RivTexturePartMgr : public cvf::Object
{
public:
    RivTexturePartMgr();
    virtual ~RivTexturePartMgr();

    virtual void appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                             const caf::DisplayCoordTransform* displayCoordTransform,
                                             const cvf::BoundingBox&           boundingBox ) = 0;

    virtual void appendPolylinePartsToModel( Rim3dView*                        view,
                                             cvf::ModelBasicList*              model,
                                             const caf::DisplayCoordTransform* displayCoordTransform,
                                             const cvf::BoundingBox&           boundingBox ) = 0;

protected:
    cvf::ref<cvf::DrawableGeo> createXYPlaneQuadGeoWithTexCoords( const cvf::Vec3dArray& cornerPoints );
    cvf::ref<cvf::Part> createSingleTexturedQuadPart( const cvf::Vec3dArray& cornerPoints, cvf::ref<cvf::TextureImage> image, bool transparent );

    cvf::ref<cvf::ShaderProgram> m_textureShaderProg;

    bool m_canUseShaders;
};
