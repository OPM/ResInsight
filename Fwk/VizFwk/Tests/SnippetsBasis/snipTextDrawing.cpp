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


#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"
#include "cvfuInputEvents.h"
#include "cvfFreeTypeFont.h"
#include "cvfFixedAtlasFont.h"

#include "cvfuSnippetPropertyPublisher.h"

#include "snipTextDrawing.h"
#include "snipTextEmbeddedFreeTypeFont.h"
#include "snipTextEmbeddedFixedAtlasFont.h"


// For debugging
#include "cvfuImageJpeg.h"


namespace snip {

using namespace cvfu;

static const int CVF_SNIP_TEXTDRAWING_MIN_FONT_SIZE = 4;
static const int CVF_SNIP_TEXTDRAWING_DPI_X = 96;   // QApplication::desktop()->physicalDpiX()
static const int CVF_SNIP_TEXTDRAWING_DPI_Y = 96;   // QApplication::desktop()->physicalDpiY()

static const String CVF_SNIP_TEXTDRAWING_FONT_A = "FreeType embedded font";
static const String CVF_SNIP_TEXTDRAWING_FONT_B = "Fixed Atlas Standard";
static const String CVF_SNIP_TEXTDRAWING_FONT_C = "Fixed Atlas Large";
static const String CVF_SNIP_TEXTDRAWING_FONT_D = "C:\\Windows\\Fonts\\arialuni.TTF";
static const String CVF_SNIP_TEXTDRAWING_FONT_E = "C:\\Windows\\Fonts\\georgia.TTF";
static const String CVF_SNIP_TEXTDRAWING_FONT_F = "C:\\Windows\\Fonts\\CENTURY.TTF";
static const String CVF_SNIP_TEXTDRAWING_FONT_G = "C:\\Windows\\Fonts\\verdana.TTF";

// Local Droid Sans fonts
static const String CVF_SNIP_TEXTDRAWING_FONT_H = "C:\\Temp\\Droid_Sans\\DroidSans.ttf";
static const String CVF_SNIP_TEXTDRAWING_FONT_I = "C:\\Temp\\base-3541b65\\base-3541b65\\DroidSansFallback.ttf";
static const String CVF_SNIP_TEXTDRAWING_FONT_J = "C:\\Temp\\base-3541b65\\base-3541b65\\DroidSansFallbackLegacy.ttf";



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TextDrawing::onInitialize()
{
    // Add 3D geometry rendering
    // ------------------------------------------
    ref<Part> cubePart = createCubePart();
    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(cubePart.p());
    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());


    // Add text rendering
    // ------------------------------------------
//     ShaderProgramGenerator gen("TextDrawing", ShaderSourceProvider::instance());
//     gen.addVertexCode(ShaderSourceRepository::vs_MinimalTexture);
//     gen.addFragmentCode(ShaderSourceRepository::fs_Text);
//     ref<ShaderProgram> prog = gen.generate();
// 

    ref<Part> textPart = createTextPart();
    ref<ModelBasicList> textModel = new ModelBasicList;
    textModel->addPart(textPart.p());

    ref<Scene> textScene = new Scene;
    textScene->addModel(textModel.p());

    ref<Rendering> textRendering = new Rendering;
    textRendering->setScene(textScene.p());
    textRendering->setClearMode(Viewport::DO_NOT_CLEAR);
    textRendering->cullSettings()->enableViewFrustumCulling(false);
    textRendering->cullSettings()->enablePixelSizeCulling(false);
    textRendering->renderEngine()->enableForcedImmediateMode(true);
    m_renderSequence->addRendering(textRendering.p());

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::X_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    // Set up font
    // ------------------------------------------
    CVF_ASSERT(m_font.isNull());
    m_fontSize = CVF_SNIP_TEXTDRAWING_MIN_FONT_SIZE * 4;

    // Set up properties
    // ------------------------------------------
    m_propFontName = new PropertyEnum("Font name");
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_A, CVF_SNIP_TEXTDRAWING_FONT_A);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_B, CVF_SNIP_TEXTDRAWING_FONT_B);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_C, CVF_SNIP_TEXTDRAWING_FONT_C);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_D, CVF_SNIP_TEXTDRAWING_FONT_D);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_E, CVF_SNIP_TEXTDRAWING_FONT_E);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_F, CVF_SNIP_TEXTDRAWING_FONT_F);
    m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_G, CVF_SNIP_TEXTDRAWING_FONT_G);
