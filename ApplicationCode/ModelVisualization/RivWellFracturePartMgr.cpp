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

#include "RivWellFracturePartMgr.h"

#include "RiaApplication.h"

#include "RigFractureGrid.h"
#include "RigMainGrid.h"
#include "RigHexIntersectionTools.h"
#include "RigCellGeometryTools.h"

#include "RigFractureCell.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimLegendConfig.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"

#include "RivFaultGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryTools.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfRenderStateDepth.h"
#include "cvfAssert.h"

#include <array>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr::RivWellFracturePartMgr(RimFracture* fracture)
    : m_rimFracture(fracture)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr::~RivWellFracturePartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createEllipseSurfacePart(const RimEclipseView& activeView)
{
    auto displayCoordTransform = activeView.displayCoordTransform();
    if (displayCoordTransform.isNull()) return nullptr;

    if (m_rimFracture)
    {
        std::vector<cvf::Vec3f> nodeCoords;
        std::vector<cvf::uint> triangleIndices;
        m_rimFracture->triangleGeometry(&triangleIndices, &nodeCoords);

        std::vector<cvf::Vec3f> displayCoords;

        for (size_t i = 0; i < nodeCoords.size(); i++)
        {
            cvf::Vec3d nodeCoordsDouble = static_cast<cvf::Vec3d>(nodeCoords[i]);
            cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(nodeCoordsDouble);
            displayCoords.push_back(static_cast<cvf::Vec3f>(displayCoordsDouble));
        }

        if (triangleIndices.empty() || displayCoords.empty())
        {
            return nullptr;
        }

        cvf::ref<cvf::DrawableGeo> geo = buildDrawableGeoFromTriangles(triangleIndices, displayCoords);
        CVF_ASSERT(geo.notNull());
            
        cvf::ref<cvf::Part> surfacePart = new cvf::Part(0, "FractureSurfacePart_ellipse");
        surfacePart->setDrawable(geo.p());
        surfacePart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));

        cvf::Color4f fractureColor = cvf::Color4f(activeView.stimPlanColors->defaultColor());
        caf::SurfaceEffectGenerator surfaceGen(fractureColor, caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
        surfacePart->setEffect(eff.p());

        return surfacePart;
    }

    return nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createStimPlanColorInterpolatedSurfacePart(const RimEclipseView& activeView)
{
    CVF_ASSERT(m_rimFracture);
    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
    CVF_ASSERT(stimPlanFracTemplate);

    auto displayCoordTransform = activeView.displayCoordTransform();
    if (displayCoordTransform.isNull()) return nullptr;

    // Note that the filtering and result mapping code below couples closely to the triangulation and vertex layout returned by triangleGeometry()
    // If this ever changes, the entire code must be revisited
    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint> triangleIndices;
    m_rimFracture->triangleGeometry(&triangleIndices, &nodeCoords);

    if (triangleIndices.empty() || nodeCoords.empty())
    {
        return nullptr;
    }

    // Transforms the node coordinates for display
    for (size_t i = 0; i < nodeCoords.size(); i++)
    {
        cvf::Vec3d doubleCoord(nodeCoords[i]);
        doubleCoord = displayCoordTransform->transformToDisplayCoord(doubleCoord);
        nodeCoords[i] = cvf::Vec3f(doubleCoord);
    }


    RimLegendConfig* legendConfig = nullptr;
    if (activeView.stimPlanColors() && activeView.stimPlanColors()->isChecked())
    {
        legendConfig = activeView.stimPlanColors()->activeLegend();
    }

    // Show selected result on the surface geometry and filter out triangles that have result values near 0
    if (legendConfig)
    {
        // Construct array with per node result values that correspond to the node coordinates of the triangle mesh
        // Since some time steps don't have result vales, we initialize the array to well known values before populating it
        std::vector<double> perNodeResultValues(nodeCoords.size(), HUGE_VAL);
        {
            size_t idx = 0;
            const std::vector<std::vector<double> > dataToPlot = stimPlanFracTemplate->resultValues(activeView.stimPlanColors->resultName(), activeView.stimPlanColors->unit(), stimPlanFracTemplate->activeTimeStepIndex());
            for (const std::vector<double>& unmirroredDataAtDepth : dataToPlot)
            {
                const std::vector<double> mirroredValuesAtDepth = mirrorDataAtSingleDepth(unmirroredDataAtDepth);
                for (double val : mirroredValuesAtDepth)
                {
                    perNodeResultValues[idx++] = val;
                }
            }
        }
        CVF_ASSERT(perNodeResultValues.size() == nodeCoords.size());


        std::vector<cvf::uint> triIndicesToInclude;
        for (size_t i = 0; i < triangleIndices.size(); i += 6)
        {
            // Include all triangles where at least one of the vertices in the triangle pair has a value above threshold
            bool includeThisTrianglePair = false;
            for (size_t j = 0; j < 6; j++)
            {
                if (perNodeResultValues[triangleIndices[i + j]] > 1e-7)
                {
                    includeThisTrianglePair = true;
                }
            }

            if (includeThisTrianglePair)
            {
                for (size_t j = 0; j < 6; j++)
                {
                    triIndicesToInclude.push_back(triangleIndices[i + j]);
                }
            }
        }

        if (triIndicesToInclude.empty())
        {
            return nullptr;
        }

        cvf::ref<cvf::DrawableGeo> geo = buildDrawableGeoFromTriangles(triIndicesToInclude, nodeCoords);
        cvf::ref<cvf::Part> surfacePart = new cvf::Part(0, "FractureSurfacePart_stimPlan");
        surfacePart->setDrawable(geo.p());
        surfacePart->setPriority(RivPartPriority::PartType::BaseLevel);
        surfacePart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));

        const cvf::ScalarMapper* scalarMapper = legendConfig->scalarMapper();
        CVF_ASSERT(scalarMapper);
        cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray(nodeCoords.size());
        textureCoords->setAll(cvf::Vec2f(0.5f, 1.0f));
        for (size_t i = 0; i < perNodeResultValues.size(); i++)
        {
            const double val = perNodeResultValues[i];
            if (val < HUGE_VAL && val == val)
            {
                textureCoords->set(i, scalarMapper->mapToTextureCoord(val)); 
            }
        }
        geo->setTextureCoordArray(textureCoords.p());

        caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);
        effGen.disableLighting(activeView.isLightingDisabled());
        cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
        surfacePart->setEffect(eff.p());

        return surfacePart;
    }
    else 
    {
        // No result is mapped, show the entire StimPlan surface with default color

        return createSingleColorSurfacePart(triangleIndices, nodeCoords, activeView.stimPlanColors->defaultColor());
    }

    return nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createSingleColorSurfacePart(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords, const cvf::Color3f& color)
{
    cvf::ref<cvf::DrawableGeo> geo = buildDrawableGeoFromTriangles(triangleIndices, nodeCoords);

    cvf::ref<cvf::Part> surfacePart = new cvf::Part(0, "FractureSurfacePart_stimPlan");
    surfacePart->setDrawable(geo.p());
    surfacePart->setPriority(RivPartPriority::PartType::BaseLevel);
    surfacePart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));

    cvf::Color4f fractureColor = cvf::Color4f(color);
    caf::SurfaceEffectGenerator surfaceGen(fractureColor, caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
    surfacePart->setEffect(eff.p());

    return surfacePart;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createStimPlanElementColorSurfacePart(const RimEclipseView& activeView)
{
    CVF_ASSERT(m_rimFracture);
    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
    CVF_ASSERT(stimPlanFracTemplate);

    if (!stimPlanFracTemplate->fractureGrid()) return nullptr;

    auto displayCoordTransform = activeView.displayCoordTransform();
    if (displayCoordTransform.isNull()) return nullptr;

    std::vector<cvf::Vec3f> stimPlanMeshVertices;
    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray;
    const cvf::ScalarMapper* scalarMapper = nullptr;

    {
        std::vector<RigFractureCell> stimPlanCells = stimPlanFracTemplate->fractureGrid()->fractureCells();

        RimLegendConfig* legendConfig = nullptr;
        if (activeView.stimPlanColors() && 
            activeView.stimPlanColors()->isChecked() &&
            activeView.stimPlanColors()->activeLegend())
        {
            legendConfig = activeView.stimPlanColors()->activeLegend();

            scalarMapper = legendConfig->scalarMapper();

            QString resultNameFromColors = activeView.stimPlanColors->resultName();
            QString resultUnitFromColors = activeView.stimPlanColors->unit();

            std::vector<double> prCellResults = stimPlanFracTemplate->fractureGridResults(resultNameFromColors,
                                                                                          resultUnitFromColors,
                                                                                          stimPlanFracTemplate->activeTimeStepIndex());

            textureCoords->reserve(prCellResults.size() * 4);

            for (size_t cIdx = 0; cIdx < stimPlanCells.size(); ++cIdx)
            {
                if (prCellResults[cIdx] > 1e-7)
                {
                    const RigFractureCell& stimPlanCell = stimPlanCells[cIdx];
                    std::vector<cvf::Vec3d> stimPlanCellPolygon = stimPlanCell.getPolygon();
                    for (const cvf::Vec3d& cellCorner : stimPlanCellPolygon)
                    {
                        stimPlanMeshVertices.push_back(static_cast<cvf::Vec3f>(cellCorner));
                        textureCoords->add(scalarMapper->mapToTextureCoord(prCellResults[cIdx])); 
                    }
                }
            }

            textureCoords->squeeze();
        }
        else
        {
            for (const auto& stimPlanCell : stimPlanCells)
            {
                for (const auto& cellCorner : stimPlanCell.getPolygon())
                {
                    stimPlanMeshVertices.push_back(static_cast<cvf::Vec3f>(cellCorner));
                }
            }
        }
    }

    if (stimPlanMeshVertices.empty())
    {
        return nullptr;
    }

    cvf::Mat4d fractureXf = m_rimFracture->transformMatrix();
    std::vector<cvf::Vec3f> nodeDisplayCoords =
        transformToFractureDisplayCoords(stimPlanMeshVertices, fractureXf, *displayCoordTransform);

    std::vector<cvf::uint> triIndicesToInclude;

    size_t cellCount = stimPlanMeshVertices.size() / 4;
    for (cvf::uint i = 0; i < cellCount; i++)
    {
        triIndicesToInclude.push_back(i * 4 + 0);
        triIndicesToInclude.push_back(i * 4 + 1);
        triIndicesToInclude.push_back(i * 4 + 2);

        triIndicesToInclude.push_back(i * 4 + 0);
        triIndicesToInclude.push_back(i * 4 + 2);
        triIndicesToInclude.push_back(i * 4 + 3);
    }

    // Show selected result on the surface geometry and filter out triangles that have result values near 0
    if (scalarMapper)
    {
        if (triIndicesToInclude.empty())
        {
            return nullptr;
        }

        cvf::ref<cvf::DrawableGeo> geo = buildDrawableGeoFromTriangles(triIndicesToInclude, nodeDisplayCoords);
        cvf::ref<cvf::Part> surfacePart = new cvf::Part(0, "FractureSurfacePart_stimPlan");
        surfacePart->setDrawable(geo.p());
        surfacePart->setPriority(RivPartPriority::PartType::BaseLevel);
        surfacePart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));

        geo->setTextureCoordArray(textureCoords.p());

        caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);
        effGen.disableLighting(activeView.isLightingDisabled());
        cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
        surfacePart->setEffect(eff.p());

        return surfacePart;
    }
    else 
    {
        // No result is mapped, show the entire StimPlan surface with default color

        return createSingleColorSurfacePart(triIndicesToInclude, nodeDisplayCoords, activeView.stimPlanColors->defaultColor());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createContainmentMaskPart(const RimEclipseView& activeView)
{
    std::vector<cvf::Vec3f> borderPolygonLocalCS =  m_rimFracture->fractureTemplate()->fractureBorderPolygon(m_rimFracture->fractureUnit());
    cvf::Mat4d frMx = m_rimFracture->transformMatrix();

    cvf::BoundingBox frBBox;
    std::vector<cvf::Vec3d> borderPolygonGlobCs;
    std::vector<cvf::Vec3d> borderPolygonLocalCsd;
    for (const auto& pv: borderPolygonLocalCS)
    {
        cvf::Vec3d pvd(pv);
        borderPolygonLocalCsd.push_back(pvd);
        pvd.transformPoint(frMx);
        borderPolygonGlobCs.push_back(pvd);
        frBBox.add(pvd);
    }

    std::vector<size_t> cellCandidates;
    activeView.mainGrid()->findIntersectingCells(frBBox, &cellCandidates);

    auto displCoordTrans = activeView.displayCoordTransform();

    std::vector<cvf::Vec3f> maskTriangles;

    for (size_t resCellIdx : cellCandidates)
    {
        if (!m_rimFracture->isEclipseCellWithinContainment(activeView.mainGrid(), resCellIdx))
        {
            // Calculate Eclipse cell intersection with fracture plane 

            std::array<cvf::Vec3d,8> corners; 
            activeView.mainGrid()->cellCornerVertices(resCellIdx, corners.data());   

            std::vector<std::vector<cvf::Vec3d> > eclCellPolygons;
            bool hasIntersection = RigHexIntersectionTools::planeHexIntersectionPolygons(corners, frMx, eclCellPolygons);

            if (!hasIntersection || eclCellPolygons.empty()) continue;

            // Transform eclCell - plane intersection onto fracture

            cvf::Mat4d invertedTransformMatrix = frMx.getInverted();
            for ( std::vector<cvf::Vec3d>& eclCellPolygon : eclCellPolygons )
            {
                for ( cvf::Vec3d& v : eclCellPolygon )
                {
                    v.transformPoint(invertedTransformMatrix);
                }
            }

            cvf::Vec3d fractureNormal = cvf::Vec3d(frMx.col(2));
            cvf::Vec3d maskOffset = fractureNormal * 0.01 * frBBox.radius();
            for (const std::vector<cvf::Vec3d>& eclCellPolygon : eclCellPolygons)
            {
                // Clip Eclipse cell polygon with fracture border

                std::vector< std::vector<cvf::Vec3d> > clippedPolygons = RigCellGeometryTools::intersectPolygons(eclCellPolygon, 
                                                                                                                 borderPolygonLocalCsd);
                for (auto& clippedPolygon : clippedPolygons)
                {
                    for (auto& v: clippedPolygon)
                    {
                        v.transformPoint(frMx);
                    }
                }

                // Create triangles from the clipped polygons

                for (auto& clippedPolygon : clippedPolygons)
                {
                    cvf::EarClipTesselator tess;
                    tess.setNormal(fractureNormal);
                    cvf::Vec3dArray cvfNodes(clippedPolygon);
                    tess.setGlobalNodeArray(cvfNodes);
                    std::vector<size_t> polyIndexes;
                    for (size_t idx = 0; idx < clippedPolygon.size(); ++idx) polyIndexes.push_back(idx);
                    tess.setPolygonIndices(polyIndexes);

                    std::vector<size_t> triangleIndices;
                    tess.calculateTriangles(&triangleIndices);

                    for (size_t idx: triangleIndices)
                    {
                        maskTriangles.push_back( cvf::Vec3f( displCoordTrans->transformToDisplayCoord(clippedPolygon[idx]  + maskOffset)) );
                    }

                    for (size_t idx: triangleIndices)
                    {
                        maskTriangles.push_back( cvf::Vec3f( displCoordTrans->transformToDisplayCoord(clippedPolygon[idx]  - maskOffset)) );
                    }
                }
            }
        }
    }

    if ( maskTriangles.size() >= 3 )
    {
        cvf::ref<cvf::DrawableGeo> maskTriangleGeo = new cvf::DrawableGeo;
        maskTriangleGeo->setVertexArray(new cvf::Vec3fArray(maskTriangles));

        cvf::ref<cvf::PrimitiveSetDirect> primitives = new cvf::PrimitiveSetDirect(cvf::PT_TRIANGLES);
        primitives->setIndexCount(maskTriangles.size());
        maskTriangleGeo->addPrimitiveSet(primitives.p());
        maskTriangleGeo->computeNormals();

        cvf::ref<cvf::Part> containmentMaskPart = new cvf::Part(0, "FractureContainmentMaskPart");
        containmentMaskPart->setDrawable(maskTriangleGeo.p());
        containmentMaskPart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));

        cvf::Color4f maskColor = cvf::Color4f(cvf::Color3f(cvf::Color3::GRAY));

        caf::SurfaceEffectGenerator surfaceGen(maskColor, caf::PO_NONE);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
        containmentMaskPart->setEffect(eff.p());

        return containmentMaskPart;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellFracturePartMgr::createStimPlanMeshPart(const RimEclipseView& activeView)
{
    if (!m_rimFracture->fractureTemplate()) return nullptr;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
    if (!stimPlanFracTemplate) return nullptr;

    cvf::ref<cvf::DrawableGeo> stimPlanMeshGeo = createStimPlanMeshDrawable(stimPlanFracTemplate, activeView);
    if (stimPlanMeshGeo.notNull())
    {
        cvf::ref<cvf::Part> stimPlanMeshPart = new cvf::Part(0, "StimPlanMesh");
        stimPlanMeshPart->setDrawable(stimPlanMeshGeo.p());

        stimPlanMeshPart->updateBoundingBox();
        stimPlanMeshPart->setPriority(RivPartPriority::PartType::TransparentMeshLines);

        caf::MeshEffectGenerator lineEffGen(cvf::Color3::BLACK);
        lineEffGen.setLineWidth(1.0f);
        cvf::ref<cvf::Effect> eff = lineEffGen.generateCachedEffect();

        stimPlanMeshPart->setEffect(eff.p());

        return stimPlanMeshPart;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createStimPlanMeshDrawable(RimStimPlanFractureTemplate* stimPlanFracTemplate, const RimEclipseView& activeView) const
{
    //TODO: This is needed to avoid errors when loading project with stimPlan fractures with multipled timesteps. 
    //Should probably be moved, since it now is called twice in some cases... 
    stimPlanFracTemplate->updateFractureGrid();

    if (!stimPlanFracTemplate->fractureGrid()) return nullptr;

    auto displayCoordTransform = activeView.displayCoordTransform();
    if (displayCoordTransform.isNull()) return nullptr;

    std::vector<RigFractureCell> stimPlanCells = stimPlanFracTemplate->fractureGrid()->fractureCells();
    std::vector<cvf::Vec3f> stimPlanMeshVertices;

    QString resultNameFromColors = activeView.stimPlanColors->resultName();
    QString resultUnitFromColors = activeView.stimPlanColors->unit();

    std::vector<double> prCellResults = stimPlanFracTemplate->fractureGridResults(resultNameFromColors,
                                                                                  resultUnitFromColors,
                                                                                  stimPlanFracTemplate->activeTimeStepIndex());

    for ( size_t cIdx = 0; cIdx < stimPlanCells.size() ; ++cIdx)
    {
        if (prCellResults[cIdx] > 1e-7)
        {
            const RigFractureCell& stimPlanCell = stimPlanCells[cIdx];
            std::vector<cvf::Vec3d> stimPlanCellPolygon = stimPlanCell.getPolygon();
            for (const cvf::Vec3d& cellCorner : stimPlanCellPolygon)
            {
                stimPlanMeshVertices.push_back(static_cast<cvf::Vec3f>(cellCorner));
            }
        }
    }

    if (stimPlanMeshVertices.empty())
    {
        return nullptr;
    }

    cvf::Mat4d fractureXf = m_rimFracture->transformMatrix();
    std::vector<cvf::Vec3f> stimPlanMeshVerticesDisplayCoords = transformToFractureDisplayCoords(stimPlanMeshVertices, 
                                                                                                 fractureXf, 
                                                                                                 *displayCoordTransform);

    cvf::Vec3fArray* stimPlanMeshVertexList;
    stimPlanMeshVertexList = new cvf::Vec3fArray;
    stimPlanMeshVertexList->assign(stimPlanMeshVerticesDisplayCoords);

    cvf::ref<cvf::DrawableGeo> stimPlanMeshGeo = new cvf::DrawableGeo;
    stimPlanMeshGeo->setVertexArray(stimPlanMeshVertexList);
    cvf::ref<cvf::UIntArray> indices = RivFaultGeometryGenerator::lineIndicesFromQuadVertexArray(stimPlanMeshVertexList);
    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(indices.p());

    stimPlanMeshGeo->addPrimitiveSet(prim.p());

    return stimPlanMeshGeo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RivWellFracturePartMgr::transformToFractureDisplayCoords(const std::vector<cvf::Vec3f>& coordinatesVector, 
                                                                                 cvf::Mat4d m, 
                                                                                 const caf::DisplayCoordTransform& displayCoordTransform)
{
    std::vector<cvf::Vec3f> polygonInDisplayCoords;
    polygonInDisplayCoords.reserve(coordinatesVector.size());

    for (const cvf::Vec3f& v : coordinatesVector)
    {
        cvf::Vec3d vd(v);
        vd.transformPoint(m);
        cvf::Vec3d displayCoordsDouble = displayCoordTransform.transformToDisplayCoord(vd);
        polygonInDisplayCoords.push_back(cvf::Vec3f(displayCoordsDouble));
    }

    return polygonInDisplayCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RivWellFracturePartMgr::mirrorDataAtSingleDepth(std::vector<double> depthData)
{
    std::vector<double> mirroredValuesAtGivenDepth;
    mirroredValuesAtGivenDepth.push_back(depthData[0]);
    for (size_t i = 1; i < (depthData.size()); i++) //starting at 1 since we don't want center value twice
    {
        double valueAtGivenX = depthData[i];
        mirroredValuesAtGivenDepth.insert(mirroredValuesAtGivenDepth.begin(), valueAtGivenX);
        mirroredValuesAtGivenDepth.push_back(valueAtGivenX);
    }

    return mirroredValuesAtGivenDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::appendGeometryPartsToModel(cvf::ModelBasicList* model, const RimEclipseView& eclView)
{
    if (!m_rimFracture->isChecked()) return;

    cvf::ref<cvf::Part> surfacePart;
    cvf::ref<cvf::Part> stimPlanMeshPart;
    cvf::ref<cvf::Part> containmentMaskPart;


    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
    if (m_rimFracture->fractureTemplate())
    {
        // StimPlan
        if (stimPlanFracTemplate)
        {
            if (m_rimFracture->stimPlanResultColorType() == RimFracture::SINGLE_ELEMENT_COLOR)
            {
                surfacePart = createStimPlanElementColorSurfacePart(eclView);
            }
            else
            {
                surfacePart = createStimPlanColorInterpolatedSurfacePart(eclView);
            }

            if (stimPlanFracTemplate->showStimPlanMesh())
            {
                stimPlanMeshPart = createStimPlanMeshPart(eclView);
            }
        }
        // Ellipse
        else 
        {
            surfacePart = createEllipseSurfacePart(eclView);
        }

        if (m_rimFracture->fractureTemplate()->fractureContainment()->isEnabled())
        {
            containmentMaskPart = createContainmentMaskPart(eclView);
        }
    }


    if (surfacePart.notNull())
    {
        model->addPart(surfacePart.p());
    }

    if (stimPlanMeshPart.notNull())
    {
        model->addPart(stimPlanMeshPart.p());
    }

    if (containmentMaskPart.notNull())
    {
        model->addPart(containmentMaskPart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::buildDrawableGeoFromTriangles(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords)
{
    CVF_ASSERT(triangleIndices.size() > 0);
    CVF_ASSERT(nodeCoords.size() > 0);

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray(triangleIndices);
    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray(nodeCoords);

    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES, indices.p()));
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivWellFracturePartMgr::stimPlanCellTouchesPolygon(const std::vector<cvf::Vec3f>& polygon, 
                                                        double xMin, double xMax, double yMin, double yMax, 
                                                        float polygonXmin, float polygonXmax, float polygonYmin, float polygonYmax)
{

    if (static_cast<float>(xMin) > polygonXmin && static_cast<float>(xMax) < polygonXmax)
    {
        if (static_cast<float>(yMin) > polygonYmin && static_cast<float>(yMax) < polygonYmax)
        {
            return true;
        }
    }

    for (cvf::Vec3f v : polygon)
    {
        if (v.x() > xMin && v.x() < xMax)
        {
            if (v.y() > yMin && v.y() < yMax)
            {
                return true;
            }
        }
    }

    return false;
}

