/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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
#include "cvfObject.h"

#include "cvfVector3.h"
#include "cvfCollection.h"
#include "cvfMatrix4.h"
#include "cvfColor4.h"

namespace cvf
{

class ModelBasicList;
class Part;
class DrawableGeo;
class Ray;
class HitItem;
class String;

template <typename> class Array;
typedef Array<Vec3f>   Vec3fArray;
typedef Array<uint>   UIntArray;

}

class RicPointTangentManipulatorPartMgr : public cvf::Object
{
public:
    enum HandleType
    {
        HORIZONTAL_PLANE,
        VERTICAL_AXIS,
        AZIMUTH, 
        INCLINATION
    };

public:
    RicPointTangentManipulatorPartMgr();
    ~RicPointTangentManipulatorPartMgr() override;

    void setOrigin(const cvf::Vec3d& origin);
    void setTangent(const cvf::Vec3d& tangent);
    void setHandleSize(double handleSize);
    void originAndTangent(cvf::Vec3d* origin, cvf::Vec3d* tangent);

    bool isManipulatorActive() const;
    void tryToActivateManipulator(const cvf::HitItem* hitItem);
    void updateManipulatorFromRay(const cvf::Ray* ray);
    void endManipulator();

    void appendPartsToModel(cvf::ModelBasicList* model);

private:
    void createAllHandleParts();
    void clearAllGeometryAndParts();
    void recreateAllGeometryAndParts();

    void createHorizontalPlaneHandle();
    void createVerticalAxisHandle();

    void addHandlePart(cvf::DrawableGeo* geo,
                       const cvf::Color4f& color, 
                       HandleType handleId, 
                       const cvf::String& partName);

    void addActiveModePart(cvf::DrawableGeo* geo, 
                           const cvf::Color4f& color, 
                           HandleType handleId, 
                           const cvf::String& partName);

    static cvf::ref<cvf::DrawableGeo> createTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray);
    static cvf::ref<cvf::DrawableGeo> createIndexedTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray, 
                                                                       cvf::UIntArray* triangleIndices);
    static cvf::ref<cvf::Part> createPart(cvf::DrawableGeo* geo,
                                          const cvf::Color4f& color,
                                          const cvf::String& partName);
private:
    size_t                      m_currentHandleIndex;
    std::vector< HandleType >   m_handleIds;             // These arrays have the same length
    cvf::Collection<cvf::Part>  m_handleParts;           // These arrays have the same length
    cvf::Collection<cvf::Part>  m_activeDragModeParts;
    cvf::Vec3d                  m_origin;
    cvf::Vec3d                  m_tangent;
    double                      m_handleSize;
    cvf::Vec3d                  m_initialPickPoint;
    cvf::Vec3d                  m_tangentOnStartManipulation;
    cvf::Vec3d                  m_originOnStartManipulation;

};



