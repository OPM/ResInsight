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

namespace snip {

using namespace cvf;
using namespace cvfu;


//==================================================================================================
//
// 
//
//==================================================================================================
class DepthPeelingFront : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Depth Peeling Front");

public:
    DepthPeelingFront();
    ~DepthPeelingFront();

    virtual bool    onInitialize();
    virtual void    onPaintEvent(PostEventAction* postEventAction);
    virtual void    onResizeEvent(int width, int height);
    virtual void    onKeyPressEvent(KeyEvent* keyEvent);
    
    virtual std::vector<cvf::String> helpText() const;

private:
    void setupShaders();
    void initRenderTargets(int width, int height);

    void bindTextureRECT(ShaderProgram* shaderProgram, const char* texname, cvf::uint texid, int texunit);

    void renderFrontToBackPeeling();

    void drawModel(ModelBasicList* model, ShaderProgram* shaderProg);
    void drawQuad();

private:
    ref<ModelBasicList> m_transparentModel;
    ref<ModelBasicList> m_solidModel;

    int m_numPasses;
    float m_opacity;

    cvf::uint m_frontFboId[2];
    cvf::uint m_frontDepthTexId[2];
    cvf::uint m_frontColorTexId[2];

    cvf::uint m_solidModelFboId;
    cvf::uint m_solidModelDepthTexId;
    cvf::uint m_solidModelColorTexId;

    cvf::uint m_frontColorBlenderFboId;
    cvf::uint m_frontColorBlenderTexId;

    ref<ShaderProgram> m_progInit;
    ref<ShaderProgram> m_progPeel;
    ref<ShaderProgram> m_progBlend;
    ref<ShaderProgram> m_progFinal;
    ref<ShaderProgram> m_progOpaque;
};

}

