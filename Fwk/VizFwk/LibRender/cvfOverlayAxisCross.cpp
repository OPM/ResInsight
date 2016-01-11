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
#include "cvfOverlayAxisCross.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfViewport.h"
#include "cvfCamera.h"
#include "cvfTextDrawer.h"
#include "cvfFont.h"
#include "cvfShaderProgram.h"
#include "cvfUniform.h"
#include "cvfMatrixState.h"
#include "cvfDrawableVectors.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfArrowGenerator.h"
#include "cvfBufferObjectManaged.h"
#include "cvfDrawableText.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif


namespace cvf {


//==================================================================================================
///
/// \class cvf::OverlayAxisCross
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
OverlayAxisCross::OverlayAxisCross(Camera* camera, Font* font)
:   m_camera(camera),
    m_xLabel("x"),
    m_yLabel("y"),
    m_zLabel("z"),
    m_textColor(Color3::BLACK),
    m_font(font),
    m_size(120, 120)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayAxisCross::~OverlayAxisCross()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::setAxisLabels( const String& xLabel, const String& yLabel, const String& zLabel )
{
    // Clipping of axis label text is depends on m_size and
    // z-part of axisMatrix.setTranslation(Vec3d(0, 0, -4.4)) defined in OverlayAxisCross::render()

    CVF_ASSERT (xLabel.size() < 7 && yLabel.size() < 7 && zLabel.size() < 7);

    m_xLabel = xLabel;
    m_yLabel = yLabel;
    m_zLabel = zLabel;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::setAxisLabelsColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayAxisCross::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::setSize(const Vec2ui& size)
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    Mat4d viewMatrix = m_camera->viewMatrix();
    render(oglContext, position, size, false, viewMatrix);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering 
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    Mat4d viewMatrix = m_camera->viewMatrix();
    render(oglContext, position, size, true, viewMatrix);
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software, const Mat4d& viewMatrix)
{
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    if (m_axis.isNull()) 
    {
        createAxisGeometry(software);
    }

    // Position the camera far enough away to make the axis and the text fit within the viewport
    Mat4d axisMatrix = viewMatrix;
    axisMatrix.setTranslation(Vec3d(-0.4, 0, -4.4));

    // Setup camera
    Camera cam;
    cam.setProjectionAsPerspective(40.0, 0.05, 100.0);
    cam.setViewMatrix(axisMatrix);
    cam.setViewport(position.x(), position.y(), size.x(), size.y());

    // Setup viewport
    cam.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);
    cam.applyOpenGL();


    // Do the actual rendering
    // -----------------------------------------------
    MatrixState matrixState(cam);
    if (software)
    {
        renderAxisImmediateMode(oglContext, matrixState);
    }
    else
    {
        renderAxis(oglContext, matrixState);
    }

