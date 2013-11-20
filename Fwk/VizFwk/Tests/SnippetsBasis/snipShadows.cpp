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
#include "cvfRenderbufferObject.h"

#include "cvfPatchGenerator.h"
#include "cvfGeometryUtils.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfArrowGenerator.h"

#include "cvfuInputEvents.h"
#include "cvfuSampleFactory.h"
#include "cvfuPartCompoundGenerator.h"

#include "snipShadows.h"

namespace snip {

class PointLightShadow : public DynamicUniformSet
{
public:
    PointLightShadow()
    {
        m_position = Vec3d(0, 0, 0);

        m_uniformSet = new UniformSet;
        m_markerTransform = new Transform;
    }

    void setPosition(Vec3d position)
    {
        m_position = position;
    }

    Vec3d position() const
    {
        return m_position;
    }

    virtual void update(Rendering* rendering)
    {
        m_uniformSet->setUniform(new UniformFloat("u_wcLightPosition", Vec3f(m_position)));

        Camera* cam = rendering->camera();
        const Mat4d& vm = cam->viewMatrix();
        Vec3f ecPos = Vec3f(m_position.getTransformedPoint(vm));
        m_uniformSet->setUniform(new UniformFloat("u_ecLightPosition", ecPos));

        Mat4d m;
        m.setTranslation(m_position);
        m_markerTransform->setLocalTransform(m);

    }
    virtual UniformSet*     uniformSet()
    {
        return m_uniformSet.p();
    }

    Transform*              markerTransform()
    {
        return m_markerTransform.p();
    }

private:
    Vec3d                       m_position;
    ref<UniformSet>             m_uniformSet;
    ref<Transform>              m_markerTransform;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Shadows::Shadows()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Shadows::~Shadows()
{
}
    


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Shadows::onInitialize()
{
    if (!m_openGLContext->capabilities()->hasCapability(OpenGLCapabilities::FRAMEBUFFER_OBJECT))
    {
        return false;
    }

    m_renderSequence->removeAllRenderings();

    m_depthTexture = new Texture(Texture::TEXTURE_2D, Texture::DEPTH_COMPONENT24);
    
    m_model = createModel();

    //  Setup first rendering with FBO to Shadow Map
    // -------------------------------------------------------------------------
    ref<Rendering> shadowRendering = new Rendering;
    {
        const int SHADOW_MAP_INIT_WIDTH  = 2048;
        const int SHADOW_MAP_INIT_HEIGHT = 2048;

        ref<Camera> shadowRenderingCamera = new Camera;
        shadowRendering->setCamera(shadowRenderingCamera.p());

        ref<Scene> scene = new Scene;
        scene->addModel(m_model.p());

        shadowRendering->setScene(scene.p());

        m_fbo = new FramebufferObject;
        shadowRendering->setTargetFramebuffer(m_fbo.p());

//         ref<RenderbufferObject> rbo = new RenderbufferObject(RenderbufferObject::RGBA, SHADOW_MAP_INIT_WIDTH, SHADOW_MAP_INIT_HEIGHT);
//         m_fbo->attachColorRenderBuffer(0, rbo.p());

        m_depthTexture->setSize(SHADOW_MAP_INIT_WIDTH, SHADOW_MAP_INIT_HEIGHT);
        m_fbo->attachDepthTexture2d(m_depthTexture.p());

        BoundingBox bb = m_model->boundingBox();
        Vec3d lightDir = Vec3d(-0.14, 0.05, -0.98).getNormalized();
        Vec3d lightUp = Vec3d(-0.52, 0.84, 0.12).getNormalized();

        shadowRenderingCamera->fitView(bb, lightDir, lightUp);
        shadowRenderingCamera->setViewport(0,0,SHADOW_MAP_INIT_WIDTH, SHADOW_MAP_INIT_HEIGHT);

        // A void FS shader as we are drawing only depth
        ShaderProgramGenerator gen("DepthOnly", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_Minimal);
        gen.addFragmentCode(ShaderSourceRepository::fs_Void);
        ref<ShaderProgram> shaderProg = gen.generate();

        ref<Effect> overrideEffect = new Effect;
//         overrideEffect->setShaderProgram(shaderProg.p());

        // Use polygon offset to avoid selfshadowing and z buffer fighting with the shadow map
//         PolygonOffset* po = new PolygonOffset;
//         po->configurePolygonPositiveOffset();
//         po->setFactor(6.0);
//         overrideEffect->setRenderState(po);

        shadowRendering->setEffectOverride(overrideEffect.p());

        m_renderSequence->addRendering(shadowRendering.p());
    }

    // Setup second rendering drawing the texture on the screen
    // -------------------------------------------------------------------------
    {
        ref<Rendering> mainRendering = new Rendering;
        mainRendering->setCamera(m_camera.p());

        ref<Scene> scene = new Scene;
        scene->addModel(m_model.p());

        mainRendering->setScene(scene.p());
        mainRendering->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

        BoundingBox bb = m_model->boundingBox();
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);

        m_renderSequence->addRendering(mainRendering.p());
    }

