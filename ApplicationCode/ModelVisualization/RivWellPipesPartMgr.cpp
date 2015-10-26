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

#include "RivWellPipesPartMgr.h"

#include "cvfLibCore.h"
#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfDrawableGeo.h"
#include "cvfRay.h"

#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "RigCaseData.h"
#include "RigCell.h"
#include "RivPipeGeometryGenerator.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include "RimEclipseCellColors.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPipesPartMgr::RivWellPipesPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell      = well;
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

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    calculateWellPipeCenterline(pipeBranchesCLCoords, pipeBranchesCellIds);

    double characteristicCellSize = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
    double pipeRadius = m_rimReservoirView->wellCollection()->pipeRadiusScaleFactor() *m_rimWell->pipeRadiusScaleFactor() * characteristicCellSize;

    for (size_t brIdx = 0; brIdx < pipeBranchesCellIds.size(); ++brIdx)
    {
        m_wellBranches.push_back(RivPipeBranchData());
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_cellIds = pipeBranchesCellIds[brIdx];

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(pipeRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());
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

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWell->wellPipeColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect(eff.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part;
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWell->wellPipeColor());
            cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }
    }

    m_needsTransformUpdate = false;
}


//--------------------------------------------------------------------------------------------------
/// Based on the points and cells, calculate a pipe centerline
/// The returned CellIds is one less than the number of centerline points,
/// and are describing the lines between the points, starting with the first line
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::calculateWellPipeCenterline(  std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                        std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds) const
{
    CVF_ASSERT(m_rimWell.notNull());
    CVF_ASSERT(m_rimReservoirView.notNull());

    bool isAutoDetectBranches = m_rimReservoirView->wellCollection()->isAutoDetectingBranches();
    
    RigCaseData*   rigReservoir = m_rimReservoirView->eclipseCase()->reservoirData();
    RigSingleWellResultsData* wellResults = m_rimWell->wellResults();

    // Make sure we have computed the static representation of the well
    if (wellResults->m_staticWellCells.m_wellResultBranches.size() == 0)
    {
        wellResults->computeStaticWellCellPath();
    }

    const RigWellResultFrame& staticWellFrame = wellResults->m_staticWellCells;
    if (staticWellFrame.m_wellResultBranches.size() == 0) return;
    
    // Initialize the return arrays
    pipeBranchesCLCoords.clear();
    pipeBranchesCellIds.clear();

    // Well head
    // Match this position with well head position in RivWellHeadPartMgr::buildWellHeadParts()
    const RigCell& whCell = rigReservoir->cellFromWellResultCell(staticWellFrame.m_wellHead);
    cvf::Vec3d whStartPos = whCell.faceCenter(cvf::StructGridInterface::NEG_K);
    const RigWellResultPoint* whResCell = &(staticWellFrame.m_wellHead);

    // Loop over all the well branches
    const std::vector<RigWellResultBranch>& resBranches = staticWellFrame.m_wellResultBranches;
    bool hasResultCells = false;
    if (resBranches.size())
    {
        for (size_t i = 0 ; i < resBranches.size(); ++i)
        {
            if (resBranches[i].m_branchResultPoints.size() != 0)
            {
                hasResultCells = true;
                break;
            }
        }
    }

    if (hasResultCells)
    {

        // Add extra coordinate between cell face and cell center 
        // to make sure the well pipe terminated in a segment parallel to z-axis
        cvf::Vec3d whIntermediate = whStartPos;
        whIntermediate.z() = (whStartPos.z() + whCell.center().z()) / 2.0;

        const RigWellResultPoint* prevWellResPoint = NULL;

        CVF_ASSERT(wellResults->isMultiSegmentWell() ||  resBranches.size() <= 1);

        // The centerline is calculated by adding a point when the pipe enters a cell, 
        // and one when the line leaves the cell.
        // For the sake of the loop:
        // The currentResultPoint (Cell) and the one we index by the loop variable is the one we calculate the entry point to.
        // The previous cell is the one we leave, and calculate the "out-point" from 


        for (size_t brIdx = 0; brIdx < resBranches.size(); brIdx++)
        {

            // Skip empty branches. Do not know why they exist, but they make problems.

            bool hasValidData = false;
            for (size_t cIdx = 0; cIdx < resBranches[brIdx].m_branchResultPoints.size(); ++cIdx)
            {
                if (resBranches[brIdx].m_branchResultPoints[cIdx].isValid())
                {
                    hasValidData = true;
                    break;
                }
            }
            if (!hasValidData) continue;


            prevWellResPoint = NULL;

            // Find the start the MSW well-branch centerline. Normal wells are started "once" at wellhead in the code above 

            pipeBranchesCLCoords.push_back(std::vector<cvf::Vec3d>());
            pipeBranchesCellIds.push_back(std::vector <RigWellResultPoint>());

            if (brIdx == 0)
            {
                // The first branch contains segment number 1, and this is the only segment connected to well head
                // See Eclipse documentation for the keyword WELSEGS
                prevWellResPoint = whResCell;

                pipeBranchesCLCoords.back().push_back(whStartPos);
                pipeBranchesCellIds.back().push_back(*prevWellResPoint);

                pipeBranchesCLCoords.back().push_back(whIntermediate);
                pipeBranchesCellIds.back().push_back(*prevWellResPoint);
            }

            // Loop over all the resultPoints in the branch

            const std::vector<RigWellResultPoint>& resBranchCells = resBranches[brIdx].m_branchResultPoints;

            for (int cIdx = 0; cIdx < static_cast<int>(resBranchCells.size()); cIdx++) // Need int because cIdx can temporarily end on -1
            {
                std::vector<cvf::Vec3d>&        branchCLCoords = pipeBranchesCLCoords.back();
                std::vector<RigWellResultPoint>& branchCellIds  = pipeBranchesCellIds.back();

                const RigWellResultPoint& currentWellResPoint = resBranchCells[cIdx];

                // Ignore invalid cells

                if (!currentWellResPoint.isValid())
                {
                    //CVF_ASSERT(false); // Some segments does not get anything yet.
                    continue;
                }

                // Add cl contribution for a geometrical resultPoint by adding exit point from previous cell, 
                // and then the result point position 

                if (!currentWellResPoint.isCell()) 
                {
                    // Use the interpolated value of branch head
                    CVF_ASSERT(currentWellResPoint.isPointValid());

                    cvf::Vec3d currentPoint = currentWellResPoint.m_bottomPosition;

                    // If we have a real previous cell, we need to go out of it, before adding the current point
                    // That is: add a CL-point describing where it leaves the previous cell.

                    if (prevWellResPoint && prevWellResPoint->isCell())
                    {
                        // Create ray between the previous and this position

                        const RigCell& prevCell = rigReservoir->cellFromWellResultCell(*prevWellResPoint);
                        cvf::Vec3d centerPreviousCell = prevCell.center();

                        cvf::Ray rayToThisCell;
                        rayToThisCell.setOrigin(centerPreviousCell);
                        rayToThisCell.setDirection((currentPoint - centerPreviousCell).getNormalized());

                        cvf::Vec3d outOfPrevCell(centerPreviousCell);

                        //int intersectionOk = prevCell.firstIntersectionPoint(rayToThisCell, &outOfPrevCell);
                        //CVF_ASSERT(intersectionOk);
                        //CVF_ASSERT(intersectionOk);
                        if ((currentPoint - outOfPrevCell).lengthSquared() > 1e-3)
                        {
                            branchCLCoords.push_back(outOfPrevCell);
                            branchCellIds.push_back(RigWellResultPoint());
                        }

                    }

                    branchCLCoords.push_back(currentPoint);
                    branchCellIds.push_back(currentWellResPoint);
                    
                    prevWellResPoint = &currentWellResPoint;

                    continue;
                }

                //
                // Handle currentWellResPoint as a real cell result points.
                //

                const RigCell& cell = rigReservoir->cellFromWellResultCell(currentWellResPoint);

                // Check if this and the previous cells has shared faces

                cvf::StructGridInterface::FaceType sharedFace;
                if (prevWellResPoint && prevWellResPoint->isCell() && rigReservoir->findSharedSourceFace(sharedFace, currentWellResPoint, *prevWellResPoint))
                {
                    // If they share faces, the shared face center is used as point
                    // describing the entry of this cell. (And exit of the previous cell)

                    branchCLCoords.push_back(cell.faceCenter(sharedFace));
                    branchCellIds.push_back(currentWellResPoint);
                }
                else
                {
                    // This and the previous cell does not share a face.
                    // Then we need to calculate the exit of the previous cell, and the entry point into this cell

                    cvf::Vec3d centerPreviousCell(cvf::Vec3d::ZERO);
                    cvf::Vec3d centerThisCell = cell.center();
                    bool distanceToWellHeadIsLonger = true;

                    // If we have a previous well result point, use its center as measure point and ray intersection start 
                    // when considering things. 

                    if (prevWellResPoint && prevWellResPoint->isValid())
                    {
                        if (prevWellResPoint->isCell())
                        {
                            const RigCell& prevCell = rigReservoir->cellFromWellResultCell(*prevWellResPoint);
                            centerPreviousCell = prevCell.center();
                        }
                        else
                        {
                            centerPreviousCell = prevWellResPoint->m_bottomPosition;
                        }

                        distanceToWellHeadIsLonger = (centerThisCell - centerPreviousCell).lengthSquared() <= (centerThisCell - whStartPos).lengthSquared();
                    }
                   

                    // First make sure this cell is not starting a new "display" branch for none MSW's

                    if (   wellResults->isMultiSegmentWell()
                            || !isAutoDetectBranches
                            || (prevWellResPoint == whResCell) 
                            || distanceToWellHeadIsLonger)
                    {
                        // Not starting a "display" branch for normal wells
                        // Calculate the exit of the previous cell, and the entry point into this cell

                        cvf::Vec3d intoThisCell(centerThisCell); // Use cell center as default for "into" point.

                        if (prevWellResPoint && prevWellResPoint->isValid())
                        {
                            // We have a defined previous point
                            // Create ray between the previous and this cell 
                            
                            cvf::Ray rayToThisCell;
                            rayToThisCell.setOrigin(centerPreviousCell);
                            rayToThisCell.setDirection((centerThisCell - centerPreviousCell).getNormalized());

                            // Intersect with the current cell to find a better entry point than the cell center

                            int intersectionCount = cell.firstIntersectionPoint(rayToThisCell, &intoThisCell);
                            bool isPreviousResPointInsideCurrentCell = (intersectionCount % 2); // Must intersect uneven times to be inside. (1 % 2 = 1)

                            // If we have a real previous cell, we need to go out of it, before entering this.
                            // That is: add a CL-point describing where it leaves the previous cell.

                            if ( prevWellResPoint->isCell())
                            {
                                cvf::Vec3d outOfPrevCell(centerPreviousCell);

                                const RigCell& prevCell = rigReservoir->cellFromWellResultCell(*prevWellResPoint);
                                //bool intersectionOk = prevCell.firstIntersectionPoint(rayToThisCell, &outOfPrevCell);
                                //CVF_ASSERT(intersectionOk);
                                //CVF_ASSERT(intersectionOk);
                                if ((intoThisCell - outOfPrevCell).lengthSquared() > 1e-3)
                                {
                                    branchCLCoords.push_back(outOfPrevCell);
                                    branchCellIds.push_back(RigWellResultPoint());
                                }
                            }
                            else if (isPreviousResPointInsideCurrentCell)
                            {
                                // Since the previous point actually is inside this cell, 
                                /// use that as the entry point into this cell
                                intoThisCell = centerPreviousCell;
                            }

                        }

                        branchCLCoords.push_back(intoThisCell);
                        branchCellIds.push_back(currentWellResPoint);
                    }
                    else
                    {
                        // Need to start a "display branch" for a Normal Well.

                        CVF_ASSERT(!wellResults->isMultiSegmentWell());

                        // This cell is further from the previous cell than from the well head, 
                        // thus we interpret it as a new branch.

                        // First finish the current branch in the previous cell
                        //branchCLCoords.push_back(branchCLCoords.back() + 1.5*(centerPreviousCell - branchCLCoords.back())  );
                        finishPipeCenterLine(pipeBranchesCLCoords, centerPreviousCell);

                        // Create new display branch
                        pipeBranchesCLCoords.push_back(std::vector<cvf::Vec3d>());
                        pipeBranchesCellIds.push_back(std::vector <RigWellResultPoint>());

                        // Start the new branch by entering the first cell (the wellhead) and intermediate
                        prevWellResPoint = whResCell;
                        pipeBranchesCLCoords.back().push_back(whStartPos);
                        pipeBranchesCellIds.back().push_back(*prevWellResPoint);

                        // Include intermediate
                        pipeBranchesCLCoords.back().push_back(whIntermediate);
                        pipeBranchesCellIds.back().push_back(*prevWellResPoint);

                        // Well now we need to step one back to take this cell again, but in the new branch.
                        cIdx--;
                        continue;
                    }
                }

                prevWellResPoint = &currentWellResPoint;
             }
            
             // For the last cell, add the point 0.5 past the center of that cell
             // Remember that prevWellResPoint actually is the last one in this branch.

             cvf::Vec3d centerLastCell;
             if (prevWellResPoint && prevWellResPoint->isCell())
             {
                 const RigCell& prevCell =  rigReservoir->cellFromWellResultCell(*prevWellResPoint);
                 centerLastCell = prevCell.center();
                 finishPipeCenterLine(pipeBranchesCLCoords, centerLastCell);
             }
             else
             {
                 // Remove the ID that is superfluous since we will not add an ending point
                 pipeBranchesCellIds.back().pop_back();
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
/// All branches are completed using the point 0.5 past the center of 
/// last cell.
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::finishPipeCenterLine(std::vector< std::vector<cvf::Vec3d> > &pipeBranchesCLCoords, const cvf::Vec3d& lastCellCenter) const
{
    CVF_ASSERT(pipeBranchesCLCoords.size());
    CVF_ASSERT(pipeBranchesCLCoords.back().size());

    cvf::Vec3d entryPointLastCell = pipeBranchesCLCoords.back().back();

    pipeBranchesCLCoords.back().push_back(entryPointLastCell + 1.5*(lastCellCenter - entryPointLastCell)  );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPipesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimReservoirView.isNull()) return;
    if (m_rimWell.isNull()) return;
    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

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

    RigSingleWellResultsData* wRes = m_rimWell->wellResults();
    if (wRes == NULL) return;

    if (frameIndex < wRes->firstResultTimeStep()) return; // Or reset colors or something

    const double closed = -0.1, producing = 1.5, water = 2.5, hcInjection = 3.5; // Closed set to -0.1 instead of 0.5 to workaround bug in the scalar mapper.

    std::list<RivPipeBranchData>::iterator brIt;
    const RigWellResultFrame& wResFrame = wRes->wellResultFrame(frameIndex);

    std::vector<double> wellCellStates;

    for (brIt = m_wellBranches.begin(); brIt != m_wellBranches.end(); ++brIt)
    {
        // Initialize well states to "closed" state
        wellCellStates.clear();
        wellCellStates.resize(brIt->m_cellIds.size(), closed);

        const std::vector <RigWellResultPoint>& cellIds =  brIt->m_cellIds;

        for (size_t wcIdx = 0; wcIdx < cellIds.size(); ++wcIdx)
        {
            // we need a faster lookup, I guess
            const RigWellResultPoint* wResCell = NULL;
            
            if (cellIds[wcIdx].isCell())
            {
                wResCell = wResFrame.findResultCell(cellIds[wcIdx].m_gridIndex, cellIds[wcIdx].m_gridCellIndex);
            }

            if (wResCell == NULL) 
            {
                // We cant find any state. This well cell is closed.
            }
            else
            {
                double cellState = closed;

                if (wResCell->m_isOpen)
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

            brIt->m_pipeGeomGenerator->pipeSurfaceTextureCoords( surfTexCoords.p(), wellCellStates, m_scalarMapper.p());
            brIt->m_surfaceDrawable->setTextureCoordArray( surfTexCoords.p());

            brIt->m_surfacePart->setEffect(m_scalarMapperSurfaceEffect.p());
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
            brIt->m_pipeGeomGenerator->centerlineTextureCoords( lineTexCoords.p(), wellCellStates, m_scalarMapper.p());

            // Set the new texture coordinates

            brIt->m_centerLineDrawable->setTextureCoordArray( lineTexCoords.p());

            // Set effects

            brIt->m_centerLinePart->setEffect(m_scalarMapperMeshEffect.p());
        }
    }
}

