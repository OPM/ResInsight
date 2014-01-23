//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfCollection.h"
#include "cvfPart.h"
#include "cvfVector3.h"

namespace cvfu {


//==================================================================================================
//
// PartCompoundGenerator
//
//==================================================================================================
class PartCompoundGenerator : public cvf::Object
{
public:
    PartCompoundGenerator();

    void setExtent(cvf::Vec3f extent);
    void setOrigin(cvf::Vec3f origin);
    void setPartDistribution(cvf::Vec3i partDistribution);
    void setUseShaders(bool useShaders); 
    void setNumEffects(int numEffects); 
    void setNumDrawableGeos(int numDrawableGeos); 
    void useRandomEffectAssignment(bool use);

    void generateBoxes(cvf::Collection<cvf::Part>* parts);
    void generateSpheres(cvf::uint numSlices, cvf::uint numStacks, cvf::Collection<cvf::Part>* parts);
    void generateTriangles(cvf::Collection<cvf::Part>* parts);

    int  numParts() const;

private:
    enum GenPrimType
    {
        BOX,
        SPHERE,
        TRIANGLE
    };

    cvf::Vec3i  m_partDistribution;
    cvf::Vec3f  m_extent;
    cvf::Vec3f  m_origin;
    bool        m_useShaders;
    int         m_numEffects;
    int         m_numDrawableGeos;
    bool        m_randomEffectAssignment;
    bool        m_randomGeoAssignment;

    void generateParts(GenPrimType primType, cvf::uint numSlices, cvf::uint numStacks, cvf::Collection<cvf::Part>* parts);
};

}
