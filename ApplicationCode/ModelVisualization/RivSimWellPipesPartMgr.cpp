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

#include "RivSimWellPipesPartMgr.h"

#include "RiaColorTables.h"
#include "RiaExtractionTools.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimVirtualPerforationResults.h"

#include "RivPipeGeometryGenerator.h"
#include "RivSectionFlattner.h"
#include "RivSimWellConnectionSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivWellConnectionFactorGeometryGenerator.h"
#include "RivWellConnectionSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::RivSimWellPipesPartMgr(RimSimWellInView* well)
    : m_rimWell(well)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::~RivSimWellPipesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dView* RivSimWellPipesPartMgr::viewWithSettings()
{
    Rim3dView* view = nullptr;
    if (m_rimWell) m_rimWell->firstAncestorOrThisOfType(view);
    
    return view;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                               size_t frameIndex, 
                                                               const caf::DisplayCoordTransform* displayXf)
{
    if (!viewWithSettings()) return;

    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    buildWellPipeParts(displayXf, false, 0.0, -1, frameIndex);

    std::list<RivPipeBranchData>::iterator it;
    for (it = m_wellBranches.begin(); it != m_wellBranches.end(); ++it)
    {
        if (it->m_surfacePart.notNull())
        {
            model->addPart(it->m_surfacePart.p());
        }

        if (it->m_centerLinePart.notNull())
        {
            model->addPart(it->m_centerLinePart.p());
        }

        if (it->m_connectionFactorsPart.notNull())
        {
            model->addPart(it->m_connectionFactorsPart.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendFlattenedDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                                        size_t frameIndex, 
                                                                        const caf::DisplayCoordTransform* displayXf, 
                                                                        double flattenedIntersectionExtentLength,
                                                                        int branchIndex)
{
    if (!viewWithSettings()) return;

    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    buildWellPipeParts(displayXf, true, flattenedIntersectionExtentLength, branchIndex, frameIndex);

    std::list<RivPipeBranchData>::iterator it;
    for (it = m_wellBranches.begin(); it != m_wellBranches.end(); ++it)
    {
        if (it->m_surfacePart.notNull())
        {
            model->addPart(it->m_surfacePart.p());
        }

        if (it->m_centerLinePart.notNull())
        {
            model->addPart(it->m_centerLinePart.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::buildWellPipeParts(const caf::DisplayCoordTransform* displayXf,
                                                bool doFlatten, 
                                                double flattenedIntersectionExtentLength,
                                                int branchIndex,
                                                size_t frameIndex)
{
    if (!this->viewWithSettings()) return;

    m_wellBranches.clear();
    m_flattenedBranchWellHeadOffsets.clear();
    m_pipeBranchesCLCoords.clear();
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    m_rimWell->calculateWellPipeStaticCenterLine(m_pipeBranchesCLCoords, pipeBranchesCellIds);

    double pipeRadius =  m_rimWell->pipeRadius();
    int crossSectionVertexCount = m_rimWell->pipeCrossSectionVertexCount();

    // Take branch selection into account
    size_t branchIdxStart = 0;
    size_t branchIdxStop = pipeBranchesCellIds.size();
    if (m_pipeBranchesCLCoords.size() > 1)
    {
        if (branchIndex >= 0 && branchIndex < static_cast<int>(branchIdxStop))
        {
            branchIdxStart = branchIndex;
            branchIdxStop  = branchIdxStart + 1;
        }
    }

    cvf::Vec3d flattenedStartOffset = cvf::Vec3d::ZERO;
    if ( m_pipeBranchesCLCoords.size() > branchIdxStart && m_pipeBranchesCLCoords[branchIdxStart].size() )
    {
        flattenedStartOffset = { 0.0, 0.0, m_pipeBranchesCLCoords[branchIdxStart][0].z() }; 
    }

    for (size_t brIdx = branchIdxStart; brIdx <branchIdxStop; ++brIdx)
    {
        cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo(m_rimWell, brIdx);

        m_wellBranches.push_back(RivPipeBranchData());
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_cellIds = pipeBranchesCellIds[brIdx];

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(pipeRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(crossSectionVertexCount);

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        cvfCoords->assign(m_pipeBranchesCLCoords[brIdx]);
        
        flattenedStartOffset.z() = m_pipeBranchesCLCoords[brIdx][0].z();
        
        m_flattenedBranchWellHeadOffsets.push_back(flattenedStartOffset.x());

        if (doFlatten)
        {        
            std::vector<cvf::Mat4d> flatningCSs = RivSectionFlattner::calculateFlatteningCSsForPolyline(m_pipeBranchesCLCoords[brIdx],
                                                                                                        cvf::Vec3d::Z_AXIS,
                                                                                                        flattenedStartOffset,
                                                                                                        &flattenedStartOffset);
            for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
            {
                (*cvfCoords)[cIdx] = ((*cvfCoords)[cIdx]).getTransformedPoint(flatningCSs[cIdx]);
                (*cvfCoords)[cIdx] = displayXf->scaleToDisplaySize((*cvfCoords)[cIdx]);
            }
        }
        else
        {
            // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.

            for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
            {
                (*cvfCoords)[cIdx] = displayXf->transformToDisplayCoord((*cvfCoords)[cIdx]);
            }
        }

        pbd.m_pipeGeomGenerator->setPipeCenterCoords(cvfCoords.p());
        pbd.m_surfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        pbd.m_centerLineDrawable = pbd.m_pipeGeomGenerator->createCenterLine();

        if (pbd.m_surfaceDrawable.notNull())
        {
            pbd.m_surfacePart = new cvf::Part(0,"SimWellPipeSurface");
            pbd.m_surfacePart->setDrawable(pbd.m_surfaceDrawable.p());

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWell->wellPipeColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect(eff.p());
            
            pbd.m_surfacePart->setSourceInfo(sourceInfo.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part(0,"SimWellPipeCenterLine");
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWell->wellPipeColor());
            cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }

        // Create slightly larger geometry for active (open) wells
        // This will avoid visual artifacts when two wells are located at the same position
        {
            pbd.m_pipeGeomGenerator->setRadius(pipeRadius * 1.1);
            pbd.m_largeSurfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        }

        pbd.m_connectionFactorGeometryGenerator = nullptr;
        pbd.m_connectionFactorsPart = nullptr;

        RimEclipseView* eclipseView = nullptr;
        m_rimWell->firstAncestorOrThisOfType(eclipseView);

        if (eclipseView && eclipseView->isVirtualConnectionFactorGeometryVisible())
        {
            RigSimWellData* simWellData = m_rimWell->simWellData();

            if (simWellData && simWellData->hasWellResult(frameIndex))
            {
                const RigWellResultFrame& wResFrame = simWellData->wellResultFrame(frameIndex);

                std::vector<CompletionVizData> completionVizDataItems;

                RimVirtualPerforationResults* virtualPerforationResult = eclipseView->virtualPerforationResult();
                {
                    auto wellPaths = m_rimWell->wellPipeBranches();

                    const RigWellPath* wellPath = wellPaths[brIdx];

                    RigEclipseWellLogExtractor* extractor = RiaExtractionTools::findOrCreateSimWellExtractor(m_rimWell, wellPath);
                    if (extractor)
                    {
                        std::vector<WellPathCellIntersectionInfo> wellPathCellIntersections = extractor->cellIntersectionInfosAlongWellPath();

                        for (const auto& intersectionInfo : wellPathCellIntersections)
                        {
                            size_t globalCellIndex = intersectionInfo.globCellIndex;

                            for (const auto& wellResultPoint : pbd.m_cellIds)
                            {
                                if (wellResultPoint.m_gridCellIndex == globalCellIndex)
                                {
                                    double startMD = intersectionInfo.startMD;
                                    double endMD = intersectionInfo.endMD;

                                    double middleMD = (startMD + endMD) / 2.0;

                                    cvf::Vec3d defaultLocationInDomainCoord = wellPath->interpolatedPointAlongWellPath(middleMD);

                                    cvf::Vec3d p1;
                                    cvf::Vec3d p2;
                                    wellPath->twoClosestPoints(defaultLocationInDomainCoord, &p1, &p2);

                                    cvf::Vec3d defaultWellPathDirection = (p2 - p1).getNormalized();

                                    cvf::Vec3d anchor = displayXf->transformToDisplayCoord(defaultLocationInDomainCoord);;

                                    const RigWellResultPoint* wResCell = wResFrame.findResultCell(wellResultPoint.m_gridIndex, wellResultPoint.m_gridCellIndex);
                                    if (wResCell->isValid())
                                    {
                                        CompletionVizData data(anchor, defaultWellPathDirection, wResCell->connectionFactor(), globalCellIndex);

                                        completionVizDataItems.push_back(data);
                                    }
                                }
                            }
                        }
                    }
                }

                if (!completionVizDataItems.empty())
                {
                    double radius = pipeRadius * virtualPerforationResult->geometryScaleFactor();
                    radius *= 2.0; // Enlarge the radius slightly to make the connection factor visible if geometry scale factor is set to 1.0

                    pbd.m_connectionFactorGeometryGenerator = new RivWellConnectionFactorGeometryGenerator(completionVizDataItems, radius);

                    cvf::ScalarMapper* scalarMapper = virtualPerforationResult->legendConfig()->scalarMapper();
                    cvf::ref<cvf::Part> part = pbd.m_connectionFactorGeometryGenerator->createSurfacePart(scalarMapper, eclipseView->isLightingDisabled());
                    if (part.notNull())
                    {
                        cvf::ref<RivSimWellConnectionSourceInfo> sourceInfo = new RivSimWellConnectionSourceInfo(m_rimWell, pbd.m_connectionFactorGeometryGenerator.p());
                        part->setSourceInfo(sourceInfo.p());
                    }

                    pbd.m_connectionFactorsPart = part;
                }
            }
        }

        if (doFlatten) flattenedStartOffset += { 2*flattenedIntersectionExtentLength, 0.0, 0.0};
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::updatePipeResultColor(size_t frameIndex)
{
    if (m_rimWell == nullptr) return;

    RigSimWellData* simWellData = m_rimWell->simWellData();
    if (simWellData == nullptr) return;

    if (!simWellData->hasWellResult(frameIndex)) return; // Or reset colors or something

    const double defaultState       = -0.1; // Closed set to -0.1 instead of 0.5 to workaround bug in the scalar mapper.
    const double producerState      = 1.5;
    const double waterInjectorState = 2.5;
    const double hcInjectorState    = 3.5;
    const double closedState        = 4.5;

    const RigWellResultFrame& wResFrame = simWellData->wellResultFrame(frameIndex);


    // Setup a scalar mapper
    cvf::ref<cvf::ScalarMapperDiscreteLinear> scalarMapper = new cvf::ScalarMapperDiscreteLinear;
    {
        cvf::Color3ubArray legendColors;
        legendColors.resize(5);
        legendColors[0] = cvf::Color3ub(m_rimWell->wellPipeColor());
        legendColors[1] = cvf::Color3::GREEN;
        legendColors[2] = cvf::Color3::BLUE;
        legendColors[3] = cvf::Color3::RED;
        legendColors[4] = cvf::Color3ub(RiaColorTables::undefinedCellColor());
        scalarMapper->setColors(legendColors);
        scalarMapper->setRange(0.0, 5.0);
        scalarMapper->setLevelCount(5, true);
    }

    caf::ScalarMapperEffectGenerator surfEffGen(scalarMapper.p(), caf::PO_1);

    if (viewWithSettings() && viewWithSettings()->isLightingDisabled())
    {
        surfEffGen.disableLighting(true);
    }

    cvf::ref<cvf::Effect> scalarMapperSurfaceEffect = surfEffGen.generateUnCachedEffect();

    caf::ScalarMapperMeshEffectGenerator meshEffGen(scalarMapper.p());
    cvf::ref<cvf::Effect> scalarMapperMeshEffect = meshEffGen.generateUnCachedEffect();

    for (auto& wellBranch : m_wellBranches)
    {
        std::vector<double> wellCellStates;
        wellCellStates.resize(wellBranch.m_cellIds.size(), defaultState);

        RimSimWellInViewCollection* wellColl = nullptr;
        if (m_rimWell)
        {
            m_rimWell->firstAncestorOrThisOfType(wellColl);
        }

        if (wellColl && wellColl->showConnectionStatusColors())
        {
            const std::vector <RigWellResultPoint>& cellIds = wellBranch.m_cellIds;
            for (size_t wcIdx = 0; wcIdx < cellIds.size(); ++wcIdx)
            {
                // we need a faster lookup, I guess
                const RigWellResultPoint* wResCell = nullptr;
            
                if (cellIds[wcIdx].isCell())
                {
                    wResCell = wResFrame.findResultCell(cellIds[wcIdx].m_gridIndex, cellIds[wcIdx].m_gridCellIndex);
                }

                if (wResCell) 
                {
                    double cellState = defaultState;

                    if (wResCell->m_isOpen)
                    {
                        switch (wResFrame.m_productionType)
                        {
                        case RigWellResultFrame::PRODUCER:
                            cellState = producerState;
                            break;
                        case RigWellResultFrame::OIL_INJECTOR:
                            cellState = hcInjectorState;
                            break;
                        case RigWellResultFrame::GAS_INJECTOR:
                            cellState = hcInjectorState;
                            break;
                        case RigWellResultFrame::WATER_INJECTOR:
                            cellState = waterInjectorState;
                            break;
                        case RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE:
                            cellState = defaultState;
                            break;
                        }
                    }
                    else
                    {
                        cellState = closedState;
                    }

                    wellCellStates[wcIdx] = cellState;
                }
            }
        }

        // Find or create texture coords array for pipe surface

        if (wellBranch.m_surfaceDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> surfTexCoords = const_cast<cvf::Vec2fArray*>(wellBranch.m_surfaceDrawable->textureCoordArray());
            if (surfTexCoords.isNull())
            {
                surfTexCoords = new cvf::Vec2fArray;
            }

            wellBranch.m_pipeGeomGenerator->pipeSurfaceTextureCoords(surfTexCoords.p(), wellCellStates, scalarMapper.p());
            
            wellBranch.m_surfaceDrawable->setTextureCoordArray(surfTexCoords.p());
            wellBranch.m_largeSurfaceDrawable->setTextureCoordArray(surfTexCoords.p());

            if (wResFrame.m_isOpen)
            {
                // Use slightly larger geometry for open wells to avoid z-fighting when two wells are located at the same position
    
                wellBranch.m_surfacePart->setDrawable(wellBranch.m_largeSurfaceDrawable.p());
            }
            else
            {
                wellBranch.m_surfacePart->setDrawable(wellBranch.m_surfaceDrawable.p());
            }
            
            wellBranch.m_surfacePart->setEffect(scalarMapperSurfaceEffect.p());
        }

        // Find or create texture coords array for pipe center line
        if (wellBranch.m_centerLineDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>(wellBranch.m_centerLineDrawable->textureCoordArray());

            if (lineTexCoords.isNull())
            {
                lineTexCoords = new cvf::Vec2fArray;
            }

            // Calculate new texture coordinates
            wellBranch.m_pipeGeomGenerator->centerlineTextureCoords( lineTexCoords.p(), wellCellStates, scalarMapper.p());

            // Set the new texture coordinates

            wellBranch.m_centerLineDrawable->setTextureCoordArray( lineTexCoords.p());

            // Set effects

            wellBranch.m_centerLinePart->setEffect(scalarMapperMeshEffect.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RivSimWellPipesPartMgr::flattenedBranchWellHeadOffsets()
{
    return m_flattenedBranchWellHeadOffsets;
}

