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
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "snipSingleTexturedQuad.h"

#include "cvfuSampleFactory.h"
#include "cvfuImageJpeg.h"


namespace snip {

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool SingleTexturedQuad::onInitialize()
{
    m_renderSequence->removeAllRenderings();

    ref<Scene> scene = new Scene;
    ref<Rendering> logoRendering = new Rendering;
    logoRendering->setScene(scene.p());
    //logoRendering->setCamera(m_camera.p());
    m_renderSequence->addRendering(logoRendering.p());
    logoRendering->camera()->viewport()->setClearColor(Color4f(Color3::WHITE));

    // Part with static texture
    ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "CeetronLogoDiffuse.jpg");
    ref<Texture> texture = new Texture(img.p());
    texture->enableMipmapGeneration(true);

    // Use a common sampler for both textures
    ref<Sampler> sampler = new Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::LINEAR_MIPMAP_LINEAR);
    sampler->setMagFilter(cvf::Sampler::LINEAR);

    float aspectRatio = static_cast<float>(img->width())/static_cast<float>(img->height());
    ref<Part> staticTexturePart = SampleFactory::createTexturedQuad(texture.p(), sampler.p(), aspectRatio);
    ref<ModelBasicList> geometryModel = new ModelBasicList;
    geometryModel->addPart(staticTexturePart.p());
    geometryModel->updateBoundingBoxesRecursive();
    scene->addModel(geometryModel.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SingleTexturedQuad::onResizeEvent(int width, int height)
{
    m_renderSequence->firstRendering()->camera()->setViewport(0, 0, width, height);
    BoundingBox bb = m_renderSequence->firstRendering()->scene()->boundingBox();
    if (bb.isValid())
    {
        m_renderSequence->firstRendering()->camera()->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    TestSnippet::onResizeEvent(width, height);
}

} // namespace snip
