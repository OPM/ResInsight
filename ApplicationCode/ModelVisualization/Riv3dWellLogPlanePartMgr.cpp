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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::append3dWellLogCurvesToModel(cvf::ModelBasicList*              model,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    std::vector<Rim3dWellLogCurve*> rim3dWellLogCurves = m_wellPath->vectorOf3dWellLogCurves();
    if (rim3dWellLogCurves.empty()) return;
    if (m_wellPath.isNull()) return;

    if (m_3dWellLogCurveGeometryGenerator.isNull())
    {
        m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator(m_wellPath.p(), m_gridView);
    }

    for (Rim3dWellLogCurve* rim3dWellLogCurve : rim3dWellLogCurves)
    {
        if (!rim3dWellLogCurve->toggleState()) continue;

        cvf::ref<cvf::Drawable> curveDrawable =
            m_3dWellLogCurveGeometryGenerator->createCurveLine(displayCoordTransform, wellPathClipBoundingBox, rim3dWellLogCurve);

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
void Riv3dWellLogPlanePartMgr::appendGridToModel(cvf::ModelBasicList*              model,
                                                 const caf::DisplayCoordTransform* displayCoordTransform,
                                                 const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                 double                            gridIntervalSize)
{
    if (m_3dWellLogCurveGeometryGenerator.isNull())
    {
        m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator(m_wellPath.p(), m_gridView);
    }

    caf::MeshEffectGenerator meshEffectGen(cvf::Color3f(0.4f, 0.4f, 0.4f));

    {
        cvf::ref<cvf::Drawable> gridHorizontalDrawable = m_3dWellLogCurveGeometryGenerator->createGrid(
            displayCoordTransform, wellPathClipBoundingBox, Rim3dWellLogCurve::HORIZONTAL_LEFT, gridIntervalSize);

        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();
        cvf::ref<cvf::Part>   part   = createPart(gridHorizontalDrawable.p(), effect.p());

        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }
    {
        cvf::ref<cvf::Drawable> gridHorizontalDrawable = m_3dWellLogCurveGeometryGenerator->createGrid(
            displayCoordTransform, wellPathClipBoundingBox, Rim3dWellLogCurve::HORIZONTAL_RIGHT, gridIntervalSize);

        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();
        cvf::ref<cvf::Part>   part   = createPart(gridHorizontalDrawable.p(), effect.p());

        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }
}
