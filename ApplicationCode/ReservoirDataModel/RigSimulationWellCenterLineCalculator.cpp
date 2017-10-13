/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigSimulationWellCenterLineCalculator.h"

#include "RigCell.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include "cvfRay.h"

//--------------------------------------------------------------------------------------------------
/// Based on the points and cells, calculate a pipe centerline
/// The returned CellIds is one less than the number of centerline points,
/// and are describing the lines between the points, starting with the first line
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline(RimSimWellInView* rimWell, 
                                                                        std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                                        std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds) 
{
    calculateWellPipeDynamicCenterline(rimWell, -1, pipeBranchesCLCoords, pipeBranchesCellIds);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCenterLineCalculator::calculateWellPipeDynamicCenterline(const RimSimWellInView* rimWell, 
                                                                               size_t timeStepIndex, 
                                                                               std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                                               std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds)
{
    CVF_ASSERT(rimWell);

    const RigSimWellData*  simWellData = rimWell->simWellData();

    RimEclipseView* eclipseView;
    rimWell->firstAncestorOrThisOfType(eclipseView);

    CVF_ASSERT(eclipseView);

    RigEclipseCaseData* eclipseCaseData      = eclipseView->eclipseCase()->eclipseCaseData();
    bool isAutoDetectBranches = eclipseView->wellCollection()->isAutoDetectingBranches();

    bool useAllCellCenters  = rimWell->isUsingCellCenterForPipe();

    calculateWellPipeCenterlineFromWellFrame(eclipseCaseData,
                                             simWellData,
                                             static_cast<int>(timeStepIndex),
                                             isAutoDetectBranches,
                                             useAllCellCenters,
                                             pipeBranchesCLCoords, 
                                             pipeBranchesCellIds);
}

//--------------------------------------------------------------------------------------------------
/// Based on the points and cells, calculate a pipe centerline
/// The returned CellIds is one less than the number of centerline points,
/// and are describing the lines between the points, starting with the first line
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(const RigEclipseCaseData* eclipseCaseData, 
                                                                                     const RigSimWellData* wellResults,
                                                                                     int timeStepIndex,
                                                                                     bool isAutoDetectBranches,
                                                                                     bool useAllCellCenters,
                                                                                     std::vector<std::vector<cvf::Vec3d>> &pipeBranchesCLCoords, 
                                                                                     std::vector<std::vector<RigWellResultPoint>> &pipeBranchesCellIds)
{
    if ( !wellResults) return;
    if ( timeStepIndex >= 0 && !wellResults->hasWellResult(timeStepIndex) ) return;

    const RigWellResultFrame* wellFramePtr = nullptr;
    
    if (timeStepIndex < 0) 
    {
        wellFramePtr =  &wellResults->staticWellCells();
    }
    else
    {
        wellFramePtr =  &(wellResults->wellResultFrame(timeStepIndex));
    }

    const RigWellResultFrame& wellFrame = *wellFramePtr;
    bool isMultiSegmentWell             = wellResults->isMultiSegmentWell();

    // Initialize the return arrays
    pipeBranchesCLCoords.clear();
    pipeBranchesCellIds.clear();

    if ( wellFrame.m_wellResultBranches.size() == 0 ) return;

    // Well head
    // Match this position with well head position in RivWellHeadPartMgr::buildWellHeadParts()

    const RigCell& whCell = eclipseCaseData->cellFromWellResultCell(wellFrame.m_wellHead);
    cvf::Vec3d whStartPos = whCell.faceCenter(cvf::StructGridInterface::NEG_K);
    const RigWellResultPoint* whResCell = &(wellFrame.m_wellHead);


    const std::vector<RigWellResultBranch>& resBranches = wellFrame.m_wellResultBranches;

    if ( ! hasAnyResultCells(resBranches) ) return;
   
    // Add extra coordinate between cell face and cell center 
    // to make sure the well pipe terminated in a segment parallel to z-axis

    cvf::Vec3d whIntermediate = whStartPos;
    whIntermediate.z() = (whStartPos.z() + whCell.center().z()) / 2.0;

    const RigWellResultPoint* prevWellResPoint = NULL;

    CVF_ASSERT(isMultiSegmentWell ||  resBranches.size() <= 1);

    // The centerline is calculated by adding a point when the pipe enters a cell, 
    // and one when the line leaves the cell.
    // For the sake of the loop:
    // The currentResultPoint (Cell) and the one we index by the loop variable is the one we calculate the entry point to.
    // The previous cell is the one we leave, and calculate the "out-point" from 


    for (size_t brIdx = 0; brIdx < resBranches.size(); brIdx++)
    {

        // Skip empty branches. Do not know why they exist, but they make problems.

        const RigWellResultBranch&  branch = resBranches[brIdx];
        if ( !hasAnyValidDataCells(branch) ) continue;

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

                    const RigCell& prevCell = eclipseCaseData->cellFromWellResultCell(*prevWellResPoint);
                    cvf::Vec3d centerPreviousCell = prevCell.center();

                    cvf::Ray rayToThisCell;
                    rayToThisCell.setOrigin(centerPreviousCell);
                    rayToThisCell.setDirection((currentPoint - centerPreviousCell).getNormalized());

                    cvf::Vec3d outOfPrevCell(centerPreviousCell);

                    prevCell.firstIntersectionPoint(rayToThisCell, &outOfPrevCell);
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

            const RigCell& cell = eclipseCaseData->cellFromWellResultCell(currentWellResPoint);

            // Check if this and the previous cells has shared faces

            cvf::StructGridInterface::FaceType sharedFace;
            if (prevWellResPoint && prevWellResPoint->isCell() && eclipseCaseData->findSharedSourceFace(sharedFace, currentWellResPoint, *prevWellResPoint))
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
                        const RigCell& prevCell = eclipseCaseData->cellFromWellResultCell(*prevWellResPoint);
                        centerPreviousCell = prevCell.center();
                    }
                    else
                    {
                        centerPreviousCell = prevWellResPoint->m_bottomPosition;
                    }

                    distanceToWellHeadIsLonger = (centerThisCell - centerPreviousCell).lengthSquared() <= (centerThisCell - whStartPos).lengthSquared();
                }


                // First make sure this cell is not starting a new "display" branch for none MSW's

                if (    isMultiSegmentWell
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

                            const RigCell& prevCell = eclipseCaseData->cellFromWellResultCell(*prevWellResPoint);
                            prevCell.firstIntersectionPoint(rayToThisCell, &outOfPrevCell);
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

                    CVF_ASSERT(!isMultiSegmentWell);

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

        if (prevWellResPoint && prevWellResPoint->isCell())
        {
            const RigCell& prevCell =  eclipseCaseData->cellFromWellResultCell(*prevWellResPoint);
            cvf::Vec3d centerLastCell = prevCell.center();
            finishPipeCenterLine(pipeBranchesCLCoords, centerLastCell);
        }
        else if (prevWellResPoint && prevWellResPoint->isPointValid())
        {
            // Continue the line with the same point, just to keep the last Cell ID
            pipeBranchesCLCoords.back().push_back(prevWellResPoint->m_bottomPosition);
        }
        else
        {
            // Remove the ID that is superfluous since we will not add an ending point
            pipeBranchesCellIds.back().pop_back();
        }
    }

    if (useAllCellCenters) addCellCenterPoints(eclipseCaseData, pipeBranchesCLCoords, pipeBranchesCellIds);

    CVF_ASSERT(pipeBranchesCellIds.size() == pipeBranchesCLCoords.size());
    for (size_t i = 0 ; i < pipeBranchesCellIds.size() ; ++i)
    {
        CVF_ASSERT(pipeBranchesCellIds[i].size() == pipeBranchesCLCoords[i].size()-1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCenterLineCalculator::addCellCenterPoints(const RigEclipseCaseData* eclipseCaseData, 
                                                                std::vector<std::vector<cvf::Vec3d>> &pipeBranchesCLCoords, 
                                                                std::vector<std::vector<RigWellResultPoint>> &pipeBranchesCellIds)
{
    for ( size_t brIdx = 0; brIdx < pipeBranchesCellIds.size(); brIdx++ )
    {
        const std::vector<RigWellResultPoint>& branchResPoints = pipeBranchesCellIds[brIdx];
        const std::vector<cvf::Vec3d>& branchClPoints = pipeBranchesCLCoords[brIdx];
        
        std::vector<RigWellResultPoint> branchResPointsWithCellCenters;
        std::vector<cvf::Vec3d> branchClPointsWithCellCenters;

        for ( size_t cIdx = 0; cIdx < branchResPoints.size(); cIdx++ )
        {
            branchResPointsWithCellCenters.push_back(branchResPoints[cIdx]);
            branchClPointsWithCellCenters.push_back(branchClPoints[cIdx]);

            if ( branchResPoints[cIdx].isCell() )
            {
                const RigCell& cell = eclipseCaseData->cellFromWellResultCell(branchResPoints[cIdx]);
                cvf::Vec3d center = cell.center();
                branchClPointsWithCellCenters.push_back(center);
                branchResPointsWithCellCenters.push_back(branchResPoints[cIdx]);          
            }
        }
        
        branchClPointsWithCellCenters.push_back(branchClPoints[branchResPoints.size()]);

        pipeBranchesCellIds[brIdx]  = branchResPointsWithCellCenters;
        pipeBranchesCLCoords[brIdx] = branchClPointsWithCellCenters;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigSimulationWellCenterLineCalculator::hasAnyResultCells(const std::vector<RigWellResultBranch> &resBranches)
{
    bool hasResultCells = false;
    if ( resBranches.size() )
    {
        for ( size_t i = 0 ; i < resBranches.size(); ++i )
        {
            if ( resBranches[i].m_branchResultPoints.size() != 0 )
            {
                hasResultCells = true;
                break;
            }
        }
    }
    return hasResultCells;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigSimulationWellCenterLineCalculator::hasAnyValidDataCells(const RigWellResultBranch& branch)
{
    bool hasValidData = false;
    for ( size_t cIdx = 0; cIdx < branch.m_branchResultPoints.size(); ++cIdx )
    {
        if ( branch.m_branchResultPoints[cIdx].isValid() )
        {
            hasValidData = true;
            break;
        }
    }

    return hasValidData;
}

//--------------------------------------------------------------------------------------------------
/// All branches are completed using the point 0.5 past the center of 
/// last cell.
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCenterLineCalculator::finishPipeCenterLine(std::vector< std::vector<cvf::Vec3d> > &pipeBranchesCLCoords, const cvf::Vec3d& lastCellCenter)
{
    CVF_ASSERT(pipeBranchesCLCoords.size());
    CVF_ASSERT(pipeBranchesCLCoords.back().size());

    cvf::Vec3d entryPointLastCell = pipeBranchesCLCoords.back().back();

    pipeBranchesCLCoords.back().push_back(entryPointLastCell + 1.5*(lastCellCenter - entryPointLastCell)  );
}


