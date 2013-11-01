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
#include "cvfModelBasicList.h"

namespace snip {

using namespace cvf;
using namespace cvfu;


//==================================================================================================
//
// 
//
//==================================================================================================
class PartHighlighterExperiment : public cvf::Object
{
public:
    PartHighlighterExperiment(ModelBasicList* selectedModel, Camera* mainCamera);

    void addRenderingsToSequence(cvf::RenderSequence* renderSequence);
    void resize(cvf::uint width, cvf::uint height);
    void prepareForRedraw();

private:
    ref<ModelBasicList>     m_selectedModel;
    cref<Camera>            m_mainCamera;       // Reference to main camera so we can copy viewpoint to the selected camera
    ref<Camera>             m_selectedCamera;

    ref<FramebufferObject>  m_selectedFbo;
    ref<FramebufferObject>  m_blurFbo;
    ref<Rendering>          m_selectedRendering;
    ref<Rendering>          m_blurRendering;
    ref<Rendering>          m_mixRendering;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class Highlight : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Highlight");

public:
    virtual bool    onInitialize();
    virtual void    onPaintEvent(PostEventAction* postEventAction);
    virtual void    onResizeEvent(int width, int height);
    virtual void    onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent);

private:
    ref<ModelBasicList>                 m_mainModel;
    ref<ModelBasicList>                 m_selectedModel;
    ref<Rendering>                      m_mainRendering;
    ref<PartHighlighter>                m_highlighter;
    //ref<PartHighlighterStencil>         m_highlighter;
    //ref<PartHighlighterExperiment>      m_highlighter;
};

}

