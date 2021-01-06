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

#include "cvfObject.h"

#include "cvfCollection.h"
#include "cvfColor4.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include <map>

namespace cvf
{
class ModelBasicList;
class Part;
class DrawableGeo;
class Ray;
class HitItem;
class String;

template <typename>
class Array;
typedef Array<Vec3f> Vec3fArray;
typedef Array<uint>  UIntArray;

} // namespace cvf

class RicPointTangentManipulatorPartMgr : public cvf::Object
{
public:
    enum HandleType
    {
        HORIZONTAL_PLANE,
        VERTICAL_AXIS,
        AZIMUTH,
        INCLINATION,
        NONE
    };

public:
    RicPointTangentManipulatorPartMgr();
    ~RicPointTangentManipulatorPartMgr() override;

    void setOrigin( const cvf::Vec3d& origin );
    void setTangent( const cvf::Vec3d& tangent );
    void setHandleSize( double handleSize );
    void originAndTangent( cvf::Vec3d* origin, cvf::Vec3d* tangent );

    bool isManipulatorActive() const;
    void tryToActivateManipulator( const cvf::HitItem* hitItem );
    void updateManipulatorFromRay( const cvf::Ray* ray );
    void endManipulator();

    void appendPartsToModel( cvf::ModelBasicList* model );

private:
    void createGeometryOnly();
    void recreateAllGeometryAndParts();

    void                       createHorizontalPlaneHandle();
    cvf::ref<cvf::DrawableGeo> createHorizontalPlaneGeo();

    void                       createVerticalAxisHandle();
    cvf::ref<cvf::DrawableGeo> createVerticalAxisGeo();

    void addHandlePart( cvf::DrawableGeo* geo, const cvf::Color4f& color, HandleType handleId, const cvf::String& partName );

    void addActiveModePart( cvf::DrawableGeo* geo, const cvf::Color4f& color, HandleType handleId, const cvf::String& partName );

    static cvf::ref<cvf::DrawableGeo> createTriangelDrawableGeo( cvf::Vec3fArray* triangleVertexArray );
    static cvf::ref<cvf::DrawableGeo> createIndexedTriangelDrawableGeo( cvf::Vec3fArray* triangleVertexArray,
                                                                        cvf::UIntArray*  triangleIndices );
    static cvf::ref<cvf::Part> createPart( cvf::DrawableGeo* geo, const cvf::Color4f& color, const cvf::String& partName );

private:
    std::map<HandleType, cvf::ref<cvf::Part>> m_handleParts; // These arrays have the same length
    cvf::Collection<cvf::Part>                m_activeDragModeParts;

    cvf::Vec3d m_origin;
    cvf::Vec3d m_tangent;
    double     m_handleSize;
    bool       m_isGeometryUpdateNeeded;

    HandleType m_activeHandle;
    cvf::Vec3d m_initialPickPoint;
    cvf::Vec3d m_tangentOnStartManipulation;
    cvf::Vec3d m_originOnStartManipulation;
};
