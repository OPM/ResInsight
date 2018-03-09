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
                                                            std::vector<Rim3dWellLogCurve*>*  rim3dWellLogCurves)
{
    if (rim3dWellLogCurves->empty()) return;

    m_3dWellLogCurveGeometryGenerator = new Riv3dWellLogCurveGeometryGenerator;

    std::vector<cvf::Vec3d> wellPathPoints = m_wellPathGeometry->m_wellPathPoints;

    std::vector<cvf::Vec3f> vertices;
    vertices.resize(wellPathPoints.size());

    cvf::Mat4d transMat;
    transMat.setTranslation(cvf::Vec3d(0, 0, 10));

    for (size_t i = 0; i < wellPathPoints.size(); i++)
    {
        wellPathPoints[i].transformPoint(transMat);
        vertices[i] = cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i]));
    }

    std::vector<cvf::uint> indices;
    indices.resize((vertices.size() - 1) * 2);

    cvf::uint counter = 0;
    for (size_t i = 0; i < indices.size(); i++)
    {
        indices[i] = counter;
        if (i % 2 == 0) counter++;
    }

    for (Rim3dWellLogCurve* rim3dWellLogCurve : *rim3dWellLogCurves)
    {
        cvf::ref<cvf::Drawable> drawable = m_3dWellLogCurveGeometryGenerator->createDrawable(vertices, indices);

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(255, 0, 0, 0.5), caf::PO_1);
        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());
        part->setEffect(effect.p());

        if (part.notNull())
        {
            model->addPart(part.p());
        }
    }
}
