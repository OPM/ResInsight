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

#include "cvfDrawable.h"
#include "cvfArray.h"
#include "cvfBoundingBox.h"
#include "cvfTextDrawer.h"

namespace cvf {

class Font;
class DrawableGeo;
class ShaderProgram;
class BufferObjectManaged;
class Camera;


//==================================================================================================
//
// DrawableText implements drawing of text strings attached to a 3D point
//
//==================================================================================================
class DrawableText : public Drawable
{
public:
    DrawableText();
    virtual ~DrawableText();

    void                        setFont(Font* font);
    ref<Font>                   font() const;

    void                        addText(const String& text, const Vec3f& position, const Vec3f& direction = Vec3f::X_AXIS);
    size_t                      numberOfTexts() const;
    void                        setVerticalAlignment(TextDrawer::Alignment alignment);
    void                        setTextColor(const Color3f& color);
    void                        setBackgroundColor(const Color3f& color);
    void                        setBorderColor(const Color3f& color);
    void                        setDrawBackground(bool drawBackground);
    void                        setDrawBorder(bool drawBorder);
    void                        setCheckPosVisible(bool checkpos);
    void                        setUseDepthBuffer(bool useDepthBuffer);

    virtual void                createUploadBufferObjectsGPU(OpenGLContext* /*oglContext*/) {}
    virtual void                releaseBufferObjectsGPU() {}

    virtual void                render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState);
    /*virtual*/ void            renderSoftware(OpenGLContext* oglContext, const MatrixState& matrixState);

    virtual size_t              vertexCount() const;
    virtual size_t              triangleCount() const;
    virtual size_t              faceCount() const;

    virtual BoundingBox         boundingBox() const;
    BoundingBox                 textBoundingBox(const String& text, const Vec3f& position, const Vec3f& direction = Vec3f::X_AXIS);

    virtual bool                rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const;
    bool                        rayIntersect(const Ray& ray, const Camera& camera, Vec3d* intersectionPoint);

    // TO BE REMOVED!
    virtual void                renderFixedFunction(OpenGLContext* oglContext, const MatrixState& matrixState) { renderSoftware(oglContext, matrixState); }
    virtual void                renderImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState) { renderSoftware(oglContext, matrixState); }

private:
    void renderText(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState);
    bool labelAnchorVisible(OpenGLContext* oglContext, const Vec3d winCoord, const Vec3f& worldCoord, bool softwareRendering) const;


private:
    std::vector<Vec3f>          m_positions;        // Coordinate of the lower left corner of where to place the text in pixel coordinates
    std::vector<String>         m_texts;            // Text strings to be drawn
    std::vector<Vec3f>          m_directions;       // Text direction

    ref<Font>                   m_font;             // Font used to draw text
    
    TextDrawer::Alignment       m_verticalAlignment;
    Color3f                     m_textColor;       
    Color3f                     m_backgroundColor;  
    Color3f                     m_borderColor;      

    bool                        m_drawBackground;
    bool                        m_drawBorder;
    bool                        m_checkPosVisible;
    bool                        m_useDepthBuffer;

    BoundingBox                 m_boundingBox;      //
};

}  // namespace cvf