//     // Local Droid Sans fonts
//     m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_H, CVF_SNIP_TEXTDRAWING_FONT_H);
//     m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_I, CVF_SNIP_TEXTDRAWING_FONT_I);
//     m_propFontName->addItem(CVF_SNIP_TEXTDRAWING_FONT_J, CVF_SNIP_TEXTDRAWING_FONT_J);
    m_propFontName->setCurrentIdent(CVF_SNIP_TEXTDRAWING_FONT_A);
    m_propertyPublisher->publishProperty(m_propFontName.p());
    updateFont(m_propFontName->currentIdent());

    m_propFontSize = new PropertyInt("Font size", m_fontSize);
    m_propFontSize->setRange(1, 36);
    m_propertyPublisher->publishProperty(m_propFontSize.p());
    updateFont(m_fontSize);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawing::onResizeEvent(int width, int height)
{
    // Update text rendering
    ref<Camera> textCamera = m_renderSequence->rendering(1)->camera();
    textCamera->viewport()->set(0, 0, width, height);
    textCamera->setProjectionAsPixelExact2D();

    // Finally, call base
    TestSnippet::onResizeEvent(width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawing::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    CVF_ASSERT(property);
    CVF_ASSERT(postEventAction);

    *postEventAction = NONE;

    if (property == m_propFontName)
    {
        String ident = m_propFontName->currentIdent();
        if (updateFont(ident))
        {
            *postEventAction = REDRAW;
        }
    }
    else if (property == m_propFontSize)
    {
        m_fontSize = m_propFontSize->value();
        if (updateFont(m_fontSize))
        {
            *postEventAction = REDRAW;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> TextDrawing::helpText() const
{
    std::vector<cvf::String> helpText;

    // Acknowledge the FreeType 2 License requirements
    helpText.push_back(L"");
    helpText.push_back(L"Credits:");
    helpText.push_back(FreeTypeFont::credit());

    return helpText;
}


//--------------------------------------------------------------------------------------------------
/// Create 3D geometry part
//--------------------------------------------------------------------------------------------------
ref<Part> TextDrawing::createCubePart()
{
    // Create cube geometry
    ref<DrawableGeo> geo = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createBox(Vec3f(0,0,0), 2.0, 2.0, 2.0, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        geo->setVertexArray(vertices.p());
        geo->setFromFaceList(*faceList);
        geo->computeNormals();
    }

    ref<Part> part = new Part;
    part->setDrawable(geo.p());

    ref<Effect> eff = new Effect;
    eff->setRenderState(new RenderStateMaterial_FF(Color3f(0, 1, 0)));
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// Create text part
//--------------------------------------------------------------------------------------------------
ref<Part> TextDrawing::createTextPart()
{
    m_textDrawable = new DrawableText;
    m_textDrawable->setFont(NULL);
    m_textDrawable->setTextColor(Color3::RED);

//     m_textDrawable->addText(L"Text:",                                               cvf::Vec3f(50, 200, 0));
//     m_textDrawable->addText(L"   ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ",                    cvf::Vec3f(50, 170, 0));
//     m_textDrawable->addText(L"   abcdefghijklmnopqrstuvwxyzæøå",                    cvf::Vec3f(50, 140, 0));
//     m_textDrawable->addText(L"   0123456789 0123456789 0123456789",                 cvf::Vec3f(50, 110, 0));
//     m_textDrawable->addText(L"   !\"#¤%&/()=?`^*@£$€{[]}´~¨',;.:+-_<>><VAWAVAA",    cvf::Vec3f(50,  80, 0));
//     m_textDrawable->addText(L"   Unicode1: \x03B1\x03B2\x03B3\x03B4\x03B5",         cvf::Vec3f(50,  50, 0));
//     m_textDrawable->addText(L"   Unicode2: 不仅是因为这两种语言截然不同",             cvf::Vec3f(50,  20, 0));

    // Set up transparency
    ref<RenderStateBlending> blending = new RenderStateBlending;
    blending->configureTransparencyBlending();

    ref<Effect> eff = new Effect;
//     eff->setRenderState(new RenderStateMaterial_FF(Color3f(1, 1, 1)));
//     eff->setRenderState(new Depth(false, Depth::LESS, false));
    eff->setRenderState(new RenderStateLighting_FF(false));
    eff->setRenderState(blending.p());
//     eff->setShaderProgram(shaderProgram);
//     //eff->setUniform(new UniformFloat("u_color", Color3f(Color3::RED)));
//     eff->setUniform(new UniformInt("u_texture2D", 0));

    ref<Part> part = new Part;
    part->setDrawable(m_textDrawable.p());
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TextDrawing::updateFont(const String& ident)
{
    if (ident == CVF_SNIP_TEXTDRAWING_FONT_A)
    {
        FreeTypeFont* font = new FreeTypeFont(CVF_SNIP_TEXTDRAWING_DPI_X, CVF_SNIP_TEXTDRAWING_DPI_Y);
        font->load(CVF_SNIPTEXT_EMBEDDED_FREETYPE_FONT_DATA, CVF_SNIPTEXT_EMBEDDED_FREETYPE_FONT_DATA_BLOCK_COUNT);
        font->setSize(m_fontSize);
        m_textDrawable->setFont(font);
        m_font = font;
    }
    else if (ident == CVF_SNIP_TEXTDRAWING_FONT_B)
    {
        FixedAtlasFont* font = new FixedAtlasFont(FixedAtlasFont::STANDARD);
        m_textDrawable->setFont(font);
        m_font = font;       
    }
    else if (ident == CVF_SNIP_TEXTDRAWING_FONT_C)
    {
        FixedAtlasFont* font = new FixedAtlasFont(FixedAtlasFont::LARGE);
        m_textDrawable->setFont(font);
        m_font = font;       
    }
    else if ((ident == CVF_SNIP_TEXTDRAWING_FONT_D) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_E) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_F) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_G) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_H) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_I) ||
        (ident == CVF_SNIP_TEXTDRAWING_FONT_J))
    {
        FreeTypeFont* font = new FreeTypeFont(CVF_SNIP_TEXTDRAWING_DPI_X, CVF_SNIP_TEXTDRAWING_DPI_Y);
        font->load(ident);
        font->setSize(m_fontSize);
        m_textDrawable->setFont(font);
        m_font = font;
    }
    else
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TextDrawing::updateFont(cvf::uint size)
{
    FreeTypeFont* freeTypeFont = dynamic_cast<FreeTypeFont*>(m_font.p());
    if (!freeTypeFont) return false;

    m_fontSize = size;
    freeTypeFont->setSize(m_fontSize);
    return true;
}

} // namespace snip
