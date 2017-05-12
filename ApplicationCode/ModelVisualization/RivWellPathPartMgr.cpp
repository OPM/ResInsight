/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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


#include "RivWellPathPartMgr.h"

#include "RiaApplication.h"

#include "RigWellPath.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletion.h"
#include "RimWellPathCompletionCollection.h"
#include "RimPerforationInterval.h"
#include "RimPerforationCollection.h"

#include "RivFishbonesSubsPartMgr.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivWellPathSourceInfo.h"
#include "RivObjectSourceInfo.h"

#include "cafEffectGenerator.h"
#include "cafDisplayCoordTransform.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfFont.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::RivWellPathPartMgr(RimWellPath* wellPath)
{
    m_rimWellPath      = wellPath;

    m_needsTransformUpdate = true;

    // Setup a scalar mapper
    cvf::ref<cvf::ScalarMapperDiscreteLinear> scalarMapper = new cvf::ScalarMapperDiscreteLinear;
    cvf::Color3ubArray legendColors;
    legendColors.resize(4);
    legendColors[0] = cvf::Color3::GRAY;
    legendColors[1] = cvf::Color3::GREEN;
    legendColors[2] = cvf::Color3::BLUE;
    legendColors[3] = cvf::Color3::RED;
    scalarMapper->setColors(legendColors);
    scalarMapper->setRange(0.0 , 4.0);
    scalarMapper->setLevelCount(4, true);

    m_scalarMapper = scalarMapper;

    caf::ScalarMapperEffectGenerator surfEffGen(scalarMapper.p(), caf::PO_1);
    m_scalarMapperSurfaceEffect = surfEffGen.generateCachedEffect();

    caf::ScalarMapperMeshEffectGenerator meshEffGen(scalarMapper.p());
    m_scalarMapperMeshEffect = meshEffGen.generateCachedEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::~RivWellPathPartMgr()
{
    clearAllBranchData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFishbonesPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    if (!m_rimWellPath) return;

    // This concept is taken from RivReservoirSimWellsPartMgr, and is required to be able to have
    // separate part managers for each view
    if (m_fishbonesPartMgrs.size() != m_rimWellPath->fishbonesSubs().size())
    {
        m_fishbonesPartMgrs.clear();

        for (auto rimFishboneSubs : m_rimWellPath->fishbonesSubs())
        {
            m_fishbonesPartMgrs.push_back(new RivFishbonesSubsPartMgr(rimFishboneSubs));
        }
    }

    for (auto rivFishbonesPartManager : m_fishbonesPartMgrs)
    {
        rivFishbonesPartManager->appendGeometryPartsToModel(model, displayCoordTransform, characteristicCellSize);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendCompletionsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    if (!m_rimWellPath || !m_rimWellPath->m_completionCollection->isChecked()) return;

    RivPipeGeometryGenerator geoGenerator;
    for (RimWellPathCompletion* completion : m_rimWellPath->m_completionCollection()->m_completions())
    {
        if (!completion->isChecked()) continue;

        std::vector<cvf::Vec3d> displayCoords;
        for (auto lateralDomainCoords : completion->coordinates())
        {
            displayCoords.push_back(displayCoordTransform->transformToDisplayCoord(lateralDomainCoords));
        }

        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(completion);

        cvf::Collection<cvf::Part> parts;
        geoGenerator.cylinderWithCenterLineParts(&parts, displayCoords, m_rimWellPath->wellPathColor(), m_rimWellPath->combinedScaleFactor() * characteristicCellSize * 0.5);
        for (auto part : parts)
        {
            part->setSourceInfo(objectSourceInfo.p());
            model->addPart(part.p());
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendPerforationsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    if (!m_rimWellPath || !m_rimWellPath->m_perforationCollection->isChecked()) return;

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    // Since we're using the index of measured depths to find the index of a point, ensure they're equal
    CVF_ASSERT(wellPathGeometry->m_measuredDepths.size() == wellPathGeometry->m_wellPathPoints.size());

    double wellPathRadius = this->wellPathRadius(characteristicCellSize, wellPathCollection);
    double perforationRadius = wellPathRadius * 1.1;

    RivPipeGeometryGenerator geoGenerator;
    for (RimPerforationInterval* perforation : m_rimWellPath->m_perforationCollection->m_perforations())
    {
        if (!perforation->isChecked()) continue;

        std::vector<cvf::Vec3d> displayCoords;
        for (size_t i = 0; i < wellPathGeometry->m_measuredDepths.size(); ++i)
        {
            double measuredDepth = wellPathGeometry->m_measuredDepths[i];
            if (measuredDepth < perforation->startMD()) continue;
            if (measuredDepth > perforation->endMD()) break;
            displayCoords.push_back(displayCoordTransform->transformToDisplayCoord(wellPathGeometry->m_wellPathPoints[i]));
        }

        if (displayCoords.size() < 2) continue;

        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(perforation);

        cvf::Collection<cvf::Part> parts;
        geoGenerator.cylinderWithCenterLineParts(&parts, displayCoords, cvf::Color3f::GREEN, perforationRadius);
        for (auto part : parts)
        {
            part->setSourceInfo(objectSourceInfo.p());
            model->addPart(part.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// The pipe geometry needs to be rebuilt on scale change to keep the pipes round
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::buildWellPathParts(cvf::Vec3d displayModelOffset, double characteristicCellSize, 
                                            cvf::BoundingBox wellPathClipBoundingBox)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    if (wellPathGeometry->m_wellPathPoints.size() < 2) return;

    clearAllBranchData();
    double wellPathRadius = this->wellPathRadius(characteristicCellSize, wellPathCollection);

    cvf::Vec3d textPosition = wellPathGeometry->m_wellPathPoints[0];

    // Generate the well path geometry as a line and pipe structure
    {
        RivPipeBranchData& pbd = m_pipeBranchData;

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(wellPathRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(wellPathCollection->wellPathCrossSectionVertexCount());
        pbd.m_pipeGeomGenerator->setPipeColor( m_rimWellPath->wellPathColor());

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        if (wellPathCollection->wellPathClip)
        {
            size_t firstVisibleSegmentIndex = cvf::UNDEFINED_SIZE_T;
            for (size_t idx = 0; idx < wellPathGeometry->m_wellPathPoints.size(); idx++)
            {
                cvf::Vec3d point = wellPathGeometry->m_wellPathPoints[idx];
                if (point.z() < (wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance))
                {
                    firstVisibleSegmentIndex = idx;
                    break;
                }
            }

            std::vector<cvf::Vec3d> clippedPoints;

            if (firstVisibleSegmentIndex != cvf::UNDEFINED_SIZE_T)
            {
                for (size_t idx = firstVisibleSegmentIndex; idx < wellPathGeometry->m_wellPathPoints.size(); idx++)
                {
                    clippedPoints.push_back(wellPathGeometry->m_wellPathPoints[idx]);
                }

                pbd.m_pipeGeomGenerator->setFirstSegmentIndex(firstVisibleSegmentIndex);
            }

            if (clippedPoints.size() < 2) return;

            textPosition = clippedPoints[0];
            cvfCoords->assign(clippedPoints);
        }
        else
        {
            cvfCoords->assign(wellPathGeometry->m_wellPathPoints);
        }
        
        // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.
        for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
        {
            cvf::Vec4d transfCoord = m_scaleTransform->worldTransform() * cvf::Vec4d((*cvfCoords)[cIdx] - displayModelOffset, 1);
            (*cvfCoords)[cIdx][0] = transfCoord[0];
            (*cvfCoords)[cIdx][1] = transfCoord[1];
            (*cvfCoords)[cIdx][2] = transfCoord[2];
        }

        pbd.m_pipeGeomGenerator->setPipeCenterCoords(cvfCoords.p());
        pbd.m_surfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        pbd.m_centerLineDrawable = pbd.m_pipeGeomGenerator->createCenterLine();

        if (pbd.m_surfaceDrawable.notNull())
        {
            pbd.m_surfacePart = new cvf::Part;
            pbd.m_surfacePart->setDrawable(pbd.m_surfaceDrawable.p());
            
            RivWellPathSourceInfo* sourceInfo = new RivWellPathSourceInfo(m_rimWellPath);
            pbd.m_surfacePart->setSourceInfo(sourceInfo);

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWellPath->wellPathColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect(eff.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part;
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWellPath->wellPathColor());
            cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }
    }

    // Generate label with well-path name

    textPosition -= displayModelOffset;
    textPosition.transformPoint(m_scaleTransform->worldTransform());
    textPosition.z() += characteristicCellSize; // * m_rimReservoirView->wellCollection()->wellHeadScaleFactor();
    textPosition.z() += 1.2 * characteristicCellSize;

    m_wellLabelPart = NULL;
    if (wellPathCollection->showWellPathLabel() && m_rimWellPath->showWellPathLabel() && !m_rimWellPath->name().isEmpty())
    {
        cvf::Font* font = RiaApplication::instance()->customFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(font);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
        drawableText->setTextColor(wellPathCollection->wellPathLabelColor());

        cvf::String cvfString = cvfqt::Utils::toString(m_rimWellPath->name());

        cvf::Vec3f textCoord(textPosition);
        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: text " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::Text);

        m_wellLabelPart = part;
    }

    m_needsTransformUpdate = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, cvf::Vec3d displayModelOffset, 
                                                          double characteristicCellSize, cvf::BoundingBox wellPathClipBoundingBox,
                                                          caf::DisplayCoordTransform* displayCoordTransform)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (m_rimWellPath.isNull()) return;

    if (wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF)
        return;

    if (wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false )
        return;

    if (m_needsTransformUpdate) 
    {
        // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
        buildWellPathParts(displayModelOffset, characteristicCellSize, wellPathClipBoundingBox);
    }
 
    if (m_pipeBranchData.m_surfacePart.notNull())
    {
        model->addPart(m_pipeBranchData.m_surfacePart.p());
    }

    if (m_pipeBranchData.m_centerLinePart.notNull())
    {
        model->addPart(m_pipeBranchData.m_centerLinePart.p());
    }

    if (m_wellLabelPart.notNull())
    {
        model->addPart(m_wellLabelPart.p());
    }

    appendFishbonesPartsToModel(model, displayCoordTransform, characteristicCellSize);
    appendCompletionsToModel(model, displayCoordTransform, characteristicCellSize);
    appendPerforationsToModel(model, displayCoordTransform, characteristicCellSize);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::setScaleTransform( cvf::Transform * scaleTransform )
{
    if (m_scaleTransform.isNull() || m_scaleTransform.p() != scaleTransform) 
    {
        m_scaleTransform = scaleTransform; 
        scheduleGeometryRegen(); 
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::scheduleGeometryRegen()
{
    m_needsTransformUpdate = true;

    m_fishbonesPartMgrs.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::clearAllBranchData()
{
    m_pipeBranchData.m_pipeGeomGenerator = NULL;
    m_pipeBranchData.m_surfacePart = NULL;
    m_pipeBranchData.m_surfaceDrawable = NULL;
    m_pipeBranchData.m_centerLinePart = NULL;
    m_pipeBranchData.m_centerLineDrawable = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivWellPathPartMgr::segmentIndexFromTriangleIndex(size_t triangleIndex)
{
    return m_pipeBranchData.m_pipeGeomGenerator->segmentIndexFromTriangleIndex(triangleIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RivWellPathPartMgr::wellPathCollection()
{
    if (!m_rimWellPath) return nullptr;

    RimWellPathCollection* wellPathCollection = nullptr;
    m_rimWellPath->firstAncestorOrThisOfType(wellPathCollection);

    return wellPathCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RivWellPathPartMgr::wellPathRadius(double characteristicCellSize, RimWellPathCollection* wellPathCollection)
{
    return wellPathCollection->wellPathRadiusScaleFactor() * m_rimWellPath->wellPathRadiusScaleFactor() * characteristicCellSize;
}
