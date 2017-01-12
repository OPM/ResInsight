/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RivWellFracturesPartMgr.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFracture.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cafDisplayCoordTransform.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturesPartMgr::RivWellFracturesPartMgr(RimEclipseWell* well)
{
    m_rimWell = well;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturesPartMgr::~RivWellFracturesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (!m_rimWell) return;

    std::vector<RimFracture*> fractures;
    m_rimWell->descendantsIncludingThisOfType(fractures);

    for (RimFracture* fracture : fractures)
    {
        if (!fracture->hasValidGeometry())
        {
            fracture->computeGeometry();
        }
    }

    if (fractures.size() > 0)
    {
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(fractures.at(0));
        if (!objHandle) return;

        RimEclipseView* mainView = nullptr;
        objHandle->firstAncestorOrThisOfType(mainView);
        if (!mainView) return;

        cvf::ref<caf::DisplayCoordTransform> transForm = mainView->displayCoordTransform();

        appendFracturePartsToModel(fractures, model, transForm.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturesPartMgr::appendFracturePartsToModel(std::vector<RimFracture*> fractures, cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform)
{
    for (RimFracture* fracture : fractures)
    {
        if (fracture->hasValidGeometry())
        {
            const std::vector<cvf::Vec3f>& nodeCoords = fracture->nodeCoords();
            const std::vector<cvf::uint>& triangleIndices = fracture->triangleIndices();
            std::vector<cvf::Vec3f> displayCoords;
            
            for (int i = 0; i < nodeCoords.size(); i++)
            {
                cvf::Vec3d nodeCoordsDouble = static_cast<cvf::Vec3d>(nodeCoords[i]);
                cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(nodeCoordsDouble);
                displayCoords.push_back(static_cast<cvf::Vec3f>(displayCoordsDouble));
            }

            cvf::ref<cvf::DrawableGeo> geo = createGeo(triangleIndices, displayCoords);

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setDrawable(geo.p());

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN)), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            part->setEffect(eff.p());

            model->addPart(part.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturesPartMgr::createGeo(const std::vector<cvf::uint>& polygonIndices, const std::vector<cvf::Vec3f>& nodeCoords)
{
    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray(polygonIndices);
    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray(nodeCoords);

    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES, indices.p()));
    geo->computeNormals();

    return geo;
}

