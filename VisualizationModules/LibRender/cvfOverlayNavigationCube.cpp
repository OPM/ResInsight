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

#include "cvfBase.h"
#include "cvfOverlayNavigationCube.h"
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
#include "cvfTextureImage.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif


namespace cvf {


//==================================================================================================
///
/// \class cvf::OverlayNavigationCube
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::OverlayNavigationCube(Camera* camera, Font* font)
:   m_camera(camera),
    m_xLabel("ax"),
    m_yLabel("by"),
    m_zLabel("cz"),
    m_textColor(Color3::BLACK),
    m_font(font),
    m_size(120, 120)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::~OverlayNavigationCube()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setAxisLabels( const String& xLabel, const String& yLabel, const String& zLabel )
{
    // Clipping of axis label text is depends on m_size and
    // z-part of axisMatrix.setTranslation(Vec3d(0, 0, -4.4)) defined in OverlayNavigationCube::render()
    CVF_ASSERT (xLabel.size() < 5 && yLabel.size() < 5 && zLabel.size() < 5);

    m_xLabel = xLabel;
    m_yLabel = yLabel;
    m_zLabel = zLabel;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setAxisLabelsColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayNavigationCube::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayNavigationCube::maximumSize()
{
    return sizeHint();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayNavigationCube::minimumSize()
{
    return sizeHint();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setSize(const Vec2ui& size)
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    Mat4d viewMatrix = m_camera->viewMatrix();
    render(oglContext, position, size, false, viewMatrix);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    Mat4d viewMatrix = m_camera->viewMatrix();
    render(oglContext, position, size, true, viewMatrix);
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software, const Mat4d& viewMatrix)
{
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    if (m_axis.isNull()) 
    {
        createAxisGeometry(software);
    }

    if (m_cubeGeos.size() == 0)
    {
        createCubeGeos();

        // Create the shader for the cube geometry
        ShaderProgramGenerator gen("CubeGeoShader", ShaderSourceProvider::instance());
        gen.configureStandardHeadlightColor();
        m_cubeGeoShader = gen.generate();
        m_cubeGeoShader->linkProgram(oglContext);
    }

    // Position the camera far enough away to make the axis and the text fit within the viewport
    Mat4d axisMatrix = viewMatrix;
    axisMatrix.setTranslation(Vec3d(0, 0, -2.0));

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

    renderCubeGeos(oglContext, software, matrixState);

    renderAxisLabels(oglContext, software, matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    CVF_ASSERT(m_axis.notNull());

    OpenGLResourceManager* resourceManager = oglContext->resourceManager();
    ref<ShaderProgram> vectorProgram = resourceManager->getLinkedVectorDrawerShaderProgram(oglContext);

    if (vectorProgram->useProgram(oglContext))
    {
        vectorProgram->clearUniformApplyTracking();
        vectorProgram->applyFixedUniforms(oglContext, matrixState);
    }

    // Draw X, Y and Z vectors
    m_axis->render(oglContext, vectorProgram.p(), matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    m_axis->renderImmediateMode(oglContext, matrixState);  
#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// Create the geometry used to draw the axis (vector arrows) and the two triangles
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createAxisGeometry(bool software)
{
    CVF_ASSERT(m_axis.isNull());

    // Axis colors
    ref<Color3fArray> colorArray = new Color3fArray;
    colorArray->resize(3);
    colorArray->set(0, Color3::RED);                // X axis
    colorArray->set(1, Color3::GREEN);              // Y axis
    colorArray->set(2, Color3::BLUE);               // Z axis

    // Positions of the vectors - All in origo
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    ref<cvf::Vec3fArray> vertexArray = new Vec3fArray;
    vertexArray->resize(3);
    vertexArray->set(0, cp[0]);    // X axis
    vertexArray->set(1, cp[0]);    // Y axis
    vertexArray->set(2, cp[0]);    // Z axis

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
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderCubeGeos(OpenGLContext* oglContext, bool software, const MatrixState& matrixState)
{
    CVF_UNUSED(software);

    for (size_t i  = 0; i < m_cubeGeos.size(); ++i)
    {
        m_cubeGeos[i]->render(oglContext, m_cubeGeoShader.p(), matrixState);
    }
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState)
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


//--------------------------------------------------------------------------------------------------
///											  Face (with local indices):
///				   7---------6                 4                    3  
///				  /|	    /|	   |z   		|---|----------|---|
///				 / |	   / |	   |  / y  	    |TL |   TOP    |TR |
///				4---------5  |     | /   		|---|----------|---|
///				|  3------|--2	   *---x		|   |          |   |
///				| /		  | /	       			| L |  CENTER  | R |
///				|/        |/	      			|   |          |   |
///				0---------1						|---|----------|---|
///											    |BL |  BOTTOM  |BR |
///												|---|----------|---|
///											   1                    2
///
///	Items:
///		Faces:
///		+X : VT_NCI_X_POS : RIGHT	: 0 2 6 5 
///		-X : VT_NCI_X_NEG : LEFT	: 3 0 4 7
///		+Y : VT_NCI_Y_POS : BACK	: 2 3 7 6
///		-Y : VT_NCI_Y_NEG : FRONT	: 0 1 5 4
///		+Z : VT_NCI_Z_POS : TOP		: 4 5 6 7
///		-Z : VT_NCI_Z_NEG : BOTTOM	: 3 2 1 0
///
///		Corners: 
///		0 : VT_NCI_CORNER_XN_YN_ZN
///		1 : VT_NCI_CORNER_XP_YN_ZN
///		2 : VT_NCI_CORNER_XP_YP_ZN
///		3 : VT_NCI_CORNER_XN_YP_ZN
///		4 : VT_NCI_CORNER_XN_YN_ZP
///		5 : VT_NCI_CORNER_XP_YN_ZP
///		6 : VT_NCI_CORNER_XP_YP_ZP
///		7 : VT_NCI_CORNER_XN_YP_ZP
///
///		Edges:
///		01: VT_NCI_EDGE_YN_ZN
///		12: VT_NCI_EDGE_XP_ZN
///		23: VT_NCI_EDGE_YP_ZN
///		03: VT_NCI_EDGE_XN_ZN
///		45: VT_NCI_EDGE_YN_ZP
///		56: VT_NCI_EDGE_XP_ZP
///		67: VT_NCI_EDGE_YP_ZP
///		47: VT_NCI_EDGE_XN_ZP
///		04: VT_NCI_EDGE_XN_YN
///		15: VT_NCI_EDGE_XP_YN
///		26: VT_NCI_EDGE_XP_YP
///		37: VT_NCI_EDGE_XN_YP
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createCubeGeos()
{
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    m_cubeGeos.clear();

    createCubeFaceGeos(NCF_Y_NEG, cp[0], cp[1], cp[5], cp[4]);//, m_yNegAxisName, m_yFaceColor, m_textureNegYAxis.p());		// Front
    createCubeFaceGeos(NCF_Y_POS, cp[2], cp[3], cp[7], cp[6]);//, m_yPosAxisName, m_yFaceColor, m_texturePosYAxis.p());		// Back
                                                                                                             
    createCubeFaceGeos(NCF_Z_POS, cp[4], cp[5], cp[6], cp[7]);//, m_zPosAxisName, m_zFaceColor, m_texturePosZAxis.p());		// Top
    createCubeFaceGeos(NCF_Z_NEG, cp[3], cp[2], cp[1], cp[0]);//, m_zNegAxisName, m_zFaceColor, m_textureNegZAxis.p());		// Bottom
                                                            
    createCubeFaceGeos(NCF_X_NEG, cp[3], cp[0], cp[4], cp[7]);//, m_xNegAxisName, m_xFaceColor, m_textureNegXAxis.p());		// left
    createCubeFaceGeos(NCF_X_POS, cp[1], cp[2], cp[6], cp[5]);//, m_xPosAxisName, m_xFaceColor, m_texturePosXAxis.p());		// Right
}


//--------------------------------------------------------------------------------------------------
///											  Face (with local indices):
///			               4                    3  
///			   |z   		|---|----------|---|
///			   |  / y  	    |TL |   TOP    |TR |
///			   | /   		|---|----------|---|
///			   *---x		|   |          |   |
///			       			| L |  CENTER  | R |
///			      			|   |          |   |
///							|---|----------|---|
///						    |BL |  BOTTOM  |BR |
///			    			|---|----------|---|
///                        1                    2
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createCubeFaceGeos(NavCubeFace face, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f p4)//, const String& name, const Color3f& baseColor, TextureImage* texture)
{
    // Get the orientation vectors for the face
//     Vec3f vNormal, vUp, vRight;
//     faceOrientation(face, &vNormal, &vUp, &vRight);

    float fCornerFactor = 0.175f;
    Vec3f p12 = p1 + (p2 - p1)*fCornerFactor;
    Vec3f p14 = p1 + (p4 - p1)*fCornerFactor;
    Vec3f pi1 = p1 + (p12 - p1) + (p14 - p1);

    Vec3f p21 = p2 + (p1 - p2)*fCornerFactor;
    Vec3f p23 = p2 + (p3 - p2)*fCornerFactor;
    Vec3f pi2 = p2 + (p21 - p2) + (p23 - p2);

    Vec3f p32 = p3 + (p2 - p3)*fCornerFactor;
    Vec3f p34 = p3 + (p4 - p3)*fCornerFactor;
    Vec3f pi3 = p3 + (p32 - p3) + (p34 - p3);

    Vec3f p41 = p4 + (p1 - p4)*fCornerFactor;
    Vec3f p43 = p4 + (p3 - p4)*fCornerFactor;
    Vec3f pi4 = p4 + (p41 - p4) + (p43 - p4);

    // Bottom left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_LEFT));
    m_cubeGeos.push_back(createQuadGeo(p1, p12, pi1, p14).p());

    // Bottom right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_RIGHT));
    m_cubeGeos.push_back(createQuadGeo(p2, p23, pi2, p21).p());

    // Top right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_RIGHT));
    m_cubeGeos.push_back(createQuadGeo(p3, p34, pi3, p32).p());

    // Top left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_LEFT));
    m_cubeGeos.push_back(createQuadGeo(p4, p41, pi4, p43).p());

    // Bottom
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM));
    m_cubeGeos.push_back(createQuadGeo(p12, p21, pi2, pi1).p());

    // Top
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP));
    m_cubeGeos.push_back(createQuadGeo(p34, p43, pi4, pi3).p());

    // Right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_RIGHT));
    m_cubeGeos.push_back(createQuadGeo(p23, p32, pi3, pi2).p());

    // Left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_LEFT));
    m_cubeGeos.push_back(createQuadGeo(p41, p14, pi1, pi4).p());

    // Inner part
    m_cubeItemType.push_back(navCubeItem(face, NCFI_CENTER));
    m_cubeGeos.push_back(createQuadGeo(pi1, pi2, pi3, pi4).p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> OverlayNavigationCube::createQuadGeo(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& v4)
{
    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertexArray = new Vec3fArray(4);
    vertexArray->set(0, v1);
    vertexArray->set(1, v2);
    vertexArray->set(2, v3);
    vertexArray->set(3, v4);

    geo->setVertexArray(vertexArray.p());

    ref<cvf::UShortArray> indices = new cvf::UShortArray(6);
    indices->set(0, 0);
    indices->set(1, 1);
    indices->set(2, 2);
    indices->set(3, 0);
    indices->set(4, 2);
    indices->set(5, 3);

    ref<cvf::PrimitiveSetIndexedUShort> primSet = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
///   		   7---------6                
///   		  /|	    /|	   |z   	
///   		 / |	   / |	   |  / y  	
///   		4---------5  |     | /   	
///   		|  3------|--2	   *---x	
///   		| /		  | /	       	
///   		|/        |/	      	
///   		0---------1             		
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::navCubeCornerPoints(Vec3f points[8])
{
    float fBoxLength = 0.65f;

    Vec3f min(-fBoxLength/2.0f, -fBoxLength/2.0f, -fBoxLength/2.0f);
    Vec3f max(fBoxLength/2.0f, fBoxLength/2.0f, fBoxLength/2.0f);

    points[0].set(min.x(), min.y(), min.z());
    points[1].set(max.x(), min.y(), min.z());
    points[2].set(max.x(), max.y(), min.z());
    points[3].set(min.x(), max.y(), min.z());
    points[4].set(min.x(), min.y(), max.z());
    points[5].set(max.x(), min.y(), max.z());
    points[6].set(max.x(), max.y(), max.z());
    points[7].set(min.x(), max.y(), max.z());
}


/*************************************************************************************************
 *//** 
 * Convert face + faceItem to VTNavCubeItem
 *
 *************************************************************************************************/
OverlayNavigationCube::NavCubeItem OverlayNavigationCube::navCubeItem(NavCubeFace face, NavCubeFaceItem faceItem) const
{
	NavCubeItem item = NCI_NONE;

	switch(face)
	{
		case NCF_X_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_X_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_XP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_XP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XP_YN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_YP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_X_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_X_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_XN_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_XN_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_YP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XN_YN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Y_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Y_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_YP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XP_YP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XN_YP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Y_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Y_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_YN_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YN_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_YN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_YN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Z_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Z_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_YP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YN_ZP; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_ZP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_ZP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Z_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Z_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_YN_ZN; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_ZN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_ZN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}

        case NCF_NONE:
        {
            CVF_FAIL_MSG("Illegal nav cube face specified");
            break;
        }
	}

	return item;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::faceOrientation(NavCubeFace face, Vec3f* normal, Vec3f* upVector, Vec3f* rightVector) const
{
	CVF_ASSERT(normal && upVector && rightVector);

	switch (face)
	{
        case NCF_X_POS: *normal = Vec3f::X_AXIS; break;
		case NCF_X_NEG: *normal = -Vec3f::X_AXIS; break;
		case NCF_Y_POS: *normal = Vec3f::Y_AXIS; break;
		case NCF_Y_NEG: *normal = -Vec3f::Y_AXIS; break;
		case NCF_Z_POS: *normal = Vec3f::Z_AXIS; break;
		case NCF_Z_NEG: *normal = -Vec3f::Z_AXIS; break;
		case NCF_NONE:	CVF_FAIL_MSG("Illegal nav cube face"); break;
	}

	if ((*normal)*m_upVector == 0.0)
	{
		if (*normal == m_upVector)	    *upVector = -m_frontVector;
		else							*upVector = m_frontVector;
	}
	else
	{
		*upVector = m_upVector;
	}

	*rightVector = *upVector^*normal;

	normal->normalize();
	upVector->normalize();
	rightVector->normalize();    
}


} // namespace cvf

