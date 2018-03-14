/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RivVirtualConnFactorPartMgr.h"

#include "RiaApplication.h"

#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimVirtualPerforationResults.h"
#include "RimWellPath.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfObject.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivVirtualConnFactorPartMgr::RivVirtualConnFactorPartMgr(RimWellPath*                  well,
                                                         RimVirtualPerforationResults* virtualPerforationResult)
    : m_rimWell(well)
    , m_virtualPerforationResult(virtualPerforationResult)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivVirtualConnFactorPartMgr::~RivVirtualConnFactorPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivVirtualConnFactorPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    RimEclipseView* eclView = nullptr;
    m_virtualPerforationResult->firstAncestorOrThisOfTypeAsserted(eclView);

    auto coordTransform = eclView->displayCoordTransform();

    RimEclipseCase* eclipseCase = nullptr;
    m_virtualPerforationResult->firstAncestorOrThisOfTypeAsserted(eclipseCase);

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    const RigVirtualPerforationTransmissibilities* trans = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (!trans) return;

    auto conn = trans->multipleCompletionsPerEclipseCell(m_rimWell, frameIndex);

    std::vector<std::pair<cvf::Vec3f, double>> centerColorPairs;

    for (const auto& cell : conn)
    {
        size_t gridIndex = cell.first.globalCellIndex();

        const RigCell& rigCell      = mainGrid->cell(gridIndex);
        cvf::Vec3d     center       = rigCell.center();
        cvf::Vec3d     displayCoord = coordTransform->transformToDisplayCoord(center);

        cvf::Color3f color = cvf::Color3f::BLUE;

        double transmissibility = HUGE_VAL;
        if (!cell.second.empty())
        {
            transmissibility = cell.second.front().transmissibility();
        }

        centerColorPairs.push_back(std::make_pair(cvf::Vec3f(displayCoord), transmissibility));
    }

    if (!centerColorPairs.empty())
    {
        double radius = mainGrid->characteristicIJCellSize() * m_virtualPerforationResult->geometryScaleFactor();

        auto scalarMapper = m_virtualPerforationResult->legendConfig()->scalarMapper();

        cvf::ref<cvf::Part> part = RivVirtualConnFactorPartMgr::createPart(centerColorPairs, radius, scalarMapper);

        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivVirtualConnFactorPartMgr::createPart(std::vector<std::pair<cvf::Vec3f, double>>& centerColorPairs,
                                                            double                                      radius,
                                                            cvf::ScalarMapper*                          scalarMapper)
{
    cvf::GeometryBuilderTriangles builder;
    cvf::GeometryUtils::createSphere(radius, 5, 5, &builder);

    auto sphereVertices = builder.vertices();
    auto sphereIndices  = builder.triangles();

    cvf::ref<cvf::Vec3fArray> vertices      = new cvf::Vec3fArray;
    cvf::ref<cvf::UIntArray>  indices       = new cvf::UIntArray;
    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray();

    auto indexCount  = centerColorPairs.size() * sphereIndices->size();
    auto vertexCount = centerColorPairs.size() * sphereVertices->size();
    indices->reserve(indexCount);
    vertices->reserve(vertexCount);
    textureCoords->reserve(vertexCount);

    textureCoords->setAll(cvf::Vec2f(0.5f, 1.0f));

    for (const auto& centerColorPair : centerColorPairs)
    {
        cvf::uint indexOffset = static_cast<cvf::uint>(vertices->size());

        for (const auto& v : *sphereVertices)
        {
            vertices->add(centerColorPair.first + v);

            if (centerColorPair.second == HUGE_VAL)
            {
                textureCoords->add(cvf::Vec2f(0.5f, 1.0f));
            }
            else
            {
                textureCoords->add(scalarMapper->mapToTextureCoord(centerColorPair.second));
            }
        }

        for (const auto& i : *sphereIndices)
        {
            indices->add(i + indexOffset);
        }
    }

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();
    drawable->setVertexArray(vertices.p());

    drawable->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES, indices.p()));
    drawable->computeNormals();

    drawable->setTextureCoordArray(textureCoords.p());

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(drawable.p());

    caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);
    bool                             disableLighting = false;
    effGen.disableLighting(disableLighting);

    cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
    part->setEffect(eff.p());

    return part;
}
