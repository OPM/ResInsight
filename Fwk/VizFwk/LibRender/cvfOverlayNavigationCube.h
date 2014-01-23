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

#include "cvfOverlayItem.h"
#include "cvfMatrix4.h"
#include "cvfColor3.h"
#include "cvfString.h"
#include "cvfBoundingBox.h"
#include "cvfCollection.h"

#include <map>

namespace cvf {

class Camera;
class DrawableGeo;
class DrawableVectors;
class Font;
class ShaderProgram;
class MatrixState;
class TextureImage;
class RenderState;

//==================================================================================================
//
// Overlay axis cross 
//
//==================================================================================================
class OverlayNavigationCube: public OverlayItem
{
public:
    enum NavCubeFace 
    {
        NCF_X_POS,
        NCF_X_NEG,
        NCF_Y_POS,
        NCF_Y_NEG,
        NCF_Z_POS,
        NCF_Z_NEG
    };

public:
    OverlayNavigationCube(Camera* camera, Font* font);
    ~OverlayNavigationCube();

    virtual Vec2ui      sizeHint();

    virtual void        render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual void        renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual bool        pick(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size);

    bool                updateHighlight(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size);
    bool                processSelection(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size, Vec3d* viewDir, Vec3d* up);

    void                setSize(const Vec2ui& size);
    void                setHome(const Vec3d& viewDirection, const Vec3d& up);
    void                setAxisLabels(const String& xLabel, const String& yLabel, const String& zlabel);
    void                setAxisLabelsColor(const Color3f& color);
    void                setFaceTexture(NavCubeFace face, TextureImage* texture);

private:
    // Note that the order of the items starting at the NCFI_BOTTOM_LEFT is important (in a CCW order)
    enum NavCubeFaceItem 
    {
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

private:
    void                render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);
    void                createAxisGeometry(bool software);
    void                renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState);
    void                renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState);
    void                renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState);
    void                renderCubeGeos(OpenGLContext* oglContext, bool software, const MatrixState& matrixState);
    void                render2dItems(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);

    void                createCubeGeos();
    void                createCubeFaceGeos(NavCubeFace face, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f p4);
    ref<DrawableGeo>    createQuadGeo(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& v4, const Vec2f& t1, const Vec2f& t2, const Vec2f& t3, const Vec2f& t4);
    void                navCubeCornerPoints(Vec3f points[8]);
    void                create2dGeos();
    ref<DrawableGeo>    create2dArrow(const Vec3f& start, const Vec3f& end);

    NavCubeItem         navCubeItem(NavCubeFace face, NavCubeFaceItem item) const;

    void                configureLocalCamera(Camera* camera, const Vec2i& position, const Vec2ui& size) const;

    void                viewConfigurationFromNavCubeItem(NavCubeItem item, Vec3d* viewDir, Vec3d* up);
    Vec3d               computeNewUpVector(const Vec3d& viewFrom, const Vec3d currentUp) const;

    size_t              pickItem(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size) const;
    NavCubeItem         pick2dItem(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size) const;

    void                updateTextureBindings(OpenGLContext* oglContext, bool software);
    bool                isFaceAlignedViewPoint() const;

    static Vec3d        snapToAxis(const Vec3d& vector, const Vec3d* pPreferIfEqual = NULL);
    static bool         vectorsParallelFuzzy(Vec3d v1, Vec3d v2);
    static int          findClosestAxis(const Vec3d& vector);

private:
    ref<Camera>                 m_camera;                   // This camera's view matrix will be used to orient the axis cross
    ref<Font>                   m_font;
    String                      m_xLabel;                   // Label to display on x axis, default 'x'
    String                      m_yLabel;           
    String                      m_zLabel;
    Color3f                     m_textColor;                // Text color 

    Vec2ui                      m_size;                     // Pixel size of the nav cube area

    Vec3d                       m_homeViewDirection;
    Vec3d                       m_homeUp;

    Collection<DrawableGeo>     m_cubeGeos;                 // These arrays have the same length
    std::vector<NavCubeItem>    m_cubeItemType;             // These arrays have the same length
    std::vector<NavCubeFace>    m_cubeGeoFace;              // These arrays have the same length

    ref<DrawableGeo>            m_homeGeo;                  // These arrays have the same length
    Collection<DrawableGeo>     m_2dGeos;                   // These arrays have the same length
    std::vector<NavCubeItem>    m_2dItemType;

    ref<ShaderProgram>          m_cubeGeoShader;
    ref<ShaderProgram>          m_cubeGeoTextureShader;
    ref<DrawableVectors>        m_axis;

    NavCubeItem	                m_hightlightItem;	        ///< The currently highlighted cube item (face, corner, edge, buttons)
    Vec3f		                m_upVector;			        ///< Specify the up vector, which is used for the orientation of the text and textures on the faces
    Vec3f                       m_frontVector;		        ///< Specify the front vector, which is used for the orientation of the top and bottom faces

    Color3f         		    m_xFaceColor;		        ///< The color of the X_POS and X_NEG faces
    Color3f         		    m_yFaceColor;		        ///< The color of the Y_POS and Y_NEG faces
    Color3f         		    m_zFaceColor;		        ///< The color of the Z_POS and Z_NEG faces
    Color3f                     m_itemHighlightColor;
    Color3f                     m_2dItemsColor;

    std::map<NavCubeFace, ref<TextureImage> >   m_faceTextures;
    std::map<NavCubeFace, ref<RenderState> >    m_faceTextureBindings;
};

}
