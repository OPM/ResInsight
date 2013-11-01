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
class DepthPeeling: public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Depth Peeling Dual");

public:
    DepthPeeling();

    virtual bool    onInitialize();
    virtual void    onResizeEvent(int width, int height);
    virtual void    onKeyPressEvent(KeyEvent* keyEvent);
    
    virtual std::vector<cvf::String> helpText() const;

private:
    void setupShaders();
    void setupRenderings(cvf::uint numPasses);
    void resizeViewportsAndBuffers(cvf::uint width, cvf::uint height);

private:
    ref<ModelBasicList> m_transparentModel;
    ref<ModelBasicList> m_solidModel;

    int m_numPasses;
    float m_opacity;

    ref<ShaderProgram> m_progInit;
    ref<ShaderProgram> m_progPeel;
    String m_fragShaderBlendCode;
    String m_fragShaderFinalCode;

    ref<FramebufferObject> m_fboSolid;
    ref<FramebufferObject> m_fboInitBlend;
    ref<FramebufferObject> m_fboClearDepth[2];
    ref<FramebufferObject> m_fboClearBlendTextures[2];
    ref<FramebufferObject> m_fboDepthPeel[2];
    ref<FramebufferObject> m_fboFullScreenUpdate[2];
};

}
