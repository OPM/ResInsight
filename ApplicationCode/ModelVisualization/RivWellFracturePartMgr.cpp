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

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimLegendConfig.h"
#include "RigFractureCell.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"

#include "RivFaultGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfRenderStateDepth.h"


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
void RivWellFracturePartMgr::generateSurfacePart(const caf::DisplayCoordTransform* displayCoordTransform)
{
    if (m_surfacePart.notNull()) return;
    if (!displayCoordTransform) return;

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

        if (triangleIndices.size() == 0 || displayCoords.size() == 0)
        {
            return;
        }

        cvf::ref<cvf::DrawableGeo> geo = buildDrawableGeoFromTriangles(triangleIndices, displayCoords);
        CVF_ASSERT(geo.notNull());
            
        m_surfacePart = new cvf::Part(0, "FractureSurfacePart");
        m_surfacePart->setDrawable(geo.p());
        m_surfacePart->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::applyFractureUniformColor(const RimEclipseView* activeView)
{
    if ( m_surfacePart.notNull() )
    {
        cvf::Color4f fractureColor = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN));

        if ( activeView )
        {
            fractureColor = cvf::Color4f(activeView->stimPlanColors->defaultColor());
        }

        caf::SurfaceEffectGenerator surfaceGen(fractureColor, caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
        m_surfacePart->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::applyResultTextureColor(const RimEclipseView* activeView)
{
    if (m_surfacePart.isNull()) return;

    if (m_rimFracture)
    {
        RimLegendConfig* legendConfig = nullptr;
        if (activeView && activeView->stimPlanColors())
        {
            if (activeView->stimPlanColors()->isChecked())
            {
                legendConfig = activeView->stimPlanColors()->activeLegend();
            }
        }

        RimFractureTemplate* fracTemplate = m_rimFracture->fractureTemplate();
        RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
        if (!stimPlanFracTemplate)
        {
            return;
        }

        float opacityLevel = activeView->stimPlanColors->opacityLevel();
        if (legendConfig)
        {
            cvf::ScalarMapper* scalarMapper =  legendConfig->scalarMapper();
            cvf::DrawableGeo* geo = dynamic_cast<cvf::DrawableGeo*> (m_surfacePart->drawable());
            cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray;
            textureCoords->resize(geo->vertexCount());

            int timeStepIndex = m_rimFracture->stimPlanTimeIndexToPlot;
            std::vector<std::vector<double> > dataToPlot = stimPlanFracTemplate->resultValues(activeView->stimPlanColors->resultName(), 
                                                                                              activeView->stimPlanColors->unit(), 
                                                                                              timeStepIndex);

            int i = 0;
            for (std::vector<double> depthData : dataToPlot)
            {
                std::vector<double> mirroredValuesAtDepth = mirrorDataAtSingleDepth(depthData);
                for (double gridXdata : mirroredValuesAtDepth)
                {
                    cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord(gridXdata);
                    
                    if (gridXdata > 1e-7)
                    {
                        texCoord[1] = 0; // Set the Y texture coordinate to the opaque line in the texture
                    }
                    textureCoords->set(i, texCoord);


                    i++;
                }
            }
            
            geo->setTextureCoordArray(textureCoords.p());

            caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);

            effGen.setOpacityLevel(0.5);
            effGen.discardTransparentFragments(true);

            if (activeView && activeView->isLightingDisabled())
            {
                effGen.disableLighting(true);
            }

            cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();

            m_surfacePart->setEffect(eff.p());
            m_surfacePart->setPriority(RivPartPriority::PartType::Transparent);
        }
        else
        {
            applyFractureUniformColor(activeView);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::generateFractureOutlinePolygonPart(const caf::DisplayCoordTransform* displayCoordTransform)
{
    m_polygonPart = nullptr;

    cvf::ref<cvf::DrawableGeo> polygonGeo = createPolygonDrawable(displayCoordTransform);

    if (polygonGeo.notNull())
    {
        m_polygonPart = new cvf::Part(0, "FractureOutline");
        m_polygonPart->setDrawable(polygonGeo.p());

        m_polygonPart->updateBoundingBox();
        m_polygonPart->setPriority(RivPartPriority::PartType::TransparentMeshLines);

        caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
        lineEffGen.setLineWidth(3.0f);
        cvf::ref<cvf::Effect> eff = lineEffGen.generateCachedEffect();

        m_polygonPart->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::generateStimPlanMeshPart(const caf::DisplayCoordTransform* displayCoordTransform)
{
    m_stimPlanMeshPart = nullptr;

    if (!m_rimFracture->fractureTemplate()) return;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
    if (!stimPlanFracTemplate) return;

    cvf::ref<cvf::DrawableGeo> stimPlanMeshGeo = createStimPlanMeshDrawable(stimPlanFracTemplate, displayCoordTransform);
    if (stimPlanMeshGeo.notNull())
    {
        m_stimPlanMeshPart = new cvf::Part(0, "StimPlanMesh");
        m_stimPlanMeshPart->setDrawable(stimPlanMeshGeo.p());

        m_stimPlanMeshPart->updateBoundingBox();
        m_stimPlanMeshPart->setPriority(RivPartPriority::PartType::TransparentMeshLines);

        caf::MeshEffectGenerator lineEffGen(cvf::Color3::BLACK);
        lineEffGen.setLineWidth(1.0f);
        cvf::ref<cvf::Effect> eff = lineEffGen.generateCachedEffect();

        m_stimPlanMeshPart->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createStimPlanMeshDrawable(RimStimPlanFractureTemplate* stimPlanFracTemplate, 
                                                                              const caf::DisplayCoordTransform* displayCoordTransform)
{
    //TODO: This is needed to avoid errors when loading project with stimPlan fractures with multipled timesteps. 
    //Should probably be moved, since it now is called twice in some cases... 
    stimPlanFracTemplate->updateFractureGrid();

    std::vector<RigFractureCell> stimPlanCells = stimPlanFracTemplate->fractureGrid()->fractureCells();
    std::vector<cvf::Vec3f> stimPlanMeshVertices;

    for (RigFractureCell stimPlanCell : stimPlanCells)
    {
        if (stimPlanCell.getDisplayValue() > 1e-7)
        {
            std::vector<cvf::Vec3d> stimPlanCellPolygon = stimPlanCell.getPolygon();
            for (cvf::Vec3d cellCorner : stimPlanCellPolygon)
            {
                stimPlanMeshVertices.push_back(static_cast<cvf::Vec3f>(cellCorner));
            }
        }
    }

    if (stimPlanMeshVertices.size() == 0)
    {
        return nullptr;
    }

    cvf::Mat4d fractureXf = m_rimFracture->transformMatrix();
    std::vector<cvf::Vec3f> stimPlanMeshVerticesDisplayCoords = transfromToFractureDisplayCoords(stimPlanMeshVertices, 
                                                                                                 fractureXf, 
                                                                                                 displayCoordTransform);

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
void RivWellFracturePartMgr::getPolygonBB(float &polygonXmin, float &polygonXmax, float &polygonYmin, float &polygonYmax)
{
    std::vector<cvf::Vec3f> polygon = m_rimFracture->fractureTemplate()->fractureBorderPolygon(m_rimFracture->fractureUnit());

    if (polygon.size() > 1)
    {
        polygonXmin = polygon[0].x();
        polygonXmax = polygon[0].x();
        polygonYmin = polygon[0].y();
        polygonYmax = polygon[0].y();
    }

    for (cvf::Vec3f v : polygon)
    {
        if (v.x() < polygonXmin) polygonXmin = v.x();
        if (v.x() > polygonXmax) polygonXmax = v.x();
        if (v.y() < polygonYmin) polygonYmin = v.y();
        if (v.y() > polygonYmax) polygonYmax = v.y();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createPolygonDrawable(const caf::DisplayCoordTransform* displayCoordTransform)
{
    std::vector<cvf::uint> lineIndices;
    std::vector<cvf::Vec3f> vertices;

    {
        std::vector<cvf::Vec3f> polygon = m_rimFracture->fractureTemplate()->fractureBorderPolygon(m_rimFracture->fractureUnit());

        cvf::Mat4d m = m_rimFracture->transformMatrix();
        std::vector<cvf::Vec3f> polygonDisplayCoords = transfromToFractureDisplayCoords(polygon, m, displayCoordTransform);

        for (size_t i = 0; i < polygonDisplayCoords.size(); ++i)
        {
            vertices.push_back(cvf::Vec3f(polygonDisplayCoords[i]));
            if (i < polygonDisplayCoords.size() - 1)
            {
                lineIndices.push_back(static_cast<cvf::uint>(i));
                lineIndices.push_back(static_cast<cvf::uint>(i + 1));
            }
        }
    }

    if (vertices.size() == 0) return nullptr;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign(vertices);
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign(lineIndices);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(idxes.p());

    cvf::ref<cvf::DrawableGeo> polygonGeo = new cvf::DrawableGeo;
    polygonGeo->setVertexArray(vx.p());
    polygonGeo->addPrimitiveSet(prim.p());

    return polygonGeo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RivWellFracturePartMgr::transfromToFractureDisplayCoords(const std::vector<cvf::Vec3f>& coordinatesVector, 
                                                                                 cvf::Mat4d m, 
                                                                                 const caf::DisplayCoordTransform* displayCoordTransform)
{
    std::vector<cvf::Vec3f> polygonInDisplayCoords;
    polygonInDisplayCoords.reserve(coordinatesVector.size());

    for (const cvf::Vec3f& v : coordinatesVector)
    {
        cvf::Vec3d vd(v);
        vd.transformPoint(m);
        cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(vd);
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
void RivWellFracturePartMgr::appendGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                        const RimEclipseView* eclView)
{
    clearGeometryCache();

    if (!m_rimFracture->isChecked()) return;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->fractureTemplate());
   auto displayCoordTransform = eclView->displayCoordTransform();
    if (m_surfacePart.isNull())
    {
        if (m_rimFracture->fractureTemplate())
        {
            if (stimPlanFracTemplate)
            {
                generateSurfacePart(displayCoordTransform.p());
                generateFractureOutlinePolygonPart(displayCoordTransform.p());

                applyResultTextureColor(eclView);
                
                if (stimPlanFracTemplate->showStimPlanMesh())
                {
                    generateStimPlanMeshPart(displayCoordTransform.p());
                }
            }
            else // Ellipse
            {
                generateSurfacePart(displayCoordTransform.p());
                applyFractureUniformColor(eclView);
            }
        }
    }

    if (m_surfacePart.notNull())
    {
        model->addPart(m_surfacePart.p());
    }

    if (m_stimPlanMeshPart.notNull()
        && stimPlanFracTemplate->showStimPlanMesh())
    {
        model->addPart(m_stimPlanMeshPart.p());
    }

    if (stimPlanFracTemplate
        && m_rimFracture->showPolygonFractureOutline()
        && m_polygonPart.notNull())
    {
        model->addPart(m_polygonPart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::clearGeometryCache()
{
    m_surfacePart = nullptr;
    m_polygonPart = nullptr;
    m_stimPlanMeshPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::buildDrawableGeoFromTriangles(const std::vector<cvf::uint>& triangleIndices, 
                                                                                 const std::vector<cvf::Vec3f>& nodeCoords)
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

