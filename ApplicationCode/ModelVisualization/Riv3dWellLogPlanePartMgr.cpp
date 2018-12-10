/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "Riv3dWellLogPlanePartMgr.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimWellPath.h"
#include "RiuViewer.h"

#include "Riv3dWellLogCurveGeometryGenerator.h"
#include "Riv3dWellLogDrawSurfaceGenerator.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfBoundingBox.h"
#include "cvfColor3.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfPart.h"
#include "cvfShaderProgram.h"

#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogPlanePartMgr::Riv3dWellLogPlanePartMgr(RimWellPath* wellPath, RimGridView* gridView)
    : m_wellPath(wellPath)
    , m_gridView(gridView)
{
    CVF_ASSERT(m_wellPath.notNull());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::appendPlaneToModel(cvf::ModelBasicList*              model,
                                                  const caf::DisplayCoordTransform* displayCoordTransform,
                                                  const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                  bool                              isStaticResult)
{
    if (m_wellPath.isNull()) return;

    if (!m_wellPath->rim3dWellLogCurveCollection()) return;

    if (!m_wellPath->rim3dWellLogCurveCollection()->isShowingPlot()) return;

    if (m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves().empty()) return;

    if (isStaticResult)
    {
        std::set<Rim3dWellLogCurve::DrawPlane> drawPlanes;
        for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
        {
            if (rim3dWellLogCurve->showInView(m_gridView))
            {
                drawPlanes.insert(rim3dWellLogCurve->drawPlane());
            }
        }
        for (Rim3dWellLogCurve::DrawPlane drawPlane : drawPlanes)
        {
            m_3dWellLogDrawSurfaceGeometryGenerators[drawPlane] = new Riv3dWellLogDrawSurfaceGenerator(m_wellPath.p());
            appendDrawSurfaceToModel(model, displayCoordTransform, wellPathClipBoundingBox, drawPlane, planeWidth());
        }
    }
    for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
    {
        if (rim3dWellLogCurve->showInView(m_gridView) && rim3dWellLogCurve->isShowingTimeDependentResult() != isStaticResult)
        {
            append3dWellLogCurveToModel(model,
                                        displayCoordTransform,
                                        wellPathClipBoundingBox,
                                        rim3dWellLogCurve,
                                        m_3dWellLogDrawSurfaceGeometryGenerators[rim3dWellLogCurve->drawPlane()]->vertices());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::append3dWellLogCurveToModel(cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                           Rim3dWellLogCurve*                rim3dWellLogCurve,
                                                           const std::vector<cvf::Vec3d>&    drawSurfaceVertices)
{
    CVF_ASSERT(rim3dWellLogCurve);

    cvf::ref<Riv3dWellLogCurveGeometryGenerator> generator = rim3dWellLogCurve->geometryGenerator();
    if (generator.isNull())
    {
        generator = new Riv3dWellLogCurveGeometryGenerator(m_wellPath.p());
        rim3dWellLogCurve->setGeometryGenerator(generator.p());
    }

    generator->createCurveDrawables(displayCoordTransform,
                                    wellPathClipBoundingBox,
                                    rim3dWellLogCurve,
                                    wellPathCenterToPlotStartOffset(rim3dWellLogCurve->drawPlane()),
                                    planeWidth(),
                                    drawSurfaceVertices,
                                    m_gridView->currentTimeStep());

    cvf::ref<cvf::DrawableGeo> curveDrawable = generator->curveDrawable();
    if (curveDrawable.notNull() && curveDrawable->boundingBox().isValid())
    {
        caf::MeshEffectGenerator meshEffectGen(rim3dWellLogCurve->color());
        meshEffectGen.setLineWidth(3.0f);
        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(curveDrawable.p());
        part->setEffect(effect.p());

        part->setSourceInfo(new RivMeshLinesSourceInfo(rim3dWellLogCurve));

        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> Riv3dWellLogPlanePartMgr::createPart(cvf::Drawable* drawable, cvf::Effect* effect)
{
    cvf::ref<cvf::Part> part = new cvf::Part;

    if (drawable && drawable->boundingBox().isValid())
    {
        part->setDrawable(drawable);
        part->setEffect(effect);
    }

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::wellPathCenterToPlotStartOffset(Rim3dWellLogCurve::DrawPlane drawPlane) const
{
    if (drawPlane == Rim3dWellLogCurve::HORIZONTAL_CENTER || drawPlane == Rim3dWellLogCurve::VERTICAL_CENTER)
    {
        return -0.5 * planeWidth();
    }
    else
    {
        double cellSize       = m_gridView->ownerCase()->characteristicCellSize();
        double wellPathOffset = std::min(m_wellPath->wellPathRadius(cellSize), 0.1 * planeWidth());
        return m_wellPath->wellPathRadius(cellSize) + wellPathOffset;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::planeWidth() const
{
    if (!m_gridView) return 0;

    double                             cellSize        = m_gridView->ownerCase()->characteristicCellSize();
    const Rim3dWellLogCurveCollection* curveCollection = m_wellPath->rim3dWellLogCurveCollection();
    return cellSize * curveCollection->planeWidthScaling();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::appendDrawSurfaceToModel(cvf::ModelBasicList*              model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                        Rim3dWellLogCurve::DrawPlane      drawPlane,
                                                        double                            samplingInterval)
{
    Rim3dWellLogCurveCollection*  curveCollection = m_wellPath->rim3dWellLogCurveCollection();
    cvf::ref<RivObjectSourceInfo> sourceInfo      = new RivObjectSourceInfo(curveCollection);

    bool showCoordinateSystemMesh = curveCollection->isShowingGrid();
    bool showBackground           = curveCollection->isShowingBackground();

    cvf::Color3f                borderColor(0.4f, 0.4f, 0.4f);
    caf::SurfaceEffectGenerator backgroundEffectGen(cvf::Color4f(1.0, 1.0, 1.0, 1.0), caf::PO_2);
    caf::MeshEffectGenerator    borderEffectGen(borderColor);
    caf::VectorEffectGenerator  curveNormalsEffectGen;
    backgroundEffectGen.enableLighting(false);

    if (!showBackground)
    {
        // Make the background invisible but still present for picking.
        backgroundEffectGen.enableColorMask(false);
        backgroundEffectGen.enableDepthTest(false);
        backgroundEffectGen.enableDepthWrite(false);
    }

    bool drawSurfaceCreated =
        m_3dWellLogDrawSurfaceGeometryGenerators[drawPlane]->createDrawSurface(displayCoordTransform,
                                                                               wellPathClipBoundingBox,
                                                                               Rim3dWellLogCurve::drawPlaneAngle(drawPlane),
                                                                               wellPathCenterToPlotStartOffset(drawPlane),
                                                                               planeWidth(),
                                                                               samplingInterval);
    if (!drawSurfaceCreated) return;

    cvf::ref<cvf::Effect> backgroundEffect   = backgroundEffectGen.generateCachedEffect();
    cvf::ref<cvf::Effect> borderEffect       = borderEffectGen.generateCachedEffect();
    cvf::ref<cvf::Effect> curveNormalsEffect = curveNormalsEffectGen.generateCachedEffect();

    cvf::ref<cvf::DrawableGeo> background = m_3dWellLogDrawSurfaceGeometryGenerators[drawPlane]->background();

    if (background.notNull())
    {
        cvf::ref<cvf::Part> part = createPart(background.p(), backgroundEffect.p());
        if (part.notNull())
        {
            model->addPart(part.p());
            part->setSourceInfo(sourceInfo.p());
        }
    }

    if (showCoordinateSystemMesh)
    {
        cvf::ref<cvf::DrawableGeo> border = m_3dWellLogDrawSurfaceGeometryGenerators[drawPlane]->border();
        if (border.notNull())
        {
            cvf::ref<cvf::Part> part = createPart(border.p(), borderEffect.p());
            if (part.notNull())
            {
                model->addPart(part.p());
            }
        }

        cvf::ref<cvf::DrawableVectors> normals = m_3dWellLogDrawSurfaceGeometryGenerators[drawPlane]->curveNormalVectors();
        if (normals.notNull())
        {
            normals->setSingleColor(borderColor);
            if (RiaApplication::instance()->useShaders())
            {
                normals->setUniformNames("u_transformationMatrix", "u_color");
            }

            cvf::ref<cvf::Part> part = createPart(normals.p(), curveNormalsEffect.p());
            if (part.notNull())
            {
                model->addPart(part.p());
                part->setSourceInfo(sourceInfo.p());
            }
        }
    }
}
