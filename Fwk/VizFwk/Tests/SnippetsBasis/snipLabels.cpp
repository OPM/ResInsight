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

#include "snipLabels.h"


// For debugging
#include "cvfuImageJpeg.h"


namespace snip {

using namespace cvfu;

static const int CVF_SNIP_Labels_MIN_FONT_SIZE = 4;
static const int CVF_SNIP_Labels_DPI_X = 96;   // QApplication::desktop()->physicalDpiX()
static const int CVF_SNIP_Labels_DPI_Y = 96;   // QApplication::desktop()->physicalDpiY()

static const String CVF_SNIP_Labels_FONT_B = "Fixed Atlas Standard";
static const String CVF_SNIP_Labels_FONT_C = "Fixed Atlas Large";



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Labels::onInitialize()
{
    // Add 3D geometry rendering
    // ------------------------------------------
    ref<Part> cubePart = createCubePart();
    ref<ModelBasicList> myModel = new ModelBasicList;
    cubePart->setPriority(1);
    myModel->addPart(cubePart.p());

    addTextParts(myModel.p());

    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::X_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    ref<OverlayAxisCross> axisCross = new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD));
    m_renderSequence->firstRendering()->addOverlayItem(axisCross.p());

    ref<OverlayColorLegend> legend = new OverlayColorLegend(new FixedAtlasFont(FixedAtlasFont::STANDARD));
    legend->setTitle("Legend title\nLegend unit");
    legend->setColor(Color3::DARK_RED);
    legend->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
    m_renderSequence->firstRendering()->addOverlayItem(legend.p());
    
    // Set up font
    // ------------------------------------------
    CVF_ASSERT(m_font.isNull());
    m_fontSize = CVF_SNIP_Labels_MIN_FONT_SIZE * 4;

    // Set up properties
    // ------------------------------------------
    m_propFontName = new PropertyEnum("Font name");
    m_propFontName->addItem(CVF_SNIP_Labels_FONT_B, CVF_SNIP_Labels_FONT_B);
    m_propFontName->addItem(CVF_SNIP_Labels_FONT_C, CVF_SNIP_Labels_FONT_C);
    m_propFontName->setCurrentIdent(CVF_SNIP_Labels_FONT_B);
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
void Labels::onPropertyChanged(Property* property, PostEventAction* postEventAction)
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
std::vector<cvf::String> Labels::helpText() const
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
ref<Part> Labels::createCubePart()
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
void Labels::addTextParts(cvf::ModelBasicList* model)
{
    m_textDrawable = new DrawableText;
    m_textDrawable->setFont(NULL);
    m_textDrawable->setTextColor(Color3::BLACK);
    m_textDrawable->addText(L"Bottom left: <-1 -1 -1>. This is a very long textqlqpsqplpfewn"                          ,Vec3f(-1.0f, -1.0f, -1.0f));
    m_textDrawable->addText(L"Bottom left: <-1 -1 -1>"                                                               ,Vec3f(-1.0f, -1.0f, -1.0f));
    m_textDrawable->addText(L"Just Outside: 1.1 1.1 1.1"                                                             ,Vec3f(1.1f, 1.1f, 1.1f));    
    m_textDrawable->addText(L"We have received credible information\nvery recently about a possible plot directed at the homeland\nthat seems to be focused on New York and Washington, D.C.\nThe official said the plot was believed to involve\nthree individuals, including a U.S. citizen." ,Vec3f(-1.0f, -1.0f, -1.9f)); 
//     m_textDrawable->setDrawBorder(false);
//     m_textDrawable->setDrawBackground(false);

    m_textDrawable->addText("First line\nSecond longer line", Vec3f(-1.0f, -1.0f, -3.1f));

    m_textDrawableOrigin = new DrawableText;
    m_textDrawableOrigin->setFont(new FixedAtlasFont(FixedAtlasFont::LARGE));
    m_textDrawableOrigin->setTextColor(Color3::RED);
    m_textDrawableOrigin->setBackgroundColor(Color3::WHITE);
    m_textDrawableOrigin->setBorderColor(Color3::PINK);
    m_textDrawableOrigin->setCheckPosVisible(false);
    m_textDrawableOrigin->addText("Origin", Vec3f(0.0f, 0.0f, 0.0f));
    m_textDrawableOrigin->addText("Multi line large text test\nSecond large text", Vec3f(1,0,0));

    ref<Effect> eff = new Effect;
    if (ShaderProgram::supportedOpenGL(m_openGLContext.p()))
    {
        ref<ShaderProgram> shaderProgram = m_openGLContext->resourceManager()->getLinkedTextShaderProgram(m_openGLContext.p());
        eff->setShaderProgram(shaderProgram.p());
    }

    ref<Part> part = new Part;
    part->setDrawable(m_textDrawable.p());
    part->setEffect(eff.p());
    part->setPriority(2);

    ref<Part> originPart = new Part;
    originPart->setDrawable(m_textDrawableOrigin.p());
    originPart->setEffect(eff.p());
    originPart->setPriority(2);

    model->addPart(part.p());
    model->addPart(originPart.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Labels::updateFont(const String& ident)
{
    if (m_textDrawable.notNull())
    {
        if (ident == CVF_SNIP_Labels_FONT_B)
        {
            FixedAtlasFont* font = new FixedAtlasFont(FixedAtlasFont::STANDARD);
            m_textDrawable->setFont(font);
            m_font = font;       
        }
        else if (ident == CVF_SNIP_Labels_FONT_C)
        {
            FixedAtlasFont* font = new FixedAtlasFont(FixedAtlasFont::LARGE);
            m_textDrawable->setFont(font);
            m_font = font;       
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Labels::updateFont(cvf::uint size)
{
    FreeTypeFont* freeTypeFont = dynamic_cast<FreeTypeFont*>(m_font.p());
    if (!freeTypeFont) return false;

    m_fontSize = size;
    freeTypeFont->setSize(m_fontSize);
    return true;
}

} // namespace snip
