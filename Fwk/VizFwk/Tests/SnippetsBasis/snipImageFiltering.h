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
class GaussianBlurExperiment : public Object
{
public:
    GaussianBlurExperiment(Texture* textureToBlur);

    void            addRenderingsToSequence(RenderSequence* renderSequence);
    void            resizeFromTextureSize();

private:
    void            createRenderings();

private:
    ref<Texture>            m_textureToBlur;        // The texture that is to be blurred, passed in constructor. 
    ref<Texture>            m_tempColorTexture;     // The intermediate texture for communicating between passes
    ref<RenderbufferObject> m_depthRbo;             // Shared RBO for depth. We don't really need depth so maybe we can get rid of this one
    ref<Rendering>          m_vertBlurRendering;
    ref<Rendering>          m_horBlurRendering;
    ref<UniformFloat>       m_vertBlurSize;
    ref<UniformFloat>       m_horBlurSize;
};



//==================================================================================================
//
// 
//
//==================================================================================================
class ImageFiltering : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("ImageFiltering (experimental)");

public:
    virtual bool    onInitialize();
    virtual void    onPaintEvent(PostEventAction* postEventAction);
    virtual void    onResizeEvent(int width, int height);
    virtual void    onPropertyChanged(Property* property, PostEventAction* postEventAction);

private:
    ref<ModelBasicList>         m_mainModel;
    ref<FramebufferObject>      m_mainFbo;
    ref<Texture>                m_mainColorTexture;
    ref<Rendering>              m_mainRendering;
    ref<Rendering>              m_finalRendering;

    ref<GaussianBlur>           m_blur;
    //ref<GaussianBlurExperiment> m_blur;

    ref<PropertyInt>            m_propGaussKernelSize;
    ref<PropertyDouble>         m_propGaussSigma;
};

}

