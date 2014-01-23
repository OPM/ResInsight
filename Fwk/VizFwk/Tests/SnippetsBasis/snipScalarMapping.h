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

#include "cvfuTestSnippet.h"
#include "cvfScalarMapperUniformLevels.h"
#include "cvfuProperty.h"

namespace snip {

using namespace cvf;
using namespace cvfu;


//==================================================================================================
//
// 
//
//==================================================================================================
class ScalarMapping : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Scalar Mapping");

private:
    class QuadMesh : public Object
    {
    public:
        QuadMesh(cvf::uint numPointsX, cvf::uint numPointsY);
        ref<DrawableGeo> generateSurface(const ScalarMapper* mapper, bool useVertexResults) const;
        ref<DrawableGeo> generateMesh() const;

    public:
        Vec3fArray  m_vertices;
        UIntArray   m_indices;
        DoubleArray m_elementResults;
        DoubleArray m_vertexResults;
    };

public:
    ScalarMapping();

    virtual bool                onInitialize();
    virtual void                onPropertyChanged(Property* property, PostEventAction* postEventAction);
    virtual void                onKeyPressEvent(KeyEvent* keyEvent);
    virtual std::vector<String> helpText() const;

private:
    void                setRangeMinMaxPropertiesFromResultRange(bool useFullRange);
    void                updateVisualization();
    static ref<Effect>  createFixedFunctionTextureResultEffect(const cvf::ScalarMapperUniformLevels& scalarMapper);
    static ref<Effect>  createShaderTextureResultEffect(const cvf::ScalarMapperUniformLevels& scalarMapper);
    static ref<Effect>  createColorResultEffect();
    static ref<Effect>  createMeshEffect();

private:
    ref<ModelBasicList>             m_model;
    ref<QuadMesh>                   m_quadMesh;
    ref<ScalarMapperUniformLevels>  m_mapper;
    ref<OverlayColorLegend>         m_legend;
    ref<OverlayAxisCross>           m_axisCross;

    ref<PropertyEnum>               m_propResMapping;          
    ref<PropertyEnum>               m_propFringesMode;          
    ref<PropertyDouble>             m_propRangeMin;          
    ref<PropertyDouble>             m_propRangeMax;          
    ref<PropertyEnum>               m_propLegendPalette;          
    ref<PropertyInt>                m_propLegendColorCount;          
    ref<PropertyBool>               m_propTextureSizeAuto;          
    ref<PropertyInt>                m_propTextureSize;          
    ref<PropertyBool>               m_propMeshLines;          
};

}

