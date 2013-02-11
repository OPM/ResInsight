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
#include "cvfBoundingBox.h"
#include "cvfCollection.h"

namespace cvf {

class Camera;
class DrawableGeo;
class DrawableVectors;
class Font;
class ShaderProgram;
class MatrixState;
class TextureImage;


//==================================================================================================
//
// Overlay axis cross 
//
//==================================================================================================
class OverlayNavigationCube: public OverlayItem
{
public:
    enum NavCubeFace {
        NCF_NONE,
        NCF_X_POS,
        NCF_X_NEG,
        NCF_Y_POS,
        NCF_Y_NEG,
        NCF_Z_POS,
        NCF_Z_NEG
    };

    // Note that the order of the items starting at the VT_NCFI_BOTTOM_LEFT is important (in a CCW order)
    enum NavCubeFaceItem {
        NCFI_NONE,
        NCFI_CENTER,
        NCFI_BOTTOM_LEFT,
        NCFI_BOTTOM,
        NCFI_BOTTOM_RIGHT,
        NCFI_RIGHT,
        NCFI_TOP_RIGHT,
        NCFI_TOP,
        NCFI_TOP_LEFT,
        NCFI_LEFT
    };

    enum NavCubeItem
    {
        NCI_NONE,

        NCI_CORNER_XN_YN_ZN,
        NCI_CORNER_XP_YN_ZN,
        NCI_CORNER_XP_YP_ZN,
        NCI_CORNER_XN_YP_ZN,
        NCI_CORNER_XN_YN_ZP,
        NCI_CORNER_XP_YN_ZP,
        NCI_CORNER_XP_YP_ZP,
        NCI_CORNER_XN_YP_ZP,

        NCI_EDGE_YN_ZN,
        NCI_EDGE_XP_ZN,
        NCI_EDGE_YP_ZN,
        NCI_EDGE_XN_ZN,
        NCI_EDGE_YN_ZP,
        NCI_EDGE_XP_ZP,
        NCI_EDGE_YP_ZP,
        NCI_EDGE_XN_ZP,
        NCI_EDGE_XN_YN,
        NCI_EDGE_XP_YN,
        NCI_EDGE_XP_YP,
        NCI_EDGE_XN_YP,

        NCI_FACE_X_POS,
        NCI_FACE_X_NEG,
        NCI_FACE_Y_POS,
        NCI_FACE_Y_NEG,
        NCI_FACE_Z_POS,
        NCI_FACE_Z_NEG,

        NCI_ARROW_LEFT,
        NCI_ARROW_RIGHT,
        NCI_ARROW_TOP,
        NCI_ARROW_BOTTOM,
        NCI_HOME,
        NCI_ROTATE_CW,
        NCI_ROTATE_CCW
    };

public:
    OverlayNavigationCube(Camera* camera, Font* font);
    ~OverlayNavigationCube();

    virtual Vec2ui  sizeHint();
    virtual Vec2ui  maximumSize();
    virtual Vec2ui  minimumSize();

    virtual void    render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual void    renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);

    void            setSize(const Vec2ui& size);
    void            updateHighlight(int winCoordX, int winCoordY);
    void            processSelection(int winCoordX, int winCoordY, const BoundingBox& boundingBox, Vec3d* eye, Vec3d* viewDirection);

    void            setAxisLabels(const String& xLabel, const String& yLabel, const String& zlabel);
    void            setAxisLabelsColor(const Color3f& color);

private:
    void render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software, const Mat4d& viewMatrix);
    void createAxisGeometry(bool software);
    void renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState);
    void renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState);
    void renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState);
    void renderCubeGeos(OpenGLContext* oglContext, bool software, const MatrixState& matrixState);

    void createCubeGeos();
    void createCubeFaceGeos(NavCubeFace face, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f p4);//, const String& name, const Color3f& baseColor, TextureImage* texture);
    void navCubeCornerPoints(Vec3f points[8]);
    ref<DrawableGeo> createQuadGeo(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& v4);

    NavCubeItem navCubeItem(NavCubeFace face, NavCubeFaceItem item) const;
    void        faceOrientation(NavCubeFace face, Vec3f* normal, Vec3f* upVector, Vec3f* rightVector) const;

private:
    ref<Camera>             m_camera;       // This camera's view matrix will be used to orient the axis cross
    String                  m_xLabel;       // Label to display on x axis, default 'x'
    String                  m_yLabel;       
    String                  m_zLabel;
    Color3f                 m_textColor;    // Text color 
    ref<Font>               m_font;

    Vec2ui                  m_size;         // Pixel size of the axis area

    Collection<DrawableGeo>     m_cubeGeos;
    std::vector<NavCubeItem>    m_cubeItemType;
    ref<ShaderProgram>          m_cubeGeoShader;
    ref<DrawableVectors>        m_axis;

    NavCubeItem	            m_hightlightItem;	///< The currently highlighted cube item (face, corner, edge, buttons)
    Vec3f		            m_upVector;			///< Specify the up vector, which is used for the orientation of the text and textures on the faces
    Vec3f                   m_frontVector;		///< Specify the front vector, which is used for the orientation of the top and bottom faces

    String          		m_xPosAxisName;	///< The name of the X_POS face
    String          		m_xNegAxisName;	///< The name of the X_NEG face
    String          		m_yPosAxisName;	///< The name of the Y_POS face
    String          		m_yNegAxisName;	///< The name of the Y_NEG face
    String          		m_zPosAxisName;	///< The name of the Z_POS face
    String          		m_zNegAxisName;	///< The name of the Z_NEG face

    ref<TextureImage>		m_texturePosXAxis;	///< The texture to draw on the X_POS face. If NULL, the specified text will be drawn.
    ref<TextureImage>		m_textureNegXAxis;	///< The texture to draw on the X_NEG face. If NULL, the specified text will be drawn.
    ref<TextureImage>		m_texturePosYAxis;	///< The texture to draw on the Y_POS face. If NULL, the specified text will be drawn.
    ref<TextureImage>		m_textureNegYAxis;	///< The texture to draw on the Y_NEG face. If NULL, the specified text will be drawn.
    ref<TextureImage>		m_texturePosZAxis;	///< The texture to draw on the Z_POS face. If NULL, the specified text will be drawn.
    ref<TextureImage>		m_textureNegZAxis;	///< The texture to draw on the Z_NEG face. If NULL, the specified text will be drawn.

    Color3f         		m_xFaceColor;		///< The color of the X_POS and X_NEG faces
    Color3f         		m_yFaceColor;		///< The color of the Y_POS and Y_NEG faces
    Color3f         		m_zFaceColor;		///< The color of the Z_POS and Z_NEG faces
};

}

