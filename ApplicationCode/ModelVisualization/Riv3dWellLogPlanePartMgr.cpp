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

#include "RigWellPath.h"

#include "Riv3dWellLogCurveGeomertyGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfColor3.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogPlanePartMgr::Riv3dWellLogPlanePartMgr(RigWellPath* wellPathGeometry)
    :m_wellPathGeometry(wellPathGeometry)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogPlanePartMgr::append3dWellLogCurvesToModel(cvf::ModelBasicList*              model,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            std::vector<Rim3dWellLogCurve*>   rim3dWellLogCurves)
{
    if (rim3dWellLogCurves.empty()) return;

    m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator(m_wellPathGeometry.p());

    for (Rim3dWellLogCurve* rim3dWellLogCurve : rim3dWellLogCurves)
    {
        if (!rim3dWellLogCurve->toggleState()) continue;
        
        cvf::ref<cvf::Drawable> curveDrawable = m_3dWellLogCurveGeometryGenerator->createCurveLine(displayCoordTransform, rim3dWellLogCurve);

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(255, 0, 0, 0.5), caf::PO_1);
        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(curveDrawable.p());
        part->setEffect(effect.p());

        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }

    //TODO: Atm, only the grid for the first curve is drawn.
    cvf::ref<cvf::Drawable> gridDrawable = m_3dWellLogCurveGeometryGenerator->createGrid(displayCoordTransform, rim3dWellLogCurves[0]);

    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(255, 255, 0, 1), caf::PO_1);
    cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(gridDrawable.p());
    part->setEffect(effect.p());

    if (part.notNull())
    {
        model->addPart(part.p());
    }
}