    renderAxisLabels(oglContext, software, matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    CVF_ASSERT(m_axis.notNull());
    CVF_ASSERT(m_xAxisTriangle.notNull());
    CVF_ASSERT(m_yAxisTriangle.notNull());

    OpenGLResourceManager* resourceManager = oglContext->resourceManager();
    ref<ShaderProgram> vectorProgram = resourceManager->getLinkedVectorDrawerShaderProgram(oglContext);

    if (vectorProgram->useProgram(oglContext))
    {
        vectorProgram->clearUniformApplyTracking();
        vectorProgram->applyFixedUniforms(oglContext, matrixState);
    }

    // Draw X, Y and Z vectors
    m_axis->render(oglContext, vectorProgram.p(), matrixState);

    // Draw X axis triangle
    ref<ShaderProgram> triangleProgram = resourceManager->getLinkedUnlitColorShaderProgram(oglContext);

    if (triangleProgram->useProgram(oglContext))
    {
        triangleProgram->clearUniformApplyTracking();
        triangleProgram->applyFixedUniforms(oglContext, matrixState);
    }

    UniformFloat xUniformColor("u_color", Color4f(Color3::RED));
    triangleProgram->applyUniform(oglContext, xUniformColor);
    m_xAxisTriangle->render(oglContext, triangleProgram.p(), matrixState);

    // Draw Y axis triangle
    UniformFloat yUniformColor("u_color", Color4f(Color3::GREEN));
    triangleProgram->applyUniform(oglContext, yUniformColor);
    m_yAxisTriangle->render(oglContext, triangleProgram.p(), matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    m_axis->renderImmediateMode(oglContext, matrixState);  

    // Draw X axis triangle
    RenderStateMaterial_FF xMaterial(RenderStateMaterial_FF::PURE_RED);
    xMaterial.applyOpenGL(oglContext);
    m_xAxisTriangle->renderImmediateMode(oglContext, matrixState);

    // Draw Y axis triangle
    RenderStateMaterial_FF yMaterial(RenderStateMaterial_FF::PURE_GREEN);
    yMaterial.applyOpenGL(oglContext);
    m_yAxisTriangle->renderImmediateMode(oglContext, matrixState);
#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// Create the geometry used to draw the axis (vector arrows) and the two triangles
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::createAxisGeometry(bool software)
{
    CVF_ASSERT(m_axis.isNull());

    // Axis colors
    ref<Color3fArray> colorArray = new Color3fArray;
    colorArray->resize(3);
    colorArray->set(0, Color3::RED);                // X axis
    colorArray->set(1, Color3::GREEN);              // Y axis
    colorArray->set(2, Color3::BLUE);               // Z axis

    // Positions of the vectors - All in origo
    ref<cvf::Vec3fArray> vertexArray = new Vec3fArray;
    vertexArray->resize(3);
    vertexArray->set(0, Vec3f(0.0f, 0.0f, 0.0f));    // X axis
    vertexArray->set(1, Vec3f(0.0f, 0.0f, 0.0f));    // Y axis
    vertexArray->set(2, Vec3f(0.0f, 0.0f, 0.0f));    // Z axis

    // Direction & magnitude of the vectors
    ref<cvf::Vec3fArray> vectorArray = new Vec3fArray;
    vectorArray->resize(3);
    vectorArray->set(0, Vec3f::X_AXIS);             // X axis
    vectorArray->set(1, Vec3f::Y_AXIS);             // Y axis
    vectorArray->set(2, Vec3f::Z_AXIS);             // Z axis

    // Create the arrow glyph for the vector drawer
    GeometryBuilderTriangles arrowBuilder;
    ArrowGenerator gen;
    gen.setShaftRelativeRadius(0.045f);
    gen.setHeadRelativeRadius(0.12f);
    gen.setHeadRelativeLength(0.2f);
    gen.setNumSlices(30);
    gen.generate(&arrowBuilder);

    if (software)
    {
        m_axis = new DrawableVectors();
    }
    else
    {
        m_axis = new DrawableVectors("u_transformationMatrix", "u_color");
    }

    m_axis->setVectors(vertexArray.p(), vectorArray.p());
    m_axis->setColors(colorArray.p());
    m_axis->setGlyph(arrowBuilder.trianglesUShort().p(), arrowBuilder.vertices().p());

    // X axis triangle
    GeometryBuilderDrawableGeo xAxisTriBuilder;
    xAxisTriBuilder.addTriangle(0, 1, 2);
    Vec3fArray xAxisTriVerts;
    xAxisTriVerts.resize(3);
    xAxisTriVerts[0] = Vec3f(0, 0, 0);
    xAxisTriVerts[1] = Vec3f(0.5f, 0, 0);
    xAxisTriVerts[2] = Vec3f(0.2f, 0.2f, 0);
    xAxisTriBuilder.addVertices(xAxisTriVerts);
    m_xAxisTriangle = xAxisTriBuilder.drawableGeo();

    // Y axis triangle
    GeometryBuilderDrawableGeo yAxisTriBuilder;
    yAxisTriBuilder.addTriangle(0, 1, 2);
    Vec3fArray yAxisTriVerts;
    yAxisTriVerts.resize(3);
    yAxisTriVerts[0] = Vec3f(0, 0, 0);
    yAxisTriVerts[1] = Vec3f(0.2f, 0.2f, 0);
    yAxisTriVerts[2] = Vec3f(0, 0.4f, 0);
    yAxisTriBuilder.addVertices(yAxisTriVerts);
    m_yAxisTriangle = yAxisTriBuilder.drawableGeo();
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayAxisCross::renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState)
{
    // Multiply with 1.08 will slightly pull the labels away from the corresponding arrow head
    Vec3f xPos(1.08f, 0, 0);
    Vec3f yPos(0, 1.08f, 0);
    Vec3f zPos(0, 0, 1.08f);

    DrawableText drawableText;
    drawableText.setFont(m_font.p());
    drawableText.setCheckPosVisible(false);
    drawableText.setDrawBorder(false);
    drawableText.setDrawBackground(false);
    drawableText.setVerticalAlignment(TextDrawer::CENTER);
    drawableText.setTextColor(m_textColor);

    if (!m_xLabel.isEmpty()) drawableText.addText(m_xLabel, xPos);
    if (!m_yLabel.isEmpty()) drawableText.addText(m_yLabel, yPos);
    if (!m_zLabel.isEmpty()) drawableText.addText(m_zLabel, zPos);


    // Do the actual rendering
    // -----------------------------------------------
    if (software)
    {
        drawableText.renderSoftware(oglContext, matrixState);
    }
    else
    {
        ref<ShaderProgram> textShader = oglContext->resourceManager()->getLinkedTextShaderProgram(oglContext);
        drawableText.render(oglContext, textShader.p(), matrixState);
    }
}

} // namespace cvf

