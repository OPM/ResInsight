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

#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFishboneWellPath.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RimWellPathFractureCollection.h"
#include "RimWellPathFracture.h"

#include "RivFishbonesSubsPartMgr.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivWellPathSourceInfo.h"

#include "RivPartPriority.h"
#include "RivWellFracturePartMgr.h"
#include "RivWellPathPartMgr.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfFont.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfqtUtils.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::RivWellPathPartMgr(RimWellPath* wellPath)
{
    m_rimWellPath = wellPath;
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
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
void RivWellPathPartMgr::appendStaticFracturePartsToModel(cvf::ModelBasicList* model, const RimEclipseView& eclView)
{
    if (!m_rimWellPath || !m_rimWellPath->showWellPath() || !m_rimWellPath->fractureCollection()->isChecked()) return;

    for (RimWellPathFracture* f : m_rimWellPath->fractureCollection()->fractures())
    {
        CVF_ASSERT(f);

        f->fracturePartManager()->appendGeometryPartsToModel(model, eclView);
    }
}
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFishboneSubsPartsToModel(cvf::ModelBasicList* model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        double characteristicCellSize)
{
    if ( !m_rimWellPath || !m_rimWellPath->fishbonesCollection()->isChecked() ) return;

    for ( auto rimFishboneSubs : m_rimWellPath->fishbonesCollection()->fishbonesSubs() )
    {
        cvf::ref<RivFishbonesSubsPartMgr> fishbSubPartMgr = new RivFishbonesSubsPartMgr(rimFishboneSubs);
        fishbSubPartMgr->appendGeometryPartsToModel(model, displayCoordTransform, characteristicCellSize);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendImportedFishbonesToModel(cvf::ModelBasicList* model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        double characteristicCellSize)
{
    if (!m_rimWellPath || !m_rimWellPath->fishbonesCollection()->wellPathCollection()->isChecked()) return;

    RivPipeGeometryGenerator geoGenerator;
    std::vector<RimFishboneWellPath*> fishbonesWellPaths;
    m_rimWellPath->descendantsIncludingThisOfType(fishbonesWellPaths);
    for (RimFishboneWellPath* fbWellPath : fishbonesWellPaths)
    {
        if (!fbWellPath->isChecked()) continue;

        std::vector<cvf::Vec3d> displayCoords;
        for (auto lateralDomainCoords : fbWellPath->coordinates())
        {
            displayCoords.push_back(displayCoordTransform->transformToDisplayCoord(lateralDomainCoords));
        }

        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(fbWellPath);

        cvf::Collection<cvf::Part> parts;
        geoGenerator.cylinderWithCenterLineParts(&parts, 
                                                 displayCoords, 
                                                 m_rimWellPath->wellPathColor(), 
                                                 m_rimWellPath->combinedScaleFactor() * characteristicCellSize * 0.5);
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
void RivWellPathPartMgr::appendPerforationsToModel(const QDateTime& currentViewDate, 
                                                   cvf::ModelBasicList* model, 
                                                   const caf::DisplayCoordTransform* displayCoordTransform, 
                                                   double characteristicCellSize)
{
    if (!m_rimWellPath || !m_rimWellPath->perforationIntervalCollection()->isChecked()) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    // Since we're using the index of measured depths to find the index of a point, ensure they're equal
    CVF_ASSERT(wellPathGeometry->m_measuredDepths.size() == wellPathGeometry->m_wellPathPoints.size());

    double wellPathRadius = m_rimWellPath->wellPathRadius(characteristicCellSize);
    double perforationRadius = wellPathRadius * 1.1;

    RivPipeGeometryGenerator geoGenerator;
    std::vector<RimPerforationInterval*> perforations;
    m_rimWellPath->descendantsIncludingThisOfType(perforations);
    for (RimPerforationInterval* perforation : perforations)
    {
        if (!perforation->isChecked()) continue;
        if (perforation->startMD() > perforation->endMD()) continue;

        if (currentViewDate.isValid() && !perforation->isActiveOnDate(currentViewDate)) continue;

        using namespace std;
        pair<vector<cvf::Vec3d>, vector<double> >  displayCoordsAndMD = wellPathGeometry->clippedPointSubset(perforation->startMD(), 
                                                                                                             perforation->endMD());
 
        if (displayCoordsAndMD.first.size() < 2) continue;

        for (cvf::Vec3d& point : displayCoordsAndMD.first) point = displayCoordTransform->transformToDisplayCoord(point);

        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(perforation);

        cvf::Collection<cvf::Part> parts;
        geoGenerator.cylinderWithCenterLineParts(&parts, displayCoordsAndMD.first, cvf::Color3f::GREEN, perforationRadius);
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
void RivWellPathPartMgr::buildWellPathParts(const caf::DisplayCoordTransform* displayCoordTransform,
                                            double characteristicCellSize, 
                                            const cvf::BoundingBox& wellPathClipBoundingBox)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    if (wellPathGeometry->m_wellPathPoints.size() < 2) return;

    clearAllBranchData();
    double wellPathRadius = m_rimWellPath->wellPathRadius(characteristicCellSize);

    cvf::Vec3d textPosition;

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
                if (firstVisibleSegmentIndex > 0)
                {
                    double wellPathStartPoint = wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
                    double stepsize = (wellPathStartPoint - wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex - 1].z()) / 
                                      (wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex].z() - wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex - 1].z());

                    cvf::Vec3d newPoint = wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex - 1] +
                                          stepsize * (wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex] - wellPathGeometry->m_wellPathPoints[firstVisibleSegmentIndex - 1]);

                    clippedPoints.push_back(newPoint);
                    pbd.m_pipeGeomGenerator->setFirstVisibleSegmentIndex(firstVisibleSegmentIndex - 1);
                }
                else
                {
                    pbd.m_pipeGeomGenerator->setFirstVisibleSegmentIndex(firstVisibleSegmentIndex);
                }

                for (size_t idx = firstVisibleSegmentIndex; idx < wellPathGeometry->m_wellPathPoints.size(); idx++)
                {
                    clippedPoints.push_back(wellPathGeometry->m_wellPathPoints[idx]);
                }

            }

            if (clippedPoints.size() < 2) return;

            cvfCoords->assign(clippedPoints);
        }
        else
        {
            cvfCoords->assign(wellPathGeometry->m_wellPathPoints);
        }
        
        // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.
        for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
        {
            (*cvfCoords)[cIdx] = displayCoordTransform->transformToDisplayCoord((*cvfCoords)[cIdx]);
        }

        textPosition = cvfCoords->get(0);

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

    textPosition.z() += 2.2 * characteristicCellSize; 

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

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model,
                                                          double characteristicCellSize,
                                                          const cvf::BoundingBox& wellPathClipBoundingBox,
                                                          const caf::DisplayCoordTransform* displayCoordTransform)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF)
        return;

    if (wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false )
        return;

    // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
    buildWellPathParts(displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox);

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

    appendFishboneSubsPartsToModel(model, displayCoordTransform, characteristicCellSize);
    appendImportedFishbonesToModel(model, displayCoordTransform, characteristicCellSize);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                           const QDateTime& timeStamp,
                                                           double characteristicCellSize,
                                                           const cvf::BoundingBox& wellPathClipBoundingBox,
                                                           const caf::DisplayCoordTransform* displayCoordTransform)
{
    CVF_ASSERT(model);

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (m_rimWellPath.isNull()) return;

    if (wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF)
        return;

    if (wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false)
        return;

    appendPerforationsToModel(timeStamp, model, displayCoordTransform, characteristicCellSize);
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

