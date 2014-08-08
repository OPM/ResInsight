/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"


//==================================================================================================
/// 
//==================================================================================================
class RigResultAccessor : public cvf::Object
{
public:
    virtual double cellScalar(size_t gridLocalCellIndex) const = 0;
    virtual double cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const = 0;
};

#if 0

//==================================================================================================
/// 
//==================================================================================================
class RigResultAccessor2d : public cvf::Object
{
public:
    virtual cvf::Vec2d cellScalar(size_t gridLocalCellIndex) const = 0;
    virtual cvf::Vec2d cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const = 0;

    virtual QString resultName() const = 0;
    
};

//==================================================================================================
/// 
//==================================================================================================
class RigTernaryResultAccessor : public Rig2DResultAccessor
{
public:
    /// Requires two of the arguments to be present
    void setTernaryResultAccessors(RigResultAccessObject* soil, RigResultAccessObject* sgas, RigResultAccessObject* swat);

    /// Returns [SOil, SWat] regardless of which one of the three is missing. if Soil or SWat is missing, it is calculated 
    /// based on the two others
    virtual cvf::Vec2d cellScalar(size_t gridLocalCellIndex) { };
    virtual cvf::Vec2d cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) { return cellScalar(size_t gridLocalCellIndex); };

    virtual QString resultName() const = 0;

};

class RivTernaryScalarMapper : public cvf::Object
{
public:
    RivTernaryScalarMapper(const cvf::Color3f& undefScalarColor, float opacityLevel) : m_undefScalarColor(undefScalarColor), m_opacityLevel(opacityLevel)
    {

    }

    /// Calculate texture coords into an image produced by updateTexture, from the scalarValue
     Vec2f               mapToTextureCoord(double soil, double swat, bool isTransparent) {}

    /// Update the supplied TextureImage to be addressable by the texture coords delivered by mapToTextureCoord
     bool                updateTexture(TextureImage* image){}

private:
    cvf::Color3f m_undefScalarColor; 
    float m_opacityLevel;
};


#endif
