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

#include "Rim3dView.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimWellPath.h"

#include "Riv3dWellLogCurveGeomertyGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfBoundingBox.h"
#include "cvfColor3.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogPlanePartMgr::Riv3dWellLogPlanePartMgr(RimWellPath* wellPath, RimGridView* gridView)
    : m_wellPath(wellPath)
    , m_gridView(gridView)
{
    CVF_ASSERT(m_wellPath.notNull());
    m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator(m_wellPath.p());
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

    if (m_wellPath->rim3dWellLogCurveCollection()->isShowingGrid())
    {
        std::set<Rim3dWellLogCurve::DrawPlane> drawPlanes;

        for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
        {
            drawPlanes.insert(rim3dWellLogCurve->drawPlane());
        }

        for (const Rim3dWellLogCurve::DrawPlane& drawPlane : drawPlanes)
        {
            appendGridToModel(model, displayCoordTransform, wellPathClipBoundingBox, drawPlane, planeWidth());
        }
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
    
    for (Rim3dWellLogCurve* rim3dWellLogCurve : m_wellPath->rim3dWellLogCurveCollection()->vectorOf3dWellLogCurves())
    {
        if (!rim3dWellLogCurve->isShowingCurve()) continue;

        std::vector<double> resultValues;
        std::vector<double> resultMds;
        rim3dWellLogCurve->curveValuesAndMds(&resultValues, &resultMds);

        cvf::ref<cvf::Drawable> curveDrawable = m_3dWellLogCurveGeometryGenerator->createCurveLine(
            displayCoordTransform, wellPathClipBoundingBox, resultValues, resultMds, planeAngle(rim3dWellLogCurve->drawPlane()), wellPathCenterToPlotStartOffset(), planeWidth());

        if (curveDrawable.isNull() || !curveDrawable->boundingBox().isValid())
        {
            continue;
        }

        caf::MeshEffectGenerator meshEffectGen(cvf::Color3f(0.9f, 0.0f, 0.0f));
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
            return cvf::PI_D;
        case Rim3dWellLogCurve::HORIZONTAL_RIGHT:
            return 0.0;
        case Rim3dWellLogCurve::VERTICAL_ABOVE:
            return cvf::PI_D / 2.0;
        case Rim3dWellLogCurve::VERTICAL_BELOW:
            return -cvf::PI_D / 2.0;
        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::wellPathCenterToPlotStartOffset() const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    return -cellSize * 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogPlanePartMgr::planeWidth() const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    return cellSize * 4;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::appendGridToModel(cvf::ModelBasicList*                model,
                                                 const caf::DisplayCoordTransform*   displayCoordTransform,
                                                 const cvf::BoundingBox&             wellPathClipBoundingBox,
                                                 const Rim3dWellLogCurve::DrawPlane& drawPlane,
                                                 double                              gridIntervalSize)
{
    caf::MeshEffectGenerator meshEffectGen(cvf::Color3f(0.4f, 0.4f, 0.4f));

    cvf::ref<cvf::Drawable> gridHorizontalDrawable = m_3dWellLogCurveGeometryGenerator->createGrid(
        displayCoordTransform, wellPathClipBoundingBox, planeAngle(drawPlane), wellPathCenterToPlotStartOffset(), planeWidth(), gridIntervalSize);

    cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();
    cvf::ref<cvf::Part>   part = createPart(gridHorizontalDrawable.p(), effect.p());

    if (part.notNull())
    {
        model->addPart(part.p());
    }
}
