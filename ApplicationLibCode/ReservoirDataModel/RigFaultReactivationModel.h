/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfPlane.h"
#include "cvfVector3.h"

#include <map>
#include <memory>
#include <vector>

namespace cvf
{
class TextureImage;
}

class RigFRModelPart
{
public:
    RigFRModelPart(){};
    ~RigFRModelPart(){};

    cvf::Vec3dArray             rect;
    cvf::ref<cvf::TextureImage> texture;
};

//==================================================================================================
///
///
//==================================================================================================
class RigFaultReactivationModel : public cvf::Object
{
public:
    enum class ModelParts
    {
        HiPart1 = 0,
        MidPart1,
        LowPart1,
        HiPart2,
        MidPart2,
        LowPart2
    };

public:
    RigFaultReactivationModel();
    ~RigFaultReactivationModel() override;

    std::vector<ModelParts> allParts() const;

    bool isValid() const;
    void reset();

    void setPlane( cvf::Vec3d anchorPoint, cvf::Vec3d normal );
    void setFaultPlaneIntersect( cvf::Vec3d faultPlaneTop, cvf::Vec3d faultPlaneBottom );
    void setMaxExtentFromAnchor( double maxExtentHorz, double minZ, double maxZ );

    void updateRects();

    void setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color );

    cvf::Vec3dArray             rect( ModelParts part ) const;
    cvf::ref<cvf::TextureImage> texture( ModelParts part ) const;

private:
    cvf::Vec3d m_planeNormal;
    cvf::Vec3d m_planeAnchor;

    cvf::Vec3d m_faultPlaneIntersectTop;
    cvf::Vec3d m_faultPlaneIntersectBottom;

    double m_maxHorzExtent;
    double m_minZ;
    double m_maxZ;

    std::map<ModelParts, RigFRModelPart> m_parts;
    bool                                 m_isValid;
};