    // Setup the locator to control the position of the light source
    {
        Vec3d shadowCamInitPos = m_renderSequence->firstRendering()->camera()->position();

        ref<ModelBasicList> markerModel = new ModelBasicList;

        m_light = new PointLightShadow;
        m_light->setPosition(shadowCamInitPos);

        ref<Part> markerPart = createLightMarker(m_openGLContext.p(), 1);
        markerPart->setTransform(m_light->markerTransform());
        markerModel->addPart(markerPart.p());

        m_renderSequence->rendering(1)->scene()->addModel(markerModel.p());

        m_renderSequence->rendering(1)->addGlobalDynamicUniformSet(m_light.p());

        m_locator = new LocatorPanWalkRotate(m_camera.p());
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ModelBasicList> Shadows::createModel()
{
    ref<ModelBasicList> model = new ModelBasicList;

    ShaderProgramGenerator gen("Shadow", ShaderSourceProvider::instance());
    gen.addVertexCode(ShaderSourceRepository::calcShadowCoord);
    gen.addVertexCode(ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(ShaderSourceRepository::src_Color);
    gen.addFragmentCode(ShaderSourceRepository::light_Phong);
    gen.addFragmentCode(ShaderSourceRepository::fs_Shadow_v33);

    ref<ShaderProgram> shaderProg = gen.generate();

    // Link and show log to see any warnings
    shaderProg->linkProgram(m_openGLContext.p());
    if (!shaderProg->programInfoLog(m_openGLContext.p()).isEmpty()) Trace::show(shaderProg->programInfoLog(m_openGLContext.p()));


    // Configure the sampler
    ref<Sampler> sampler = new Sampler;
    sampler->setMinFilter(Sampler::LINEAR);
    //sampler->setMagFilter(Sampler::NEAREST);
    sampler->setMagFilter(Sampler::LINEAR);

    sampler->setWrapModeS(Sampler::REPEAT);
    sampler->setWrapModeT(Sampler::REPEAT);

    // Setup the texture binding render state
    ref<RenderStateTextureBindings> textureBindings = new RenderStateTextureBindings;
    textureBindings->addBinding(m_depthTexture.p(), sampler.p(), "u_shadowMap");

    PartCompoundGenerator partGen;
    partGen.setPartDistribution(Vec3i(3, 3, 3));
    partGen.setNumEffects(8);
    partGen.useRandomEffectAssignment(false);
    partGen.setExtent(Vec3f(3,3,3));
    partGen.setOrigin(Vec3f(2, 5, 3));

    Collection<Part> parts;
    partGen.generateSpheres(20, 20, &parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();
        Effect* eff = part->effect();

        eff->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
        eff->setShaderProgram(shaderProg.p());
        eff->setRenderState(textureBindings.p());

        model->addPart(part);
    }

    {
        PatchGenerator gen;
        gen.setOrigin(Vec3d(0, 0, 0));
        gen.setExtent(10, 10);
        gen.setSubdivisions(10, 10);
        {
            GeometryBuilderDrawableGeo builder;
            gen.generate(&builder);

            ref<DrawableGeo> geo = builder.drawableGeo();
            ref<Part> part = new Part;
            part->setDrawable(geo.p());

            ref<Effect> eff = new Effect;
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
            eff->setUniform(new UniformFloat("u_color", Color4f(0, 1, 0, 1)));
            eff->setRenderState(textureBindings.p());

            part->setEffect(eff.p());
            model->addPart(part.p());
        }
    }

    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(1, 50, 50, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        Mat4f matr;
        matr.setTranslation(Vec3f(5,2,5));
        size_t i;
        for (i = 0; i < vertices->size(); i++)
        {
            Vec3f v = vertices->get(i);
            v.transformPoint(matr);
            vertices->set(i, v);
        }

        ref<DrawableGeo> geo1 = new DrawableGeo;
        geo1->setVertexArray(vertices.p());   
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo1.p());

        ref<Effect> eff1 = new Effect;
        eff1->setShaderProgram(shaderProg.p());
        eff1->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
        eff1->setUniform(new UniformFloat("u_color", Color4f(1, 0, 0, 1)));
        eff1->setRenderState(textureBindings.p());
        
        part->setEffect(eff1.p());

        model->addPart(part.p());
    }

    {
        GeometryBuilderFaceList builder;
        ArrowGenerator gen;
        gen.generate(&builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        Mat4f matr = Mat4f::fromRotation(Vec3f(1,1,0), static_cast<float>(cvf::Math::toRadians(90.0)));
        matr.setTranslation(Vec3f(7,5,2));
        size_t i;
        for (i = 0; i < vertices->size(); i++)
        {
            Vec3f v = vertices->get(i);
            v.transformPoint(matr);
            vertices->set(i, v);
        }

        ref<DrawableGeo> geo1 = new DrawableGeo;
        geo1->setVertexArray(vertices.p());   
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo1.p());

        ref<Effect> eff1 = new Effect;
        eff1->setShaderProgram(shaderProg.p());
        eff1->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
        eff1->setUniform(new UniformFloat("u_color", Color4f(1, 1, 1, 1)));
        eff1->setRenderState(textureBindings.p());

        part->setEffect(eff1.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();

    return model;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Shadows::onPaintEvent(PostEventAction* postEventAction)
{
    // Set light uniform in all parts
    ref<Rendering> shadowRendering = m_renderSequence->firstRendering();

    Mat4f lightProjMat = Mat4f(shadowRendering->camera()->projectionMatrix());
    Mat4f lightViewMat = Mat4f(shadowRendering->camera()->viewMatrix());
    Mat4f bias = Mat4f( 0.5, 0.0, 0.0, 0.5, 
                        0.0, 0.5, 0.0, 0.5,
                        0.0, 0.0, 0.5, 0.5,
                        0.0, 0.0, 0.0, 1.0);

    Mat4f cameraInvMat = Mat4f(m_camera->invertedViewMatrix());

    Mat4f shadowMat = bias*lightProjMat*lightViewMat*cameraInvMat;

    ref<UniformMatrixf> lightViewProjMat = new UniformMatrixf("u_lightViewProjectionMatrix", shadowMat);
    size_t i;
    for (i = 0; i < m_model->partCount(); i++)
    {
        Part* part = m_model->part(i);
        CVF_ASSERT(part);

        part->effect()->setUniform(lightViewProjMat.p());
    }

    TestSnippet::onPaintEvent(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Shadows::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (m_locator.notNull() && m_light.notNull())
    {
        if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier))
        {
            int x = mouseEvent->x();
            int y = mouseEvent->y();

            if (mouseEvent->buttons() == MiddleButton || 
                mouseEvent->buttons() == (LeftButton | RightButton))
            {
                m_locator->setOperation(LocatorPanWalkRotate::WALK);
            }
            else
            {
                m_locator->setOperation(LocatorPanWalkRotate::PAN);
            }

            m_locator->setPosition(m_light->position());
            m_locator->start(x, y);

            mouseEvent->setRequestedAction(REDRAW);

            return;
        }
    }

    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Shadows::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    if (m_locator.notNull() && m_light.notNull())
    {
        if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier))
        {
            int x = mouseEvent->x();
            int y = mouseEvent->y();
            m_locator->update(x, y);
            m_light->setPosition(m_locator->position());

            // Update the position of the light cam also
            ref<Rendering> shadowRendering = m_renderSequence->firstRendering();
            Camera* cam = shadowRendering->camera();
            cam->setFromLookAt(m_locator->position(), m_model->boundingBox().center(), cam->up());

            mouseEvent->setRequestedAction(REDRAW);
            return;
        }
    }

    TestSnippet::onMouseMoveEvent(mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Shadows::createLightMarker(OpenGLContext* oglContext, double size)
{
    GeometryBuilderDrawableGeo builder;
    GeometryUtils::createSphere(size/2.0, 20, 20, &builder);
    ref<DrawableGeo> geo = builder.drawableGeo();
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());

    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();

    prog->linkProgram(oglContext);
    if (!prog->programInfoLog(oglContext).isEmpty()) Trace::show(prog->programInfoLog(oglContext));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW)));

    part->setEffect(eff.p());
    part->updateBoundingBox();

    return part.p();
}

} // namespace snip
