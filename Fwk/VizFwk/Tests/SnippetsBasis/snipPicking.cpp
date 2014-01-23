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
#include "cvfLibGeometry.h"

#include "snipPicking.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Picking::Picking()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Picking::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateBoxes(&parts);

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    myModel->updateBoundingBoxesRecursive();

    Rendering* mainRendering = m_renderSequence->rendering(0);
    mainRendering->scene()->addModel(myModel.p());
    
    BoundingBox bb = myModel->boundingBox();
    m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);

    
    m_auxModel = new ModelBasicList;
    m_facesModel = new ModelBasicList;

    ref<Scene> extraScene = new Scene;
    extraScene->addModel(m_auxModel.p());
    extraScene->addModel(m_facesModel.p());

    ref<Rendering> extraRendering = new Rendering;
    extraRendering->setScene(extraScene.p());
    extraRendering->setCamera(mainRendering->camera());
    extraRendering->setClearMode(Viewport::DO_NOT_CLEAR);

    m_renderSequence->addRendering(extraRendering.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (buttonPressed == LeftButton && mouseEvent->modifiers() == ControlModifier)
    {
        Rendering* mainRendering = m_renderSequence->rendering(0);

        ref<cvf::RayIntersectSpec> ris = mainRendering->rayIntersectSpecFromWindowCoordinates(mouseEvent->x(), mouseEvent->y());

        DebugTimer tim("Picking");
        cvf::HitItemCollection hic;
        mainRendering->rayIntersect(*ris, &hic);
        tim.reportTime();

        if (hic.count() > 0)
        {
            const cvf::HitItem* item = hic.firstItem();
            CVF_ASSERT(item && item->part());

            Vec3d isectPt = item->intersectionPoint();
            Trace::show("IntersectPt: %f %f %f", isectPt.x(), isectPt.y(), isectPt.z());
            updateHitPointMarkerPos(item->intersectionPoint());

            const HitDetail* detail = item->detail();
            const HitDetailDrawableGeo* detailGeo = dynamic_cast<const HitDetailDrawableGeo*>(detail);
            if (detailGeo)
            {
                cvf::Trace::show("face idx %d", detailGeo->faceIndex());
            }

            repickAndHighlightAllFaces(*ris->ray(), hic);

            // Be good and set rotation point
            m_trackball->setRotationPoint(isectPt);
        }
        else
        {
            removeHitPointMarker();
        }

        mouseEvent->setRequestedAction(REDRAW);

        return;
    }

    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::onKeyPressEvent(KeyEvent* keyEvent)
{
    char keyChar = keyEvent->character();

    if (keyChar == 'l')
    {
        drawMainRenderingUsingLines(true);
        Trace::show("Draw using line drawing ON");
    }
    if (keyChar == 'L')
    {
        drawMainRenderingUsingLines(false);
        Trace::show("Draw using line drawing OFF");
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// '
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> Picking::helpText() const
{
    std::vector<String> help;
    help.push_back(String("l/ - toggle drawing using line drawing"));

    return help;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::drawMainRenderingUsingLines(bool drawUsingLines)
{
    Collection<Part> allParts;
    m_renderSequence->firstRendering()->scene()->allParts(&allParts);

    size_t i;
    for (i = 0; i < allParts.size(); i++)
    {
        Part* part = allParts.at(i);
        Effect* eff = part->effect();

        if (drawUsingLines)
        {
            eff->setRenderState(new RenderStatePolygonMode(RenderStatePolygonMode::LINE));
        }
        else
        {
            eff->removeRenderState(RenderState::POLYGON_MODE);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::updateHitPointMarkerPos(const Vec3d& hitPoint)
{
    CVF_ASSERT(m_auxModel.notNull());

    const int hitPointPartId = 101;
    ref<Part> hitPointPart = m_auxModel->findPartByID(hitPointPartId);
    if (hitPointPart.isNull())
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(0.05, 10, 10, &builder);
        ref<DrawableGeo> geo = new DrawableGeo;
        geo->setVertexArray(builder.vertices().p());
        geo->setFromFaceList(*builder.faceList());
        geo->computeNormals();

        ref<Effect> eff = new Effect;
        eff->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_MAGENTA));

        hitPointPart = new Part;
        hitPointPart->setId(hitPointPartId);
        hitPointPart->setDrawable(geo.p());
        hitPointPart->setEffect(eff.p());
        hitPointPart->setTransform(new Transform);

        m_auxModel->addPart(hitPointPart.p());

        m_auxModel->updateBoundingBoxesRecursive();
    }

    ref<Transform> trans = hitPointPart->transform();
    trans->setLocalTransform(Mat4d::fromTranslation(hitPoint));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::removeHitPointMarker()
{
    CVF_ASSERT(m_auxModel.notNull());

    const int hitPointPartId = 101;
    ref<Part> hitPointPart = m_auxModel->findPartByID(hitPointPartId);
    if (hitPointPart.notNull())
    {
        m_auxModel->removePart(hitPointPart.p());
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Picking::repickAndHighlightAllFaces(const Ray& rayIn, const HitItemCollection& hic)
{
    m_facesModel->removeAllParts();

    ref<Effect> eff = new Effect;
    eff->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_CYAN));

    cvf::uint i;
    for (i = 0; i < hic.count(); i++)
    {
        const HitItem* item = hic.item(i);
        const Part* part = item->part();
        const DrawableGeo* geo = dynamic_cast<const DrawableGeo*>(part->drawable());
        if (geo)
        {
            Ray ray(rayIn);

            const Transform* partTransform = part->transform();
            if (partTransform)
            {
                ray.transform(partTransform->worldTransform().getInverted());
            }

            UIntArray faceIndices;
            if (geo->rayIntersect(ray, NULL, &faceIndices))
            {
                ref<DrawableGeo> facesGeo = extractFacesFromGeo(*geo, faceIndices);

                if (partTransform)
                {
                    facesGeo->transform(partTransform->worldTransform());
                }

                ref<Part> facesPart = new Part;
                facesPart->setDrawable(facesGeo.p());
                facesPart->setEffect(eff.p());

                m_facesModel->addPart(facesPart.p());
            }
        }
    }

    m_facesModel->updateBoundingBoxesRecursive();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> Picking::extractFacesFromGeo(const DrawableGeo& geo, const UIntArray& faces)
{
    GeometryBuilderFaceList builder;

    size_t i;
    for (i = 0; i < faces.size(); i++)
    {
        size_t faceIndex = faces[i];
        UIntArray conn;
        geo.getFaceIndices(faceIndex, &conn);

        builder.addFace(conn);
    }

    ref<Vec3fArray> verts = new Vec3fArray(*geo.vertexArray());
    ref<UIntArray> faceList = builder.faceList();

    ref<DrawableGeo> faceGeo = new DrawableGeo;
    faceGeo->setVertexArray(verts.p());
    faceGeo->setFromFaceList(*faceList);
    faceGeo->computeNormals();

    return faceGeo;
}


} // namespace snip

