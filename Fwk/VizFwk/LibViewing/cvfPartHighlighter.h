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

#include "cvfObject.h"
#include "cvfColor3.h"

namespace cvf {

class FramebufferObject;
class RenderSequence;
class Rendering;
class Model;
class Camera;
class GaussianBlur;
class UniformFloat;
class ClipPlaneSet;



//==================================================================================================
//
// 
//
//==================================================================================================
class PartHighlighter : public Object
{
public:
    PartHighlighter(Model* highlightModel, Camera* mainCamera);

    void    setHighlightColor(const Color3f& color);

    void    addRenderingsToSequence(RenderSequence* renderSequence);
    void    removeRenderingsFromSequence(RenderSequence* renderSequence);
    void    resize(int x, int y, uint width, uint height);
    void    prepareForRedraw();

    void    setClipPlaneSet(ClipPlaneSet* clipPlaneSet);

private:
    void    configureOverrideEffects(bool useClipping);

private:
    ref<Model>              m_highlightModel;       // Model containing the parts that should be highlighted
    ref<Camera>             m_highlightCamera;      // Camera to use in the highlight rendering, need a separate camera as long as clear color is part of camera/viewport
    cref<Camera>            m_mainCamera;           // Reference to main camera so we can copy viewpoint to the selected camera
    Color3f                 m_highlightColor;
    
    ref<FramebufferObject>  m_drawFbo;              // FBO used in the first draw pass
    ref<Rendering>          m_drawRendering;        // Rendering to render the highlight model footprint as basis for the blur pass
    ref<Rendering>          m_drawRenderingLines;   // Optional rendering to render the highlight model using line polygon mode (used to grow the footprint a bit)
    ref<Rendering>          m_mixRendering;         // Rendering used in final mix pass
    ref<UniformFloat>       m_highlightClrUniform;  // Uniform that controls the highlight color used in the mix rendering
    ref<GaussianBlur>       m_blur;                 // Blur helper
};



//==================================================================================================
//
// 
//
//==================================================================================================
class PartHighlighterStencil : public Object
{
public:
    PartHighlighterStencil(Model* highlightModel, Camera* mainCamera);

    void    setHighlightColor(const Color3f& color);

    void    addRenderingsToSequence(RenderSequence* renderSequence);
    void    removeRenderingsFromSequence(RenderSequence* renderSequence);
    void    resize(uint width, uint height);
    void    prepareForRedraw();

private:
    void    applyHighlightColor();

private:
    ref<Model>              m_highlightModel;       // Model containing the parts that should be highlighted
    ref<Camera>             m_highlightCamera;      // Camera to use in the highlight rendering, need a separate camera as long as clear color is part of camera/viewport
    cref<Camera>            m_mainCamera;           // Reference to main camera so we can copy viewpoint to the selected camera
    Color3f                 m_highlightColor;
    
    ref<FramebufferObject>  m_drawFbo;              // FBO used in the first draw pass
    ref<FramebufferObject>  m_blurFbo;              // FBO for blur pass
    ref<Rendering>          m_drawRendering;
    ref<Rendering>          m_blurRendering;
    ref<Rendering>          m_mixRendering;
};


}
