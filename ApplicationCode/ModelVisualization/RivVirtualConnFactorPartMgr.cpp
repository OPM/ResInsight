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
#include "RiaExtractionTools.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"

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

    std::vector<WellPathCellIntersectionInfo> wellPathCellIntersections;
    {
        RigEclipseWellLogExtractor* extractor = RiaExtractionTools::wellLogExtractorEclipseCase(m_rimWell, eclipseCase);
        if (extractor)
        {
            wellPathCellIntersections = extractor->cellIntersectionInfosAlongWellPath();
        }
    }

    std::vector<CompletionVizData> centerColorPairs;
    for (const auto& cell : conn)
    {
        size_t gridIndex = cell.first.globalCellIndex();

        const RigCell& rigCell = mainGrid->cell(gridIndex);

        cvf::Vec3d locationInDomainCoord = rigCell.center();
        cvf::Vec3d wellPathDirection     = cvf::Vec3d::X_AXIS;

        if (!wellPathCellIntersections.empty())
        {
            for (const auto& intersectionInfo : wellPathCellIntersections)
            {
                if (intersectionInfo.globCellIndex == cell.first.globalCellIndex())
                {
                    double startMD = intersectionInfo.startMD;
                    double endMD   = intersectionInfo.endMD;

                    double middleMD = (startMD + endMD) / 2.0;

                    locationInDomainCoord = m_rimWell->wellPathGeometry()->interpolatedPointAlongWellPath(middleMD);

                    cvf::Vec3d p1;
                    cvf::Vec3d p2;
                    m_rimWell->wellPathGeometry()->twoClosestPoints(locationInDomainCoord, &p1, &p2);

                    wellPathDirection = (p2 - p1).getNormalized();

                    continue;
                }
            }
        }

        cvf::Vec3d displayCoord = coordTransform->transformToDisplayCoord(locationInDomainCoord);

        double transmissibility = HUGE_VAL;
        if (!cell.second.empty())
        {
            transmissibility = cell.second.front().transmissibility();
        }

        centerColorPairs.push_back(CompletionVizData(displayCoord, wellPathDirection, transmissibility));
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
cvf::ref<cvf::Part> RivVirtualConnFactorPartMgr::createPart(std::vector<CompletionVizData>& vizDataItems,
                                                            double                          radius,
                                                            cvf::ScalarMapper*              scalarMapper)
{
    std::vector<cvf::Vec3f> verticesForOneObject;
    std::vector<cvf::uint>  indicesForOneObject;

    RivVirtualConnFactorPartMgr::createStarGeometry(&verticesForOneObject, &indicesForOneObject, radius, radius * 0.3);

    cvf::ref<cvf::Vec3fArray> vertices      = new cvf::Vec3fArray;
    cvf::ref<cvf::UIntArray>  indices       = new cvf::UIntArray;
    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray();

    auto indexCount  = vizDataItems.size() * indicesForOneObject.size();
    auto vertexCount = vizDataItems.size() * verticesForOneObject.size();
    indices->reserve(indexCount);
    vertices->reserve(vertexCount);
    textureCoords->reserve(vertexCount);

    textureCoords->setAll(cvf::Vec2f(0.5f, 1.0f));

    for (const auto& item : vizDataItems)
    {
        auto rotMatrix = rotationMatrixBetweenVectors(cvf::Vec3d::Y_AXIS, item.m_direction);

        cvf::uint indexOffset = static_cast<cvf::uint>(vertices->size());

        for (const auto& v : verticesForOneObject)
        {
            auto rotatedPoint = v.getTransformedPoint(rotMatrix);

            vertices->add(cvf::Vec3f(item.m_anchor) + rotatedPoint);

            if (item.m_connectionFactor == HUGE_VAL)
            {
                textureCoords->add(cvf::Vec2f(0.5f, 1.0f));
            }
            else
            {
                textureCoords->add(scalarMapper->mapToTextureCoord(item.m_connectionFactor));
            }
        }

        for (const auto& i : indicesForOneObject)
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

    bool disableLighting = true;
    effGen.disableLighting(disableLighting);

    cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
    part->setEffect(eff.p());

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivVirtualConnFactorPartMgr::createStarGeometry(std::vector<cvf::Vec3f>* vertices,
                                                     std::vector<cvf::uint>*  indices,
                                                     double                   radius,
                                                     double                   thickness)
{
    auto p0 = cvf::Vec3f::Z_AXIS * radius;
    auto p2 = cvf::Vec3f::X_AXIS * -radius;
    auto p4 = -p0;
    auto p6 = -p2;

    double innerFactor = 5.0;

    auto p1 = (p0 + p2) / innerFactor;
    auto p3 = (p2 + p4) / innerFactor;

    auto p5 = -p1;
    auto p7 = -p3;

    auto p8 = cvf::Vec3f::Y_AXIS * thickness;
    auto p9 = -p8;

    vertices->push_back(p0);
    vertices->push_back(p1);
    vertices->push_back(p2);
    vertices->push_back(p3);
    vertices->push_back(p4);
    vertices->push_back(p5);
    vertices->push_back(p6);
    vertices->push_back(p7);
    vertices->push_back(p8);
    vertices->push_back(p9);

    // Top
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(8);

    indices->push_back(0);
    indices->push_back(8);
    indices->push_back(7);

    indices->push_back(0);
    indices->push_back(9);
    indices->push_back(1);

    indices->push_back(0);
    indices->push_back(7);
    indices->push_back(9);

    // Left
    indices->push_back(2);
    indices->push_back(3);
    indices->push_back(8);

    indices->push_back(2);
    indices->push_back(8);
    indices->push_back(1);

    indices->push_back(2);
    indices->push_back(9);
    indices->push_back(3);

    indices->push_back(2);
    indices->push_back(1);
    indices->push_back(9);

    // Bottom
    indices->push_back(4);
    indices->push_back(5);
    indices->push_back(8);

    indices->push_back(4);
    indices->push_back(8);
    indices->push_back(3);

    indices->push_back(4);
    indices->push_back(9);
    indices->push_back(5);

    indices->push_back(4);
    indices->push_back(3);
    indices->push_back(9);

    // Right
    indices->push_back(6);
    indices->push_back(7);
    indices->push_back(8);

    indices->push_back(6);
    indices->push_back(8);
    indices->push_back(5);

    indices->push_back(6);
    indices->push_back(9);
    indices->push_back(7);

    indices->push_back(6);
    indices->push_back(5);
    indices->push_back(9);
}

//--------------------------------------------------------------------------------------------------
/// Taken from OverlayNavigationCube::computeNewUpVector
/// Consider move to geometry util class
//--------------------------------------------------------------------------------------------------
cvf::Mat4f RivVirtualConnFactorPartMgr::rotationMatrixBetweenVectors(const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    using namespace cvf;

    Vec3d rotAxis = v1 ^ v2;
    rotAxis.normalize();

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp(v1*v2, -1.0, 1.0);
    const double angle = Math::acos(dotProduct);
    Mat4d rotMat = Mat4d::fromRotation(rotAxis, angle);

    Mat4f myMat(rotMat);

    return myMat;
}
