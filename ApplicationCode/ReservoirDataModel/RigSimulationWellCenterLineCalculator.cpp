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
#include "cvfBoundingBoxTree.h"
#include "RigMainGrid.h"
#include <deque>
#include <list>

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

    bool isMultiSegmentWell             = wellResults->isMultiSegmentWell();

    RigWellResultFrame splittedWellFrame;
    if (!isMultiSegmentWell && isAutoDetectBranches)
    {
        splittedWellFrame = splitIntoBranches(*wellFramePtr, eclipseCaseData);
        wellFramePtr = &splittedWellFrame;
        isMultiSegmentWell = true;
    }

    const RigWellResultFrame& wellFrame = *wellFramePtr;

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



















//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class BranchSplitter
{
public:
    BranchSplitter(const RigWellResultFrame& awellResultFrame, 
                   const RigEclipseCaseData* eclipseCaseData)
                   :m_eclipseCaseData(eclipseCaseData), m_orgWellResultFrame(awellResultFrame)
    {
        CVF_ASSERT(m_orgWellResultFrame.m_wellResultBranches.size() <= 1);
        m_branchedWell = m_orgWellResultFrame;
        
        buildCellSearchTree();

        buildCellsToNeighborsMap();

        // Detect and remove small loops

        for (auto& cellNeighborsPair: m_cellsWithNeighbors)
        {
            if (cellNeighborsPair.second.size() > 2)
            {

            }
        }

        buildUnusedCellsSet();
  
        buildBranchLinesOfContinousNeighbourCells();
  
        
        class DistToEndPoint
        {
        public:
            DistToEndPoint(double adist, std::list< std::pair<bool, std::deque<size_t> > >::iterator aBranchLineIt, bool aToFrontOfBranchLine2)
                : dist(adist), branchLineIt(aBranchLineIt), toFrontOfBranchLine(aToFrontOfBranchLine2)
            {}

            double dist;

            std::list< std::pair<bool, std::deque<size_t> > >::iterator branchLineIt;
            bool toFrontOfBranchLine; 

            bool operator<(const DistToEndPoint& other ) const { return dist < other.dist; } 
        };

        auto cmp = [] (std::list< std::pair<bool, std::deque<size_t> > >::iterator a, 
                       std::list< std::pair<bool, std::deque<size_t> > >::iterator b) 
                       { 
                           return &(*a) < &(*b);
                       };

        std::set< std::list< std::pair<bool, std::deque<size_t> > >::iterator, decltype(cmp) > unusedBranchLineIterators(cmp);


        std::map<int, std::multiset<DistToEndPoint> > resBranchIdxToBranchLineEndPointsDists;

        /// Creating useful lambda functions

        auto buildResBranchToBranchLineEndsDistMap = 
        [&unusedBranchLineIterators, &resBranchIdxToBranchLineEndPointsDists, this](cvf::Vec3d& fromPoint, int resultBranchIndex)
        {
            for ( auto it :unusedBranchLineIterators )
            {
                {
                    double dist = calculateFrontToPointDistance(it->second, fromPoint);
                    resBranchIdxToBranchLineEndPointsDists[resultBranchIndex].insert(DistToEndPoint(dist, it, true));
                }

                {
                    double dist = calculateEndToPointDistance(it->second, fromPoint);
                    resBranchIdxToBranchLineEndPointsDists[resultBranchIndex].insert(DistToEndPoint(dist, it, false));
                }
            }
        };

        auto removeBranchLineFromDistanceMap = 
            [&resBranchIdxToBranchLineEndPointsDists](std::list< std::pair<bool, std::deque<size_t> > >::iterator branchLineToMergeIt)
        {
            for ( auto& brIdx_DistToEndPointSet : resBranchIdxToBranchLineEndPointsDists )
            {
                std::vector<std::multiset<DistToEndPoint>::iterator> iteratorsToErase;
                for ( auto it = brIdx_DistToEndPointSet.second.begin(); it != brIdx_DistToEndPointSet.second.end(); ++it )
                {
                    if ( it->branchLineIt == branchLineToMergeIt )
                    {
                        iteratorsToErase.push_back(it);
                    }
                }

                for (auto itToErase : iteratorsToErase)  brIdx_DistToEndPointSet.second.erase(itToErase);
            }
        };

        // Make the result container ready

        m_branchedWell.m_wellResultBranches.clear();
        m_branchedWell.m_wellResultBranches.push_back(RigWellResultBranch());

        // Build set of unused branch lines

        for (auto brLIt = m_branchLines.begin(); brLIt != m_branchLines.end(); ++brLIt)
        {
            if (brLIt->first) unusedBranchLineIterators.insert(brLIt);
        }

        // Calculate wellhead to branch line ends distances
        {
            const RigCell& whCell = m_eclipseCaseData->cellFromWellResultCell(m_orgWellResultFrame.m_wellHead);
            cvf::Vec3d whStartPos = whCell.faceCenter(cvf::StructGridInterface::NEG_K);

            buildResBranchToBranchLineEndsDistMap(whStartPos, -1);
        }

        // Add the branchLine closest to wellhead into the result
        {
            auto closestEndPointIt =  resBranchIdxToBranchLineEndPointsDists[-1].begin();
            addBranchLineToLastWellResultBranch(closestEndPointIt->branchLineIt, closestEndPointIt->toFrontOfBranchLine);
            unusedBranchLineIterators.erase(closestEndPointIt->branchLineIt);
            removeBranchLineFromDistanceMap(closestEndPointIt->branchLineIt);
        }

        // Add the branchLines that starts directly from another branchLine
        {
            for ( auto brLIt = m_branchLines.begin(); brLIt != m_branchLines.end(); ++brLIt )
            {
                if ( !brLIt->first )
                {
                    m_branchedWell.m_wellResultBranches.push_back(RigWellResultBranch());
                    addBranchLineToLastWellResultBranch(brLIt, true);
                }
            }
        }

        while ( !unusedBranchLineIterators.empty() ) 
        {
            //    Calculate distance from end of all currently added result branches to all branch lines

            for (size_t resultBranchIndex = 0; resultBranchIndex < m_branchedWell.m_wellResultBranches.size(); ++resultBranchIndex)
            {
                if (! resBranchIdxToBranchLineEndPointsDists.count((int)resultBranchIndex) 
                    && m_branchedWell.m_wellResultBranches[resultBranchIndex].m_branchResultPoints.size()
                    && m_branchedWell.m_wellResultBranches[resultBranchIndex].m_branchResultPoints.back().isCell())
                {
                    const RigCell& whCell = eclipseCaseData->cellFromWellResultCell(m_branchedWell.m_wellResultBranches[resultBranchIndex].m_branchResultPoints.back());
                    cvf::Vec3d branchEndPoint = whCell.center();

                    buildResBranchToBranchLineEndsDistMap(branchEndPoint, (int)resultBranchIndex);
                }
            }

            // Find the result branch to grow, by finding the one with the closest distance to a branchLine

            int minDistanceBrIdx = -1;
            DistToEndPoint closestEndPoint(HUGE_VAL,m_branchLines.end(), true);

            for (auto& brIdx_DistToEndPointSet : resBranchIdxToBranchLineEndPointsDists)
            {
                if (brIdx_DistToEndPointSet.second.begin()->dist < closestEndPoint.dist)
                {
                    minDistanceBrIdx =  brIdx_DistToEndPointSet.first;
                    closestEndPoint = *( brIdx_DistToEndPointSet.second.begin());
                }
            }

            // Grow the result branch with the branchLine

            auto closestEndPointIt =  resBranchIdxToBranchLineEndPointsDists[minDistanceBrIdx].begin();
            auto branchLineToAddIt = closestEndPointIt->branchLineIt;
            addBranchLineToWellResultBranch(minDistanceBrIdx, branchLineToAddIt, closestEndPointIt->toFrontOfBranchLine);

            // Remove the branchLine from the control datastructures

            unusedBranchLineIterators.erase(branchLineToAddIt);
            resBranchIdxToBranchLineEndPointsDists.erase(minDistanceBrIdx);
            removeBranchLineFromDistanceMap(branchLineToAddIt);
        } 

    }



    RigWellResultFrame splittedWellResultFrame()
    {
        return m_branchedWell;
    }

private:

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void addBranchLineToLastWellResultBranch(std::list< std::pair<bool, std::deque<size_t> > >::iterator branchLineIt, bool startAtFront)
    {
        addBranchLineToWellResultBranch( static_cast<int>(m_branchedWell.m_wellResultBranches.size()) - 1, branchLineIt, startAtFront);
    }

    //--------------------------------------------------------------------------------------------------
    /// branchIdx == -1 creates a new branch
    //--------------------------------------------------------------------------------------------------
    void addBranchLineToWellResultBranch(int branchIdx, std::list< std::pair<bool, std::deque<size_t> > >::iterator branchLineIt, bool startAtFront)
    {
        if (branchIdx < 0)
        {
            m_branchedWell.m_wellResultBranches.push_back(RigWellResultBranch());
            branchIdx = static_cast<int>( m_branchedWell.m_wellResultBranches.size()) - 1;
        }

        RigWellResultBranch& currentBranch = m_branchedWell.m_wellResultBranches[branchIdx];  
        std::deque<size_t> wellCellIndices = branchLineIt->second;
        if (!startAtFront) std::reverse(wellCellIndices.begin(), wellCellIndices.end());

        const std::vector<RigWellResultPoint>& orgWellResultPoints = m_orgWellResultFrame.m_wellResultBranches[0].m_branchResultPoints;

        for (size_t wcIdx : wellCellIndices)
        {
            currentBranch.m_branchResultPoints.push_back(orgWellResultPoints[wcIdx]);
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void buildCellSearchTree()
    {
        const std::vector<RigWellResultPoint>& orgWellResultPoints = m_orgWellResultFrame.m_wellResultBranches[0].m_branchResultPoints;
        size_t cellCount = orgWellResultPoints.size();

        m_cellBoundingBoxes.resize(cellCount);

        const std::vector<cvf::Vec3d>& nodes = m_eclipseCaseData->mainGrid()->nodes();

        for ( size_t cIdx = 0; cIdx < cellCount; ++cIdx )
        {
            if ( !orgWellResultPoints[cIdx].isCell() ) continue;
            const RigCell& wellCell = m_eclipseCaseData->cellFromWellResultCell(orgWellResultPoints[cIdx]);

            if ( wellCell.isInvalid() ) continue;

            const caf::SizeTArray8& cellIndices = wellCell.cornerIndices();

            cvf::BoundingBox& cellBB = m_cellBoundingBoxes[cIdx];
            cellBB.add(nodes[cellIndices[0]]);
            cellBB.add(nodes[cellIndices[1]]);
            cellBB.add(nodes[cellIndices[2]]);
            cellBB.add(nodes[cellIndices[3]]);
            cellBB.add(nodes[cellIndices[4]]);
            cellBB.add(nodes[cellIndices[5]]);
            cellBB.add(nodes[cellIndices[6]]);
            cellBB.add(nodes[cellIndices[7]]);
        }

        m_cellSearchTree.buildTreeFromBoundingBoxes(m_cellBoundingBoxes, nullptr);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void buildCellsToNeighborsMap()
    {
        const std::vector<RigWellResultPoint>& orgWellResultPoints = m_orgWellResultFrame.m_wellResultBranches[0].m_branchResultPoints;
        size_t cellCount = orgWellResultPoints.size();

        for ( size_t cIdx = 0; cIdx < cellCount; ++ cIdx )
        {
            std::vector<size_t> closeCells;
            m_cellSearchTree.findIntersections(m_cellBoundingBoxes[cIdx], &closeCells);

            const RigCell& c1 = m_eclipseCaseData->cellFromWellResultCell(orgWellResultPoints[cIdx]);

            for ( size_t idxToCloseCell : closeCells )
            {
                if ( idxToCloseCell != cIdx && m_cellsWithNeighbors[cIdx].count(idxToCloseCell) == 0 )
                {
                    const RigCell& c2 = m_eclipseCaseData->cellFromWellResultCell(orgWellResultPoints[idxToCloseCell]);
                    auto contactFace = RigNNCData::calculateCellFaceOverlap(c1, c2, *(m_eclipseCaseData->mainGrid()), nullptr, nullptr);

                    if ( contactFace != cvf::StructGridInterface::NO_FACE )
                    {
                        m_cellsWithNeighbors[cIdx].insert(idxToCloseCell);
                        m_cellsWithNeighbors[idxToCloseCell].insert(cIdx);
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void buildUnusedCellsSet()
    {
        const std::vector<RigWellResultPoint>& orgWellResultPoints = m_orgWellResultFrame.m_wellResultBranches[0].m_branchResultPoints;
        size_t cellCount = orgWellResultPoints.size();

        for ( size_t i = 0; i < cellCount; ++i )
        {
            if ( orgWellResultPoints[i].isCell() ) m_unusedWellCellIndices.insert(i);
        }
    }
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void buildBranchLinesOfContinousNeighbourCells()
    {
        for ( auto& cellWithNeighborsPair : m_cellsWithNeighbors )
        {
            auto it = m_unusedWellCellIndices.find(cellWithNeighborsPair.first);
            if ( it !=  m_unusedWellCellIndices.end() )
            {
                m_unusedWellCellIndices.erase(it);

                // Create a new branchline containing with the cell itself.
                m_branchLines.push_back(std::make_pair(true, std::deque<size_t>()));
                auto currentBranchLineIt = std::prev(m_branchLines.end());
                auto& branchList = currentBranchLineIt->second;

                branchList.push_back(cellWithNeighborsPair.first);

                unsigned endToGrow = 0; // 0 end, 1 front, 1< new branch
                for ( size_t neighbour : cellWithNeighborsPair.second )
                {
                    if ( m_unusedWellCellIndices.count(neighbour) )
                    {
                        m_unusedWellCellIndices.erase(neighbour);
                        if ( endToGrow == 0 )
                        {
                            branchList.push_back(neighbour);
                            growBranchListEnd(currentBranchLineIt);
                            endToGrow++;
                        }
                        else if ( endToGrow == 1 )
                        {
                            branchList.push_front(neighbour);
                            growBranchListFront(currentBranchLineIt);
                            endToGrow++;

                        }
                        else // if ( endToGrow > 1 )
                        {
                            m_branchLines.push_back(std::make_pair(false, std::deque<size_t>{cellWithNeighborsPair.first, neighbour }));
                            auto newBranchLineIt = std::prev(m_branchLines.end());
                            growBranchListEnd(newBranchLineIt);
                        }
                    }
                }
            }
        }
    }



    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void growBranchListEnd( std::list< std::pair<bool, std::deque<size_t> > >::iterator  branchListIt)
    {
        std::deque<size_t>& branchList = branchListIt->second;

        CVF_ASSERT(branchList.size());

        size_t startCell = branchList.back();
        const auto& neighbors = m_cellsWithNeighbors[startCell];

        // Find first unused cell among the neighbors

        auto it = neighbors.begin();
        for (; it != neighbors.end(); ++it)
        {
            size_t neighbor = *it;
            if (m_unusedWellCellIndices.count(neighbor))
            {
                branchList.push_back(neighbor);
                m_unusedWellCellIndices.erase(neighbor);
                ++it;
                break;
            }
        }

        // If we added a cell grow further
        if ( branchList.back() != startCell ) growBranchListEnd(branchListIt);

        while ( it != neighbors.end())  // Possible branches starting
        {
            size_t neighbor = *it;
            if (m_unusedWellCellIndices.count(neighbor))
            {
                m_branchLines.push_back(std::make_pair(false, std::deque<size_t>{startCell, neighbor}) );
                m_unusedWellCellIndices.erase(neighbor);
                auto lastBranchIt = std::prev(m_branchLines.end());

                growBranchListEnd(lastBranchIt);
            }
            ++it;
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void growBranchListFront( std::list< std::pair<bool, std::deque<size_t> > >::iterator  branchListIt)
    {
        std::deque<size_t>& branchList = branchListIt->second;

        CVF_ASSERT(branchList.size());

        size_t startCell = branchList.front();
        const auto& neighbors = m_cellsWithNeighbors[startCell];

        // Find first unused cell among the neighbors

        auto it = neighbors.begin();
        for (; it != neighbors.end(); ++it)
        {
            size_t neighbor = *it;
            if (m_unusedWellCellIndices.count(neighbor))
            {
                branchList.push_front(neighbor);
                m_unusedWellCellIndices.erase(neighbor);
                ++it;
                break;
            }
        }

        // If we added a cell grow further
        if ( branchList.front() != startCell ) growBranchListFront(branchListIt);

        while ( it != neighbors.end())  // Possible branches starting
        {
            size_t neighbor = *it;
            if (m_unusedWellCellIndices.count(neighbor))
            {
                m_branchLines.push_back(std::make_pair(false, std::deque<size_t>{startCell, neighbor}) );
                m_unusedWellCellIndices.erase(neighbor);
                auto lastBranchIt = std::prev(m_branchLines.end());

                growBranchListEnd(lastBranchIt);
            }
            ++it;
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    double calculateFrontToPointDistance(const std::deque<size_t>& second, const cvf::Vec3d& point)
    {
        // Todo, more fancy virtual curvature based distance using an estimated direction from the branch-end

        return calculateWellCellToPointDistance(second.front(), point);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    double calculateEndToPointDistance(const std::deque<size_t>& second, const cvf::Vec3d& point)
    {
        // Todo, more fancy virtual curvature based distance using an estimated direction from the branch-end

        return calculateWellCellToPointDistance(second.back(), point);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    double calculateWellCellToPointDistance(size_t wellCellIdx, const cvf::Vec3d& point)
    {
        const std::vector<RigWellResultPoint>& orgWellResultPoints = m_orgWellResultFrame.m_wellResultBranches[0].m_branchResultPoints;

        const RigCell& c = m_eclipseCaseData->cellFromWellResultCell(orgWellResultPoints[wellCellIdx]);

        cvf::Vec3d cellCenter = c.center();

        return (point-cellCenter).length();
    }

    // The bool tells if this can be expanded in the front,
    // Set to false when the branchLine starts from a branching cell (cell with more than two neighbors)
    std::list< std::pair<bool, std::deque<size_t> > > m_branchLines;

    std::vector<cvf::BoundingBox> m_cellBoundingBoxes;
    cvf::BoundingBoxTree m_cellSearchTree;

    std::map<size_t, std::set<size_t> > m_cellsWithNeighbors;
    std::set<size_t> m_unusedWellCellIndices;

    RigWellResultFrame m_branchedWell;

    const RigEclipseCaseData* m_eclipseCaseData;
    const RigWellResultFrame& m_orgWellResultFrame;


};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellResultFrame RigSimulationWellCenterLineCalculator::splitIntoBranches(const RigWellResultFrame& wellResultFrame, 
                                                                            const RigEclipseCaseData* eclipseCaseData)
{
    BranchSplitter splitter(wellResultFrame, eclipseCaseData);

    return splitter.splittedWellResultFrame();
}