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

namespace cvf {

class Texture;
class RenderbufferObject;
class RenderSequence;
class Rendering;
class UniformFloat;



//==================================================================================================
//
// 
//
//==================================================================================================
class GaussianBlur : public Object
{
public:
public:
    GaussianBlur(Texture* textureToBlur, int kernelSize, double sigma);

    void            setSigma(double sigma);

    void            addRenderingsToSequence(RenderSequence* renderSequence);
    void            removeRenderingsFromSequence(RenderSequence* renderSequence);
    void            resizeFromTextureSize();

private:
    void            createRenderings();

private:
    ref<Texture>            m_textureToBlur;        // The texture that is to be blurred, passed in constructor. 
    int                     m_kernelSize;           // Kernel size
    double                  m_sigma;                // Standard deviation for the Gaussian distribution

    ref<Texture>            m_tempColorTexture;     // The intermediate texture for communicating between passes
    ref<Rendering>          m_vertBlurRendering;    // Rendering for the vertical pass
    ref<Rendering>          m_horBlurRendering;     // Rendering for the horizontal pass
    ref<UniformFloat>       m_uniformSigma;         // 
    ref<UniformFloat>       m_uniformVertBlurSize;
    ref<UniformFloat>       m_uniformHorBlurSize;
};


}
