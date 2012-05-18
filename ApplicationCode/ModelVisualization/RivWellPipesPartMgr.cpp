/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


#include "cvfLibCore.h"

#include "RimReservoir.h"
#include "RigReservoir.h"
#include "RivWellPipesPartMgr.h"
#include "RigCell.h"
#include "RivPipeGeometryGenerator.h"
#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfDrawableGeo.h"
#include "cvfRay.h"
#include "cafEffectGenerator.h"
#include "RimReservoirView.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPipesPartMgr::RivWellPipesPartMgr(RimReservoirView* reservoirView, RimWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell      = well;
    m_needsTransformUpdate = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPipesPartMgr::~RivWellPipesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::buildWellPipeParts()
{
    if (m_rimReservoirView.isNull()) return;

    m_wellBranches.clear();

    std::vector< size_t > pipeBranchIds;
    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <CellId> > pipeBranchesCellIds;

    calculateWellPipeCenterline( pipeBranchIds, pipeBranchesCLCoords, pipeBranchesCellIds);

    double characteristicCellSize = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid()->characteristicCellSize();
    double pipeRadius = m_rimReservoirView->wellCollection()->pipeRadiusScaleFactor() *m_rimWell->pipeRadiusScaleFactor() * characteristicCellSize;

    for (size_t brIdx = 0; brIdx < pipeBranchIds.size(); ++brIdx)
    {
        m_wellBranches.push_back(RivPipeBranchData());
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_branchId = pipeBranchIds[brIdx];
        pbd.m_cellIds = pipeBranchesCellIds[brIdx];

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(pipeRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(12);
        pbd.m_pipeGeomGenerator->setPipeColor( m_rimWell->wellPipeColor());

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        cvfCoords->assign(pipeBranchesCLCoords[brIdx]);
        
        // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.
        const RigMainGrid* mainGrid = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid();

        for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
        {
            cvf::Vec4d transfCoord = m_scaleTransform->worldTransform()* cvf::Vec4d((*cvfCoords)[cIdx] - mainGrid->displayModelOffset(), 1);
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

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWell->wellPipeColor()), true);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateEffect();

            pbd.m_surfacePart->setEffect(eff.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part;
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWell->wellPipeColor());
            cvf::ref<cvf::Effect> eff = gen.generateEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }
    }

    m_needsTransformUpdate = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::calculateWellPipeCenterline(  std::vector< size_t >& pipeBranchIds, 
                                                        std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                        std::vector< std::vector <CellId> >& pipeBranchesCellIds) const
{
    CVF_ASSERT(m_rimWell.notNull());
    CVF_ASSERT(m_rimReservoirView.notNull());
    
    RigReservoir*   rigReservoir = m_rimReservoirView->eclipseCase()->reservoirData();
    RigWellResults* wellResults = m_rimWell->wellResults();

    const RigWellResultFrame& staticWellFrame = m_rimWell->wellResults()->m_staticWellCells;

    // Make sure we have computed the static representation of the well
    if (staticWellFrame.m_wellResultBranches.size() == 0)
    {
        wellResults->computeStaticWellCellPath();
    }
    if (staticWellFrame.m_wellResultBranches.size() == 0) return;

    // Initialize the return arrays
    pipeBranchIds.clear();
    pipeBranchesCLCoords.clear();
    pipeBranchesCellIds.clear();

    // Well head
    // Match this position with well head position in RivWellHeadPartMgr::buildWellHeadParts()
    const RigCell& whCell = rigReservoir->cellFromWellResultCell(staticWellFrame.m_wellHead);
    cvf::Vec3d whStartPos = whCell.faceCenter(cvf::StructGridInterface::NEG_K);
    const RigWellResultCell* whResCell = &(staticWellFrame.m_wellHead);

    // Loop over all the well branches
    const std::vector<RigWellResultBranch>& resBranches = staticWellFrame.m_wellResultBranches;

    for (size_t brIdx = 0; brIdx < resBranches.size(); brIdx++)
    {
        if (resBranches[brIdx].m_wellCells.size() == 0) continue; // Skip empty branches. Do not know why they exist, but they make problems.

        // Create a new branch
        pipeBranchIds.push_back(resBranches[brIdx].m_branchNumber);
        pipeBranchesCLCoords.push_back(std::vector<cvf::Vec3d>());
        pipeBranchesCellIds.push_back(std::vector <CellId>());
        
        // We start by entering the first cell (the wellhead)
        const RigWellResultCell* prevResCell = whResCell;
        pipeBranchesCLCoords.back().push_back(whStartPos);
        pipeBranchesCellIds.back().push_back(CellId(prevResCell->m_gridIndex, prevResCell->m_gridCellIndex));

        // Add extra coordinate between cell face and cell center 
        // to make sure the well pipe terminated in a segment parallel to z-axis
        cvf::Vec3d whIntermediate = whStartPos;
        whIntermediate.z() = (whStartPos.z() + whCell.center().z()) / 2.0;
        pipeBranchesCLCoords.back().push_back(whIntermediate);
        pipeBranchesCellIds.back().push_back(CellId(prevResCell->m_gridIndex, prevResCell->m_gridCellIndex));


        // Loop over all the resultCells in the branch
        const std::vector<RigWellResultCell>& resBranchCells = resBranches[brIdx].m_wellCells;

        for (size_t cIdx = 0; cIdx < resBranchCells.size(); cIdx++)
        {
            std::vector<cvf::Vec3d>& branchCLCoords = pipeBranchesCLCoords.back();
            std::vector<CellId>&     branchCellIds  = pipeBranchesCellIds.back();

            const RigWellResultCell& resCell = resBranchCells[cIdx];
            const RigCell& cell = rigReservoir->cellFromWellResultCell(resCell);

            // Check if this and the previous cells has shared faces

            cvf::StructGridInterface::FaceType sharedFace;
            if (rigReservoir->findSharedSourceFace(sharedFace, resCell, *prevResCell))
            {
                branchCLCoords.push_back(cell.faceCenter(sharedFace));
                branchCellIds.push_back(CellId(resCell.m_gridIndex, resCell.m_gridCellIndex));
            }
            else
            {
                // This and the previous cell does not share a face. We need to go out of the previous cell, before entering this.
                const RigCell& prevCell =  rigReservoir->cellFromWellResultCell(*prevResCell);
                cvf::Vec3d centerPrevCell = prevCell.center();
                cvf::Vec3d centerThisCell = cell.center();

                // First make sure this cell is not starting a new "display" branch 
                if ( (prevResCell == whResCell) || (centerThisCell-centerPrevCell).lengthSquared() <= (centerThisCell - whStartPos).lengthSquared())
                {
                    // Create ray and intersect with cells

                    cvf::Ray rayToThisCell;
                    rayToThisCell.setOrigin(centerPrevCell);
                    rayToThisCell.setDirection((centerThisCell - centerPrevCell).getNormalized());

                    cvf::Vec3d outOfPrevCell(centerPrevCell);
                    cvf::Vec3d intoThisCell(centerThisCell);

                    bool intersectionOk = prevCell.firstIntersectionPoint(rayToThisCell, &outOfPrevCell);
                    //CVF_ASSERT(intersectionOk);
                    intersectionOk = cell.firstIntersectionPoint(rayToThisCell, &intoThisCell);
                    //CVF_ASSERT(intersectionOk);
                    if ((intoThisCell - outOfPrevCell).lengthSquared() > 1e-3)
                    {
                        branchCLCoords.push_back(outOfPrevCell);
                        branchCellIds.push_back(CellId(cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T));
                    }
                    branchCLCoords.push_back(intoThisCell);
                    branchCellIds.push_back(CellId(resCell.m_gridIndex, resCell.m_gridCellIndex));
                }
                else
                {
                    // This cell is further from the previous cell than from the well head, 
                    // thus we interpret it as a new branch.

                    // First finish the current branch
                    branchCLCoords.push_back(branchCLCoords.back() + 1.5*(centerPrevCell - branchCLCoords.back())  );

                    // Create new display branch
                    pipeBranchIds.push_back(resBranches[brIdx].m_branchNumber);
                    pipeBranchesCLCoords.push_back(std::vector<cvf::Vec3d>());
                    pipeBranchesCellIds.push_back(std::vector <CellId>());

                    // Start the new branch by entering the first cell (the wellhead) and intermediate
                    prevResCell = whResCell;
                    pipeBranchesCLCoords.back().push_back(whStartPos);
                    pipeBranchesCellIds.back().push_back(CellId(prevResCell->m_gridIndex, prevResCell->m_gridCellIndex));

                    // Include intermediate
                    pipeBranchesCLCoords.back().push_back(whIntermediate);
                    pipeBranchesCellIds.back().push_back(CellId(prevResCell->m_gridIndex, prevResCell->m_gridCellIndex));

                    // Well now we need to step one back to take this cell again, but in the new branch.
                    cIdx--;
                    continue;
                }
            }

            prevResCell = &resCell;

            // If we are looking at last cell, add the point 0.5 past the center of that cell
            if (cIdx == resBranchCells.size() -1) 
            {
                 branchCLCoords.push_back(branchCLCoords.back() + 1.5*(cell.center() - branchCLCoords.back())  );
            }
        }
    }

    CVF_ASSERT(pipeBranchesCellIds.size() == pipeBranchesCLCoords.size());
    for (size_t i = 0 ; i < pipeBranchesCellIds.size() ; ++i)
    {
        CVF_ASSERT(pipeBranchesCellIds[i].size() == pipeBranchesCLCoords[i].size()-1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimReservoirView.isNull()) return;
    if (m_rimWell.isNull()) return;

    if (   m_rimReservoirView->wellCollection()->wellPipeVisibility() != RimWellCollection::FORCE_ALL_ON 
        && m_rimWell->showWellPipes() == false) return;

    if (   m_rimWell->wellResults()->firstResultTimeStep() == cvf::UNDEFINED_SIZE_T 
        || frameIndex < m_rimWell->wellResults()->firstResultTimeStep() ) 
        return;

    if (m_needsTransformUpdate) buildWellPipeParts();

    std::list<RivPipeBranchData>::iterator it;
    for (it = m_wellBranches.begin(); it != m_wellBranches.end(); it++)
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
void RivWellPipesPartMgr::updatePipeResultColor(size_t frameIndex)
{
    if (m_rimWell == NULL) return;

    RigWellResults* wRes = m_rimWell->wellResults();
    if (wRes == NULL) return;

    if (frameIndex < wRes->firstResultTimeStep()) return; // Or reset colors or something

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
    scalarMapper->setLevelsFromColorCount(4);

    const double closed = -0.1, producing = 1.5, water = 2.5, hcInjection = 3.5; // Closed set to -0.1 instead of 0.5 to workaround bug in the scalar mapper.

    std::list<RivPipeBranchData>::iterator brIt;
    const RigWellResultFrame& wResFrame = wRes->wellResultFrame(frameIndex);

    std::vector<double> wellCellStates;

    for (brIt = m_wellBranches.begin(); brIt != m_wellBranches.end(); ++brIt)
    {
        size_t branchId = brIt->m_branchId;

        // find corresponding result branch
        size_t i;
        for ( i = 0; i < wResFrame.m_wellResultBranches.size(); ++i)
        {
            if (wResFrame.m_wellResultBranches[i].m_branchNumber == branchId) break;
        }

        // Initialize well states to "closed" state
        wellCellStates.clear();
        wellCellStates.resize(brIt->m_cellIds.size(), closed);

        // Find result values
        if (i >= wResFrame.m_wellResultBranches.size())
        {
            // Branch not found in results. Do nothing. Keep the "closed" states
        }
        else
        {
            const std::vector<RigWellResultCell>& resBranch = wResFrame.m_wellResultBranches[i].m_wellCells;
            const std::vector <CellId>& cellIds =  brIt->m_cellIds;

            // Find cellIds[wcIdx] in resBranch
            //   ifFound then give color from cell state
            //   else give closed color

            // The search can be simplified to comparing the current element against 
            // the first unmatched element in the result branch vector since the result is (supposed) to be 
            // a subset of the complete branch where all elements have the same order

            size_t rcIdx = 0; // index to first unmatched element in results 
            for (size_t wcIdx = 0; wcIdx < cellIds.size(); ++wcIdx)
            {
                if (rcIdx >= resBranch.size()) 
                {
                    // We are beyond the result cells. This well cell is closed.
                }
                else
                {
                    if (   cellIds[wcIdx].gridIndex == resBranch[rcIdx].m_gridIndex
                        && cellIds[wcIdx].cellIndex == resBranch[rcIdx].m_gridCellIndex)
                    {
                        double cellState = closed;
                        
                        if (resBranch[rcIdx].m_isOpen)
                        {
                            switch (wResFrame.m_productionType)
                            {
                            case RigWellResultFrame::PRODUCER:
                                cellState = producing;
                                break;
                            case RigWellResultFrame::OIL_INJECTOR:
                                cellState = hcInjection;
                                break;
                            case RigWellResultFrame::GAS_INJECTOR:
                                cellState = hcInjection;
                                break;
                            case RigWellResultFrame::WATER_INJECTOR:
                                cellState = water;
                                break;
                            case RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE:
                                cellState = closed;
                                break;
                           }
                        }
                       
                        wellCellStates[wcIdx] = cellState;
                        rcIdx++;
                    }
                }
            }
        }

        // Find or create texture coords array for pipe surface

        if (brIt->m_surfaceDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> surfTexCoords = const_cast<cvf::Vec2fArray*>(brIt->m_surfaceDrawable->textureCoordArray());

            if (surfTexCoords.isNull())
            {
                surfTexCoords = new cvf::Vec2fArray;
            }

            brIt->m_pipeGeomGenerator->pipeSurfaceTextureCoords( surfTexCoords.p(), wellCellStates, scalarMapper.p());
            brIt->m_surfaceDrawable->setTextureCoordArray( surfTexCoords.p());

            caf::ScalarMapperEffectGenerator surfEffGen(scalarMapper.p(), true);
            cvf::ref<cvf::Effect> seff = surfEffGen.generateEffect();

            brIt->m_surfacePart->setEffect(seff.p());
        }

        // Find or create texture coords array for pipe center line
        if (brIt->m_centerLineDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>(brIt->m_centerLineDrawable->textureCoordArray());

            if (lineTexCoords.isNull())
            {
                lineTexCoords = new cvf::Vec2fArray;
            }

            // Calculate new texture coordinates
            brIt->m_pipeGeomGenerator->centerlineTextureCoords( lineTexCoords.p(), wellCellStates, scalarMapper.p());

            // Set the new texture coordinates

            brIt->m_centerLineDrawable->setTextureCoordArray( lineTexCoords.p());

            // Set effects

            caf::ScalarMapperMeshEffectGenerator meshEffGen(scalarMapper.p());
            cvf::ref<cvf::Effect> meff = meshEffGen.generateEffect();

            brIt->m_centerLinePart->setEffect(meff.p());
        }
    }
}

