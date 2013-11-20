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

#include "snipShaderUniforms.h"

#include "cvfuInputEvents.h"
#include "cvfuSnippetPropertyPublisher.h"


namespace snip {


// Fake a global uniform set by deriving from DynamicUniformSet and doing nothing in update()
class MyGlobalUniformSet : public DynamicUniformSet
{
public:
    MyGlobalUniformSet()
    {
        m_uniformSet = new UniformSet;
        setColor1(Color3f::RED);
    }

    void setColor1(Color3f color)
    {
        m_uniformSet->setUniform(new UniformFloat("u_color1", color));
    }

    virtual UniformSet* uniformSet()
    {
        return m_uniformSet.p();
    }

    virtual void update(Rendering*)
    {
    }

private:
    cvf::ref<cvf::UniformSet>   m_uniformSet;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderUniforms::onInitialize()
{
    m_propGlobalUniformSet = new PropertyBool("Use global uniform set", true);
    m_propertyPublisher->publishProperty(m_propGlobalUniformSet.p());

    // Create the single shader program with default uniforms
    ref<ShaderProgram> shaderProg;
    {
        ShaderProgramGenerator shaderGen("ShaderUniforms", ShaderSourceProvider::instance());
        shaderGen.addVertexCodeFromFile("ShaderUniforms_Vert");
        shaderGen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
        shaderGen.addFragmentCodeFromFile("ShaderUniforms_Frag");
        shaderProg = shaderGen.generate();
        shaderProg->setDefaultUniform(new UniformFloat("u_color0", Color3f(Color3::WHITE)));
        shaderProg->setDefaultUniform(new UniformFloat("u_color1", Color3f(Color3::MAGENTA)));
        shaderProg->setDefaultUniform(new UniformFloat("u_colorMixFactor", 1.0f));
    }

    // No uniforms, relies only on the shader program defaults
    m_effNoUniforms = new Effect;
    m_effNoUniforms->setShaderProgram(shaderProg.p());

    // Effect with only mix factor set (uses colors from default)
    m_effWithSomeUniforms = new Effect;
    m_effWithSomeUniforms->setShaderProgram(shaderProg.p());
    m_effWithSomeUniforms->setUniform(new UniformFloat("u_colorMixFactor", 0));

    // Fully specified uniforms
    m_effWithAllUniforms = new Effect;
    m_effWithAllUniforms->setShaderProgram(shaderProg.p());
    m_effWithAllUniforms->setUniform(new UniformFloat("u_color0", Color3f(Color3::BLACK)));
    m_effWithAllUniforms->setUniform(new UniformFloat("u_color1", Color3f(Color3::GREEN)));
    m_effWithAllUniforms->setUniform(new UniformFloat("u_colorMixFactor", 1.0f));


    // Box part
    ref<Part> boxPart = new Part;
    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(-2, 0, 0), Vec3d(2, 2, 2));
        GeometryBuilderDrawableGeo builder;
        gen.generate(&builder);
        boxPart->setDrawable(builder.drawableGeo().p());
    }

    // Sphere part
    ref<Part> spherePart = new Part;
    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(1, 10, 10, &builder);
        spherePart->setDrawable(builder.drawableGeo().p());
    }

    // Cylinder part
    ref<Part> cylinderPart = new Part;
    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createObliqueCylinder(1, 1, 2, 0, 0, 10, true, true, true, 1, &builder);
        ref<DrawableGeo> geo = builder.drawableGeo();
        geo->transform(Mat4d::fromTranslation(Vec3d(2, 0, -1)));
        cylinderPart->setDrawable(geo.p());
    }

    ref<ModelBasicList> myModel = new ModelBasicList;
    boxPart->setEffect(m_effNoUniforms.p());
    spherePart->setEffect(m_effWithSomeUniforms.p());
    cylinderPart->setEffect(m_effWithAllUniforms.p());

    myModel->addPart(boxPart.p());
    myModel->addPart(spherePart.p());
    myModel->addPart(cylinderPart.p());
    myModel->updateBoundingBoxesRecursive();


    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    applyCurrentProperties();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderUniforms::applyCurrentProperties()
{
    ref<Rendering> rendering = m_renderSequence->firstRendering();

    String text;

    if (m_propGlobalUniformSet->value())
    {
        ref<MyGlobalUniformSet> globUniformSet = new MyGlobalUniformSet;
        rendering->addGlobalDynamicUniformSet(globUniformSet.p());
        text = "With global uniform set: box=red, sphere=white, cylinder=green";
    }
    else
    {
        rendering->removeAllGlobalDynamicUniformSets();
        text = "Without global uniform set: box=magenta, sphere=white, cylinder=green";
    }

    // Add text box to describe the expected result
    ref<Font> font = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    ref<OverlayTextBox> textBox = new OverlayTextBox(font.p());
    textBox->setText(text);
    textBox->setSizeToFitText();
    textBox->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
    rendering->removeAllOverlayItems();
    rendering->addOverlayItem(textBox.p());
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderUniforms::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    if (property == m_propGlobalUniformSet)
    {
        applyCurrentProperties();
    }

    *postEventAction = REDRAW;
}




} // namespace snip

