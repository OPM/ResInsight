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

#include "RiaColorTables.h"

#include "Rim3dView.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimWellPath.h"

#include "Riv3dWellLogCurveGeomertyGenerator.h"
#include "Riv3dWellLogGridGeomertyGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfBoundingBox.h"
#include "cvfColor3.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogPlanePartMgr::Riv3dWellLogPlanePartMgr(RimWellPath* wellPath, RimGridView* gridView)
    : m_wellPath(wellPath)
    , m_gridView(gridView)
{
    CVF_ASSERT(m_wellPath.notNull());
    m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator(m_wellPath.p());
    m_3dWellLogGridGeometryGenerator = new Riv3dWellLogGridGeometryGenerator(m_wellPath.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::appendPlaneToModel(cvf::ModelBasicList*              model,
                                                  const caf::DisplayCoordTransform* displayCoordTransform,
                                                  const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    if (m_wellPath.isNull()) return;

    if (!m_wellPath->rim3dWellLogCurveCollection()) return;

    if (!m_wellPath->rim3dWellLogCurveCollection()->isShowingPlot()) return;

    if (m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves().empty()) return;

    append3dWellLogCurvesToModel(model, displayCoordTransform, wellPathClipBoundingBox);

    for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
    {
        appendGridToModel(model, displayCoordTransform, wellPathClipBoundingBox, rim3dWellLogCurve, planeWidth());        
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::append3dWellLogCurvesToModel(cvf::ModelBasicList*              model,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    if (m_wellPath.isNull()) return;
    if (!m_wellPath->rim3dWellLogCurveCollection()) return;
    if (m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves().empty()) return;

    const Rim3dWellLogCurveCollection*         curveCollection = m_wellPath->rim3dWellLogCurveCollection();
    Rim3dWellLogCurveCollection::PlanePosition planePosition   = curveCollection->planePosition();

    size_t colorIndex = 0;

    for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
    {
        colorIndex++;
        if (!rim3dWellLogCurve->isShowingCurve()) continue;

        std::vector<double> resultValues;
        std::vector<double> resultMds;
        rim3dWellLogCurve->curveValuesAndMds(&resultValues, &resultMds);

        cvf::ref<cvf::Drawable> curveDrawable =
            m_3dWellLogCurveGeometryGenerator->createCurveLine(displayCoordTransform,
                                                               wellPathClipBoundingBox,
                                                               resultValues,
                                                               resultMds,
                                                               planeAngle(rim3dWellLogCurve->drawPlane()),
                                                               wellPathCenterToPlotStartOffset(planePosition),
                                                               planeWidth());

        if (curveDrawable.isNull() || !curveDrawable->boundingBox().isValid())
        {
            continue;
        }

        caf::MeshEffectGenerator meshEffectGen(rim3dWellLogCurve->color());
        meshEffectGen.setLineWidth(2.0f);
        cvf::ref<cvf::Effect>    effect = meshEffectGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(curveDrawable.p());
        part->setEffect(effect.p());

        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }
}

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
double Riv3dWellLogPlanePartMgr::planeAngle(const Rim3dWellLogCurve::DrawPlane& drawPlane)
{
    switch (drawPlane)
    {
        case Rim3dWellLogCurve::HORIZONTAL_LEFT:
            return cvf::PI_D / 2.0;
        case Rim3dWellLogCurve::HORIZONTAL_RIGHT:
            return -cvf::PI_D / 2.0;
        case Rim3dWellLogCurve::VERTICAL_ABOVE:
            return 0.0;
        case Rim3dWellLogCurve::VERTICAL_BELOW:
            return cvf::PI_D;
        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::wellPathCenterToPlotStartOffset(Rim3dWellLogCurveCollection::PlanePosition planePosition) const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    if (planePosition == Rim3dWellLogCurveCollection::ALONG_WELLPATH)
    {
        return m_wellPath->wellPathRadius(cellSize) * 2;
    }
    else
    {
        return -0.5*planeWidth();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::planeWidth() const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    return cellSize * 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::appendGridToModel(cvf::ModelBasicList*                model,
                                                 const caf::DisplayCoordTransform*   displayCoordTransform,
                                                 const cvf::BoundingBox&             wellPathClipBoundingBox,
                                                 const Rim3dWellLogCurve*            rim3dWellLogCurve,
                                                 double                              gridIntervalSize)
{
    const Rim3dWellLogCurveCollection*         curveCollection = m_wellPath->rim3dWellLogCurveCollection();
    Rim3dWellLogCurveCollection::PlanePosition planePosition = curveCollection->planePosition();
    bool                                       showGrid = curveCollection->isShowingGrid();
    bool                                       showBackground = curveCollection->isShowingBackground();

    caf::SurfaceEffectGenerator backgroundEffectGen(cvf::Color4f(1.0, 1.0, 1.0, 1.0), caf::PO_2);
    caf::MeshEffectGenerator    gridBorderEffectGen(cvf::Color3f(0.4f, 0.4f, 0.4f));
    caf::MeshEffectGenerator    curveNormalsEffectGen(cvf::Color3f(0.4f, 0.4f, 0.4f));
    backgroundEffectGen.enableLighting(false);

    bool gridCreated = m_3dWellLogGridGeometryGenerator->createGrid(displayCoordTransform,
        wellPathClipBoundingBox,
        planeAngle(rim3dWellLogCurve->drawPlane()),
        wellPathCenterToPlotStartOffset(planePosition),
        planeWidth(),
        gridIntervalSize);
    if (!gridCreated) return;

    cvf::ref<cvf::Effect> backgroundEffect = backgroundEffectGen.generateCachedEffect();
    cvf::ref<cvf::Effect> borderEffect = gridBorderEffectGen.generateCachedEffect();
    cvf::ref<cvf::Effect> curveNormalsEffect = curveNormalsEffectGen.generateCachedEffect();

    cvf::ref<cvf::DrawableGeo> background = m_3dWellLogGridGeometryGenerator->background();
    if (showBackground && background.notNull())
    {
        cvf::ref<cvf::Part> part = createPart(background.p(), backgroundEffect.p());
        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }

    if (showGrid) {
        cvf::ref<cvf::DrawableGeo> border = m_3dWellLogGridGeometryGenerator->border();
        if (border.notNull())
        {
            cvf::ref<cvf::Part> part = createPart(border.p(), borderEffect.p());
            if (part.notNull())
            {
                model->addPart(part.p());
            }
        }

        cvf::ref<cvf::DrawableGeo> normals = m_3dWellLogGridGeometryGenerator->curveNormalLines();
        if (normals.notNull())
        {
            cvf::ref<cvf::Part> part = createPart(normals.p(), curveNormalsEffect.p());
            if (part.notNull())
            {
                model->addPart(part.p());
            }
        }
    }
}
