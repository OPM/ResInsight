//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfOverlayItem.h"
#include "cvfMatrix4.h"
#include "cvfColor3.h"
#include "cvfString.h"

namespace cvf {

class Camera;
class DrawableGeo;
class DrawableVectors;
class Font;
class ShaderProgram;
class MatrixState;


//==================================================================================================
//
// Overlay axis cross 
//
//==================================================================================================
class OverlayAxisCross : public OverlayItem
{
public:
    OverlayAxisCross(Camera* camera, Font* font);
    ~OverlayAxisCross();

    virtual Vec2ui  sizeHint();
    virtual Vec2ui  maximumSize();
    virtual Vec2ui  minimumSize();

    virtual void    render(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size);
    virtual void    renderSoftware(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size);

    void            setSize(const Vec2ui& size);

    void            setAxisLabels(const String& xLabel, const String& yLabel, const String& zlabel);
    void            setAxisLabelsColor(const Color3f& color);

private:
    void render(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size, bool software, const Mat4d& viewMatrix);
    void createAxisGeometry(bool software);
    void renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState);
    void renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState);
    void renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState);

private:
    ref<Camera>         m_camera;       // This camera's view matrix will be used to orient the axis cross
    String              m_xLabel;       // Label to display on x axis, default 'x'
    String              m_yLabel;       
    String              m_zLabel;
    Color3f             m_textColor;    // Text color 
    ref<Font>           m_font;

    Vec2ui              m_size;         // Pixel size of the axis area

    ref<DrawableVectors>    m_axis;
    ref<DrawableGeo>        m_xAxisTriangle;
    ref<DrawableGeo>        m_yAxisTriangle;
};

}
