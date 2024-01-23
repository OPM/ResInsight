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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfScene.h"
#include "cvfOpenGLContext.h"


//==================================================================================================
//
// 
//
//==================================================================================================
class QTBSceneFactory
{
public:
    QTBSceneFactory(bool useShaders);

    cvf::ref<cvf::Scene>                createTestScene(const cvf::OpenGLCapabilities& capabilities) const;

private:
    cvf::ref<cvf::Part>                 createTexturedPart(const cvf::OpenGLCapabilities& capabilities) const;
    cvf::ref<cvf::Part>                 createDrawableTextPart(const cvf::OpenGLCapabilities& capabilities) const;
    static cvf::ref<cvf::DrawableGeo>   createQuadGeoWithTexCoords(const cvf::Vec3f& origin, const cvf::Vec3f& u, const cvf::Vec3f& v);
    static cvf::ref<cvf::TextureImage>  create4x4ColoredImage();

private:
    bool m_useShaders;
};

