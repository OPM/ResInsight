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
#include "cvfLibViewing.h"
#include "cvfLibRender.h"

#include "snipOverlayItems.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"
#include "cvfuImageJpeg.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OverlayItems::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    m_model = new ModelBasicList;

    cvf::ShaderSourceProvider::instance();

    ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    m_shaderProg  = spGen.generate();

    m_useShaders = false;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();
        ref<Effect> eff = part->effect();

        eff->setShaderProgram(m_useShaders ? m_shaderProg.p() : NULL);

        m_model->addPart(parts[i].p());
    }

    m_model->updateBoundingBoxesRecursive();

    m_renderSequence->firstRendering()->scene()->addModel(m_model.p());

    //addOverlayItemsVariousTypes();
    addOverlayItemsTestLayout();

    BoundingBox bb = m_model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItems::addOverlayItemsVariousTypes()
{
    // Setup the overlay items
    {
        ref<OverlayAxisCross> overlay = new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD));
        overlay->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_LEFT);
        m_renderSequence->firstRendering()->addOverlayItem(overlay.p());
    }
    

    // Image overlay - Global Alpha
    {
        ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "CeetronLogoDiffuse.jpg");
        ref<OverlayImage> overlay = new OverlayImage(img.p());
        overlay->setGlobalAlpha(0.7f);
        overlay->setBlending(cvf::OverlayImage::GLOBAL_ALPHA);
        overlay->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(overlay.p());
    }

    // Image overlay - Opaque small
    {
        ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "CeetronLogoDiffuse.jpg");
        ref<OverlayImage> overlay = new OverlayImage(img.p());
        overlay->setPixelSize(cvf::Vec2ui(img->width()/2, img->height()/2));
        overlay->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(overlay.p());
    }

    // Image overlay - Texture Alpha
    {
        ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "CeetronLogoDiffuse.jpg");

        // Create alpha values, as we cannot ready any format with alpha...
        for (cvf::uint y = 0; y < img->height(); ++y)
        {
            for (cvf::uint x = 0; x < img->width(); ++x)
            {
                cvf::Color4ub pixel = img->pixel(x, y);
                if (pixel.r() >= 245 && pixel.g() >= 245 && pixel.b() >= 245)
                {
                    pixel.a() = 0;
                    img->setPixel(x, y, pixel);
                }
            }
        }

        ref<OverlayImage> overlay = new OverlayImage(img.p());
        overlay->setBlending(cvf::OverlayImage::TEXTURE_ALPHA);
        overlay->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(overlay.p());
    }

    ref<OverlayTextBox> tb1 = new OverlayTextBox(new FixedAtlasFont(FixedAtlasFont::STANDARD));
    tb1->setText("This is the first text box");
    tb1->setTextColor(Color3f(Color3::WHITE));
    tb1->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
    m_renderSequence->firstRendering()->addOverlayItem(tb1.p());

    ref<OverlayTextBox> tb2 = new OverlayTextBox(new FixedAtlasFont(FixedAtlasFont::LARGE));
    tb2->setText("This is LARGE text");
    tb2->setDrawBackground(true);
    tb2->setDrawBorder(true);
    tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
    m_renderSequence->firstRendering()->addOverlayItem(tb2.p());

    ref<Font> font = new FixedAtlasFont(FixedAtlasFont::LARGE);
    ref<Font> fontStd = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Lower Die";
        tb2->setText(text);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }
    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Billet";
        tb2->setText(text);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }
    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Upper Die - hard coded";
        Vec2ui size = font->textExtent(text);
        size.x() += 10; size.y() += 10;
        tb2->setText(text);
        tb2->setPixelSize(size);
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::BOTTOM_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }


    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Example View Title here";
        tb2->setText(text);
        tb2->setTextColor(Color3f(0,0,0));
        tb2->setDrawBorder(false);
        tb2->setDrawBackground(false);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }
    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(fontStd.p());
        String text = "An example view description here";
        tb2->setText(text);
        tb2->setDrawBorder(false);
        tb2->setSizeToFitText();
        tb2->setDrawBackground(false);
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Only background";
        tb2->setText(text);
        tb2->setDrawBorder(false);
        tb2->setSizeToFitText();
        tb2->setBackgroundColor(Color3::CRIMSON);
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }
    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Only border";
        tb2->setText(text);
        tb2->setDrawBackground(false);
        tb2->setSizeToFitText();
        tb2->setBorderColor(Color3::GOLD);
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(font.p());
        String text = "Multi line overlay text\nLarge font";
        tb2->setText(text);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(fontStd.p());
        String text = "Multi line overlay text\nStandard font\nHard coded size";
        Vec2ui size = fontStd->textExtent(text);
        size.x() += 10; size.y() += 50;
        tb2->setPixelSize(size);
        tb2->setText(text);
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(fontStd.p());
        String text = "Single line size to fit with border text GujYg";
        tb2->setText(text);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

    {
        ref<OverlayTextBox> tb2 = new OverlayTextBox(fontStd.p());
        String text = "Single line size to fit no border text GujYg";
        tb2->setDrawBorder(false);
        tb2->setText(text);
        tb2->setSizeToFitText();
        tb2->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_RIGHT);
        m_renderSequence->firstRendering()->addOverlayItem(tb2.p());
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItems::addOverlayItemsTestLayout()
{
    ref<Font> font = new FixedAtlasFont(FixedAtlasFont::STANDARD);

    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::HORIZONTAL, font.p(), "TLH1");
    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::HORIZONTAL, font.p(), "TLH2\nWith one extra lines");
    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::HORIZONTAL, font.p(), "TLH3 - long text string to occupy a lot of horizontal space");
    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::HORIZONTAL, font.p(), "TLH4");
    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::VERTICAL,   font.p(), "TLV1");
    createAddOverlayTextBox(OverlayItem::TOP_LEFT,      OverlayItem::VERTICAL,   font.p(), "TLV2");

    createAddOverlayTextBox(OverlayItem::TOP_RIGHT,     OverlayItem::VERTICAL,   font.p(), "TRV1");

    createAddOverlayTextBox(OverlayItem::BOTTOM_LEFT,   OverlayItem::HORIZONTAL, font.p(), "BLH1");
    createAddOverlayTextBox(OverlayItem::BOTTOM_LEFT,   OverlayItem::HORIZONTAL, font.p(), "BLH2\nmulti\nline");
    createAddOverlayTextBox(OverlayItem::BOTTOM_LEFT,   OverlayItem::VERTICAL,   font.p(), "BLV1 - a longer text here");

    createAddOverlayTextBox(OverlayItem::BOTTOM_RIGHT,  OverlayItem::HORIZONTAL, font.p(), "BRH1");
    createAddOverlayTextBox(OverlayItem::BOTTOM_RIGHT,  OverlayItem::HORIZONTAL, font.p(), "BRH2");
    createAddOverlayTextBox(OverlayItem::BOTTOM_RIGHT,  OverlayItem::HORIZONTAL, font.p(), "BRH3 ... with a long text string to occupy some space");
    createAddOverlayTextBox(OverlayItem::BOTTOM_RIGHT,  OverlayItem::VERTICAL,   font.p(), "BRV1");

    createAddFixedPosOverlayTextBox(150, 100, font.p(), "F1");
    createAddFixedPosOverlayTextBox(150, 140, font.p(), "F2");
    createAddFixedPosOverlayTextBox(10,  10,  font.p(), "F3");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItems::createAddOverlayTextBox(OverlayItem::AnchorCorner corner, OverlayItem::LayoutScheme scheme, Font* font, const String& textString)
{
    ref<OverlayTextBox> tb = new OverlayTextBox(font);

    String text = textString;
    switch (corner)
    {
        case OverlayItem::TOP_LEFT:     text += "\nTopLeft"; break;
        case OverlayItem::TOP_RIGHT:    text += "\nTopRight"; break;
        case OverlayItem::BOTTOM_LEFT:  text += "\nBottomLeft"; break;
        case OverlayItem::BOTTOM_RIGHT: text += "\nBottomLeft"; break;
        default:                        CVF_FAIL_MSG("Unhandled");
    }

    switch (scheme)
    {
        case OverlayItem::HORIZONTAL:  text += "\nHorizontal"; break;
        case OverlayItem::VERTICAL:    text += "\nVertical"; break;
        default:                       CVF_FAIL_MSG("Unhandled");
    }

    tb->setText(text);
    tb->setSizeToFitText();
    tb->setLayout(scheme, corner);
    m_renderSequence->firstRendering()->addOverlayItem(tb.p());
}


