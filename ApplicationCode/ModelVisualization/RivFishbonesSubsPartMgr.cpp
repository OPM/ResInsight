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

#include "RivFishbonesSubsPartMgr.h"

#include "RigWellPath.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"

#include "RivObjectSourceInfo.h"
#include "RivPipeGeometryGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::RivFishbonesSubsPartMgr(RimFishbonesMultipleSubs* subs)
    : m_rimFishbonesSubs(subs)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::~RivFishbonesSubsPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::appendGeometryPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    clearGeometryCache();

    if (!m_rimFishbonesSubs->isChecked()) return;

    if (m_parts.size() == 0)
    {
        buildParts(displayCoordTransform, characteristicCellSize);
    }

    for (auto part : m_parts)
    {
        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::clearGeometryCache()
{
    m_parts.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::buildParts(caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    RimWellPath* wellPath = nullptr;
    m_rimFishbonesSubs->firstAncestorOrThisOfTypeAsserted(wellPath);

    RivPipeGeometryGenerator geoGenerator;
    geoGenerator.setRadius(m_rimFishbonesSubs->tubingRadius());

    for (size_t subIndex = 0; subIndex < m_rimFishbonesSubs->locationOfSubs().size(); subIndex++)
    {
        for (size_t lateralIndex = 0; lateralIndex < m_rimFishbonesSubs->lateralLengths().size(); lateralIndex++)
        {
            std::vector<cvf::Vec3d> lateralDomainCoords = m_rimFishbonesSubs->coordsForLateral(subIndex, lateralIndex);

            std::vector<cvf::Vec3d> displayCoords;
            for (auto domainCoord : lateralDomainCoords)
            {
                displayCoords.push_back(displayCoordTransform->transformToDisplayCoord(domainCoord));
            }

            cylinderWithCenterLineParts(&m_parts, displayCoords, wellPath->wellPathColor(), wellPath->combinedScaleFactor() * characteristicCellSize * 0.5);

            cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(m_rimFishbonesSubs);

            for (auto p : m_parts)
            {
                p->setSourceInfo(objectSourceInfo.p());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::cylinderWithCenterLineParts(cvf::Collection<cvf::Part>* destinationParts, const std::vector<cvf::Vec3d>& centerCoords, const cvf::Color3f& color, double radius)
{
    cvf::ref<RivPipeGeometryGenerator> geoGenerator = new RivPipeGeometryGenerator;
    geoGenerator->setRadius(radius);
    geoGenerator->setCrossSectionVertexCount(12);

    cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray(centerCoords);
    geoGenerator->setPipeCenterCoords(cvfCoords.p());

    cvf::ref<cvf::DrawableGeo> surfaceGeo = geoGenerator->createPipeSurface();
    if (surfaceGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(surfaceGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }

    cvf::ref<cvf::DrawableGeo> centerLineGeo = geoGenerator->createCenterLine();
    if (centerLineGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(centerLineGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }
}

