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
#include "cvfuProperty.h"

namespace snip {

using namespace cvf;
using namespace cvfu;


//==================================================================================================
//
// 
//
//==================================================================================================
class DrawableAnimation : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Modifying Drawable Animation");

public:
    DrawableAnimation();

    virtual bool    onInitialize();
    virtual void    onUpdateAnimation(double animationTime, PostEventAction* postEventAction);
    virtual void    onPropertyChanged(Property* property, PostEventAction* postEventAction);

private:
    struct SrcData
    {
        ref<Vec3fArray>     vertices;
        ref<Vec3fArray>     normals;
        ref<UIntArray>      indices;
    };

private:
    static SrcData          extractSrcDataFromGeo(DrawableGeo* geo);
    static ref<DrawableGeo> createGeoFromSrcData(SrcData srcData, double geometryCompletionRatio);
    static void             updateGeo_ScopeOnly(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo);
    static void             updateGeo_RespecifyIndices(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo);
    static void             updateGeo_NewIndices(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo);

private:
    std::vector<SrcData>    m_srcData;
    ref<ModelBasicList>     m_model;
    double                  m_lastAnimUpdateTimeStamp;
    double                  m_geometryCompletionRatio;
    ref<PropertyEnum>       m_propUpdateType;     
};

}