void OverlayItems::createAddFixedPosOverlayTextBox(int posX, int posY, Font* font, const String& textString)
{
    ref<OverlayTextBox> tb = new OverlayTextBox(font);

    String text = textString;
    text += String("pos=(%1, %2)").arg(posX).arg(posY);

    tb->setText(text);
    tb->setSizeToFitText();
    tb->setBackgroundColor(cvf::Color3::RED);
    tb->setLayoutFixedPosition(Vec2i(posX, posY));
    m_renderSequence->firstRendering()->addOverlayItem(tb.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItems::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (buttonPressed == LeftButton && mouseEvent->modifiers() == ControlModifier)
    {
        Rendering* mainRendering = m_renderSequence->rendering(0);

        int winCoordX = mouseEvent->x();
        int winCoordY = mouseEvent->y();
        Trace::show("\nPicking overlay item at (%d,%d)...", winCoordX, winCoordY);
        OverlayItem* item = mainRendering->overlayItemFromWindowCoordinates(winCoordX, winCoordY);
        if (item)
        {
            if (dynamic_cast<OverlayTextBox*>(item))
            {
                OverlayTextBox* textItem = dynamic_cast<OverlayTextBox*>(item);
                Trace::show("Hit text box with text:\n%s", textItem->text().toAscii().ptr());
            }
            else
            {
                Trace::show("Hit some overlay item");
            }
        }
        else
        {
            Trace::show("NO hits");
        }

        return;
    }


    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItems::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 's')
    {
        m_useShaders = true;
    }
    if (keyEvent->character() == 'f')
    {
        m_useShaders = false;
    }

    Collection<Part> partCollection;
    m_model->allParts(&partCollection);

    size_t i;
    for (i = 0; i < partCollection.size(); i++)
    {
        ref<Part> part = partCollection[i];
        CVF_ASSERT(part.notNull());

        part->effect()->setShaderProgram(m_useShaders ? m_shaderProg.p() : NULL);
    }

    keyEvent->setRequestedAction(REDRAW);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> OverlayItems::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'s' to use a shader program for rendering");
    helpText.push_back("'f' to use fixed function pipeline for rendering");

    return helpText;
}




} // namespace snip

