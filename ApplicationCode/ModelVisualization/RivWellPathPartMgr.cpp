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

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellPath.h"

#include "Rim3dWellLogCurveCollection.h"
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

#include "Riv3dWellLogPlanePartMgr.h"
#include "RivFishbonesSubsPartMgr.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivSectionFlattner.h"
#include "RivWellConnectionFactorPartMgr.h"
#include "RivWellFracturePartMgr.h"
#include "RivWellPathPartMgr.h"
#include "RivWellPathSourceInfo.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
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
RivWellPathPartMgr::RivWellPathPartMgr(RimWellPath* wellPath, Rim3dView* view)
{
    m_rimWellPath = wellPath;
    m_rimView = view;
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
void RivWellPathPartMgr::appendStaticFracturePartsToModel(cvf::ModelBasicList* model)
{
    if (m_rimView.isNull()) return;

    const RimEclipseView* eclView = dynamic_cast<const RimEclipseView*>(m_rimView.p());
    if (!eclView) return;

    if (!m_rimWellPath || !m_rimWellPath->showWellPath() || !m_rimWellPath->fractureCollection()->isChecked()) return;

    for (RimWellPathFracture* f : m_rimWellPath->fractureCollection()->fractures())
    {
        CVF_ASSERT(f);

        f->fracturePartManager()->appendGeometryPartsToModel(model, *eclView);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFishboneSubsPartsToModel(cvf::ModelBasicList* model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        double characteristicCellSize)
{
    if ( !m_rimWellPath || !m_rimWellPath->fishbonesCollection()->isChecked() ) return;

    for (const auto& rimFishboneSubs : m_rimWellPath->fishbonesCollection()->fishbonesSubs() )
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
        for (const auto& lateralDomainCoords : fbWellPath->coordinates())
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
void RivWellPathPartMgr::appendPerforationsToModel(cvf::ModelBasicList* model, 
                                                   size_t timeStepIndex,
                                                   const caf::DisplayCoordTransform* displayCoordTransform, 
                                                   double characteristicCellSize,
                                                   bool doFlatten)
{
    if (!m_rimWellPath || !m_rimWellPath->perforationIntervalCollection()->isChecked()) return;

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    QDateTime currentTimeStamp;
    if (m_rimView)
    {
        RimCase* rimCase = nullptr;
        m_rimView->firstAncestorOrThisOfType(rimCase);

        if (rimCase)
        {
            std::vector<QDateTime> timeStamps = rimCase->timeStepDates();
            if (timeStepIndex < timeStamps.size())
            {
                currentTimeStamp = timeStamps[timeStepIndex];
            }
        }
    }

    // Since we're using the index of measured depths to find the index of a point, ensure they're equal
    CVF_ASSERT(wellPathGeometry->m_measuredDepths.size() == wellPathGeometry->m_wellPathPoints.size());

    double wellPathRadius = this->wellPathRadius(characteristicCellSize, wellPathCollection);
    double perforationRadius = wellPathRadius * 1.1;

    RivPipeGeometryGenerator geoGenerator;
    std::vector<RimPerforationInterval*> perforations;
    m_rimWellPath->descendantsIncludingThisOfType(perforations);
    for (RimPerforationInterval* perforation : perforations)
    {
        using namespace std;

        if (!perforation->isChecked()) continue;
        if (perforation->startMD() > perforation->endMD()) continue;

        if (!perforation->isActiveOnDate(currentTimeStamp)) continue;
        
        double horizontalLengthAlongWellPath = 0.0;
        vector<cvf::Vec3d> perfIntervalCL;
        {
            pair<vector<cvf::Vec3d>, vector<double> >  perfintervalCoordsAndMD = wellPathGeometry->clippedPointSubset(perforation->startMD(),
                                                                                                                      perforation->endMD(),
                                                                                                                      &horizontalLengthAlongWellPath);
            perfIntervalCL = perfintervalCoordsAndMD.first;
        }

        if (perfIntervalCL.size() < 2) continue;


        vector<cvf::Vec3d> perfIntervalCLDiplayCS;
        if ( doFlatten )
        {
            cvf::Vec3d dummy;
            vector<cvf::Mat4d> flatningCSs =
                RivSectionFlattner::calculateFlatteningCSsForPolyline(perfIntervalCL,
                                                                      cvf::Vec3d::Z_AXIS,
                                                                      { horizontalLengthAlongWellPath, 0.0, perfIntervalCL[0].z() },
                                                                      &dummy);

            for ( size_t cIdx = 0; cIdx < perfIntervalCL.size(); ++cIdx )
            {
                auto clpoint = perfIntervalCL[cIdx].getTransformedPoint(flatningCSs[cIdx]);
                perfIntervalCLDiplayCS.push_back( displayCoordTransform->scaleToDisplaySize(clpoint));
            }
        }
        else
        {
            for ( cvf::Vec3d& point : perfIntervalCL )
            { 
                perfIntervalCLDiplayCS.push_back( displayCoordTransform->transformToDisplayCoord(point));
            }
        }

        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(perforation);

        cvf::Collection<cvf::Part> parts;
        geoGenerator.cylinderWithCenterLineParts(&parts, perfIntervalCLDiplayCS, cvf::Color3f::GREEN, perforationRadius);
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
void RivWellPathPartMgr::appendVirtualTransmissibilitiesToModel(cvf::ModelBasicList*              model,
                                                                size_t timeStepIndex,
                                                                const caf::DisplayCoordTransform* displayCoordTransform,
                                                                double                            characteristicCellSize)
{
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(m_rimView.p());
    if (!eclView) return;

    if (!eclView->isVirtualConnectionFactorGeometryVisible()) return;

    RimEclipseCase* eclipseCase = nullptr;
    eclView->firstAncestorOrThisOfType(eclipseCase);
    if (!eclipseCase) return;

    const RigVirtualPerforationTransmissibilities* trans = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (trans)
    {
        m_wellConnectionFactorPartMgr = new RivWellConnectionFactorPartMgr(m_rimWellPath, eclView->virtualPerforationResult());

        m_wellConnectionFactorPartMgr->appendDynamicGeometryPartsToModel(model, timeStepIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// The pipe geometry needs to be rebuilt on scale change to keep the pipes round
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::buildWellPathParts(const caf::DisplayCoordTransform* displayCoordTransform,
                                            double characteristicCellSize, 
                                            const cvf::BoundingBox& wellPathClipBoundingBox, 
                                            bool doFlatten)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    const std::vector<cvf::Vec3d>& wellpathCenterLine = wellPathGeometry->m_wellPathPoints;
     
    if (wellpathCenterLine.size() < 2) return;

    clearAllBranchData();

    double wellPathRadius = this->wellPathRadius(characteristicCellSize, wellPathCollection);

    // Generate the well path geometry as a line and pipe structure

    m_pipeGeomGenerator = new RivPipeGeometryGenerator;

    m_pipeGeomGenerator->setRadius(wellPathRadius);
    m_pipeGeomGenerator->setCrossSectionVertexCount(wellPathCollection->wellPathCrossSectionVertexCount());

    double horizontalLengthAlongWellToClipPoint = 0.0;

    std::vector<cvf::Vec3d> clippedWellPathCenterLine;

    // Skip visualization if outside the domain of this case
    {
        cvf::Vec3d casemax = wellPathClipBoundingBox.max();
        cvf::Vec3d casemin = wellPathClipBoundingBox.min();
        cvf::Vec3d caseext = wellPathClipBoundingBox.extent();

        // Add up to the sealevel
        cvf::BoundingBox relevantWellpathBBox = wellPathClipBoundingBox;
        relevantWellpathBBox.add(cvf::Vec3d(casemax.x(), casemax.y(), 0.0));

        // Add some sideways leeway

        cvf::Vec3d addSize = 3.0*cvf::Vec3d(caseext.x(), caseext.y(), 0.0);
        relevantWellpathBBox.add(casemax + addSize);
        relevantWellpathBBox.add(casemin - addSize);

        if ( !RigWellPath::isPolylineTouchingBBox(wellpathCenterLine, relevantWellpathBBox) )
        {
            return;
        }
    }

    if ( wellPathCollection->wellPathClip )
    {
        double maxZClipHeight =  wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
        clippedWellPathCenterLine =  RigWellPath::clipPolylineStartAboveZ(wellpathCenterLine,
                                                                          maxZClipHeight,
                                                                          &horizontalLengthAlongWellToClipPoint);
    }
    else
    {
        clippedWellPathCenterLine =  wellpathCenterLine;
    }

    if ( clippedWellPathCenterLine.size() < 2 ) return;

    cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray(clippedWellPathCenterLine.size());

    // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.

    if ( doFlatten )
    {
        cvf::Vec3d dummy;
        std::vector<cvf::Mat4d> flatningCSs =
            RivSectionFlattner::calculateFlatteningCSsForPolyline(clippedWellPathCenterLine,
                                                                  cvf::Vec3d::Z_AXIS,
                                                                  { horizontalLengthAlongWellToClipPoint, 0.0, clippedWellPathCenterLine[0].z() },
                                                                  &dummy);

        for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
        {
            auto clpoint = clippedWellPathCenterLine[cIdx].getTransformedPoint(flatningCSs[cIdx]);
            (*cvfCoords)[cIdx] = displayCoordTransform->scaleToDisplaySize(clpoint);
        }
    }
    else
    {
        for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
        {
            (*cvfCoords)[cIdx] = displayCoordTransform->transformToDisplayCoord(clippedWellPathCenterLine[cIdx]);
        }
    }

    m_pipeGeomGenerator->setPipeCenterCoords(cvfCoords.p());
    m_surfaceDrawable = m_pipeGeomGenerator->createPipeSurface();
    m_centerLineDrawable = m_pipeGeomGenerator->createCenterLine();

    if ( m_surfaceDrawable.notNull() )
    {
        m_surfacePart = new cvf::Part;
        m_surfacePart->setDrawable(m_surfaceDrawable.p());

        RivWellPathSourceInfo* sourceInfo = new RivWellPathSourceInfo(m_rimWellPath, m_rimView);
        m_surfacePart->setSourceInfo(sourceInfo);

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWellPath->wellPathColor()), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        m_surfacePart->setEffect(eff.p());
    }

    if ( m_centerLineDrawable.notNull() )
    {
        m_centerLinePart = new cvf::Part;
        m_centerLinePart->setDrawable(m_centerLineDrawable.p());

        caf::MeshEffectGenerator gen(m_rimWellPath->wellPathColor());
        cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

        m_centerLinePart->setEffect(eff.p());
    }

    // Generate label with well-path name

    cvf::Vec3d textPosition = cvfCoords->get(0);
    textPosition.z() += 2.2 * characteristicCellSize; 

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
void RivWellPathPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                          const caf::DisplayCoordTransform* displayCoordTransform,
                                                          double                            characteristicCellSize,
                                                          const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF)
        return;

    if (wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false )
        return;

    // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
    buildWellPathParts(displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox, false);

    if (m_surfacePart.notNull())
    {
        model->addPart(m_surfacePart.p());
    }

    if (m_centerLinePart.notNull())
    {
        model->addPart(m_centerLinePart.p());
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
void RivWellPathPartMgr::appendFlattenedStaticGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                   const caf::DisplayCoordTransform* displayCoordTransform, 
                                                                   double characteristicCellSize, 
                                                                   const cvf::BoundingBox& wellPathClipBoundingBox)
{
    // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
    buildWellPathParts(displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox, true);

    if (m_surfacePart.notNull())
    {
        model->addPart(m_surfacePart.p());
    }

    if (m_centerLinePart.notNull())
    {
        model->addPart(m_centerLinePart.p());
    }

    if (m_wellLabelPart.notNull())
    {
        model->addPart(m_wellLabelPart.p());
    } 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                           size_t                            timeStepIndex,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           double                            characteristicCellSize,
                                                           const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    CVF_ASSERT(model);

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (m_rimWellPath.isNull()) return;

    if (wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    if (wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false)
    {
        return;
    }

    appendPerforationsToModel(model, timeStepIndex, displayCoordTransform, characteristicCellSize, false);
    appendVirtualTransmissibilitiesToModel(model, timeStepIndex, displayCoordTransform, characteristicCellSize);

    if (!m_rimWellPath->rim3dWellLogCurveCollection()) return;
    if (!m_rimWellPath->rim3dWellLogCurveCollection()->showPlot()) return;
    if (m_rimWellPath->vectorOf3dWellLogCurves().empty()) return;

    RimGridView* gridView = dynamic_cast<RimGridView*>(m_rimView.p());
    if (!gridView) return;

    m_3dWellLogCurvePartMgr = new Riv3dWellLogPlanePartMgr(m_rimWellPath, gridView);
    m_3dWellLogCurvePartMgr->append3dWellLogCurvesToModel(model, displayCoordTransform, m_rimWellPath->vectorOf3dWellLogCurves());

    if (m_rimWellPath->rim3dWellLogCurveCollection()->showGrid())
    {
        m_3dWellLogCurvePartMgr->appendGridToModel(model, displayCoordTransform, wellPathClipBoundingBox, 200);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFlattenedDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                    size_t timeStepIndex, 
                                                                    const caf::DisplayCoordTransform* displayCoordTransform, 
                                                                    double characteristicCellSize, 
                                                                    const cvf::BoundingBox& wellPathClipBoundingBox)
{
    CVF_ASSERT(model);

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if (!wellPathCollection) return;

    if (m_rimWellPath.isNull()) return;

    appendPerforationsToModel(model, timeStepIndex, displayCoordTransform, characteristicCellSize, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::clearAllBranchData()
{
    m_pipeGeomGenerator = nullptr;
    m_surfacePart = nullptr;
    m_surfaceDrawable = nullptr;
    m_centerLinePart = nullptr;
    m_centerLineDrawable = nullptr;
    m_wellLabelPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivWellPathPartMgr::segmentIndexFromTriangleIndex(size_t triangleIndex)
{
    return m_pipeGeomGenerator->segmentIndexFromTriangleIndex(triangleIndex);
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
