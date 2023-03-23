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
class Transform;
class Part;
class ScalarMapper;
class DrawableGeo;
class BoundingBox;
class ShaderProgram;
class TextureImage;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

namespace ZGYAccess
{
class SeismicSliceData;
}

class RimSeismicSectionCollection;
class RimSeismicSection;
class Rim3dView;
class RivPolylinePartMgr;

class RivSeismicSectionPartMgr : public cvf::Object
{
public:
    explicit RivSeismicSectionPartMgr( RimSeismicSection* section );

    void appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                     const cvf::BoundingBox&           boundingBox );

    void appendPolylinePartsToModel( Rim3dView*                        view,
                                     cvf::ModelBasicList*              model,
                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                     const cvf::BoundingBox&           boundingBox );

protected:
    cvf::ref<cvf::DrawableGeo> createXYPlaneQuadGeoWithTexCoords( const cvf::Vec3dArray& cornerPoints );
    cvf::ref<cvf::Part>        createSingleTexturedQuadPart( const cvf::Vec3dArray& cornerPoints, cvf::ref<cvf::TextureImage> image );

    cvf::TextureImage* createImageFromData( ZGYAccess::SeismicSliceData* data );

private:
    caf::PdmPointer<RimSeismicSection> m_section;
    cvf::ref<RivPolylinePartMgr>       m_polylinePartMgr;
    cvf::ref<cvf::ShaderProgram>       m_textureShaderProg;

    bool m_canUseShaders;
};
