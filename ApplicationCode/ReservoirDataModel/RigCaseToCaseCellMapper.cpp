/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigCaseToCaseCellMapper.h"
#include "RigFemPart.h"
#include "RigMainGrid.h"
#include "RigFemPartGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigMainGrid* dependentEclGrid)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(NULL),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->cells().size(), cvf::UNDEFINED_INT);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigMainGrid* dependentEclGrid)
    : m_masterGrid(NULL),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->cells().size(), cvf::UNDEFINED_INT);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigFemPart* dependentFemPart)
    : m_masterGrid(NULL),
      m_dependentGrid(NULL),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(dependentFemPart)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const int * RigCaseToCaseCellMapper::masterCaseCellIndices(int dependentCaseReservoirCellIndex, int* masterCaseCellIndexCount) const
{
    int seriesIndex = m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex];

    if (seriesIndex == cvf::UNDEFINED_INT)
    {
        (*masterCaseCellIndexCount) = 0;
        return NULL;
    }

    if (seriesIndex < 0)
    {
        (*masterCaseCellIndexCount) = static_cast<int>(m_masterCellIndexSeries[-seriesIndex].size());
        return &(m_masterCellIndexSeries[-seriesIndex][0]);    
    }
    else
    {
        (*masterCaseCellIndexCount) = 1;
        return &(m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex]);
    }
}

class RigNeighborCornerFinder
{
public:
    RigNeighborCornerFinder(const RigMainGrid* mainGrid, size_t baseI, size_t baseJ, size_t baseK)
        : m_mainGrid(mainGrid),
        m_baseI(baseI),
        m_baseJ(baseJ),
        m_baseK(baseK)
    {}

    const caf::SizeTArray8* neighborIndices(int offsetI, int offsetJ, int offsetK)
    {
        if (offsetI < 0 && m_baseI == 0) return NULL;
        if (offsetJ < 0 && m_baseJ == 0) return NULL;
        if (offsetK < 0 && m_baseK == 0) return NULL;
        if (offsetI > 0 && m_baseI == m_mainGrid->cellCountI()-1) return NULL;
        if (offsetJ > 0 && m_baseJ == m_mainGrid->cellCountJ()-1) return NULL;
        if (offsetK > 0 && m_baseK == m_mainGrid->cellCountK()-1) return NULL;

        size_t gridLocalCellIndex = m_mainGrid->cellIndexFromIJK(m_baseI + offsetI, m_baseJ + offsetJ, m_baseK + offsetK);
        const RigCell& cell = m_mainGrid->cells()[gridLocalCellIndex];
        return &(cell.cornerIndices());
    }

private:
    const RigMainGrid* m_mainGrid;
    size_t m_baseI;
    size_t m_baseJ; 
    size_t m_baseK;
};

//--------------------------------------------------------------------------------------------------
/// Average of neighbor corresponding nodes
//--------------------------------------------------------------------------------------------------
void estimatedFemCellFromEclCell(const RigMainGrid* eclGrid, size_t reservoirCellIndex,  cvf::Vec3d estimatedElmCorners[8])
{
    CVF_TIGHT_ASSERT(reservoirCellIndex < eclGrid->cellCount()); // Assume reservoirCellIdx == localGridCellIdx for maingrid
    
    const std::vector<cvf::Vec3d>& eclNodes = eclGrid->nodes();
    
    size_t I,J,K;
    eclGrid->ijkFromCellIndex(reservoirCellIndex, &I, &J, &K);
    RigNeighborCornerFinder nbFinder(eclGrid, I,J,K);

    // Cell corner Averaging mapping: Local cell index in neighbor matching specific corner of this cell
    // N - Negative P - positive
    // 0 <- NI[1] NINJ[2] NJ[3] NK[4] NINK[5] NINJNK[6] NJNK[7] 
    // 1 <- NJ[2] PINJ[3] PI[0] NK[5] NJNK[6] PINJNK[7] PINK[4] 
    // 2 <- PI[3] PIPJ[0] PJ[1] NK[6] PINK[7] PIPJNK[4] PJNK[5]
    // 3 <- PJ[0] NIPJ[1] NI[2] NK[7] PJNK[4] NIPJNK[5] NINK[6]
    // 4 <- NI[5] NINJ[6] NJ[7] PK[0] NIPK[1] NINJPK[2] NJPK[3]
    // 5 <- NJ[6] PINJ[7] PI[4] PK[1] NJPK[2] PINJPK[3] PIPK[0]
    // 6 <- PI[7] PIPJ[4] PJ[5] PK[2] PIPK[3] PIPJPK[0] PJPK[1]
    // 7 <- PJ[4] NIPJ[5] NI[6] PK[3] PJPK[0] NIPJPK[1] NIPK[2]

    const caf::SizeTArray8* IJK      = nbFinder.neighborIndices( 0, 0, 0);
    const caf::SizeTArray8* NI       = nbFinder.neighborIndices(-1, 0, 0);
    const caf::SizeTArray8* NJ       = nbFinder.neighborIndices( 0,-1, 0);
    const caf::SizeTArray8* PI       = nbFinder.neighborIndices( 1, 0, 0);
    const caf::SizeTArray8* PJ       = nbFinder.neighborIndices( 0, 1, 0);
    const caf::SizeTArray8* NK       = nbFinder.neighborIndices( 0, 0,-1);
    const caf::SizeTArray8* PK       = nbFinder.neighborIndices( 0, 0, 1);
    const caf::SizeTArray8* NINJ     = nbFinder.neighborIndices(-1,-1, 0);
    const caf::SizeTArray8* PINJ     = nbFinder.neighborIndices( 1,-1, 0);

    const caf::SizeTArray8* PIPJ     = nbFinder.neighborIndices( 1, 1, 0);
    const caf::SizeTArray8* NIPJ     = nbFinder.neighborIndices(-1, 1, 0);
    const caf::SizeTArray8* NINK     = nbFinder.neighborIndices(-1, 0,-1);
    const caf::SizeTArray8* NJNK     = nbFinder.neighborIndices( 0,-1,-1);
    const caf::SizeTArray8* PINK     = nbFinder.neighborIndices( 1, 0,-1);
    const caf::SizeTArray8* PJNK     = nbFinder.neighborIndices( 0, 1,-1);
    const caf::SizeTArray8* NIPK     = nbFinder.neighborIndices(-1, 0, 1);
    const caf::SizeTArray8* NJPK     = nbFinder.neighborIndices( 0,-1, 1);
    const caf::SizeTArray8* PIPK     = nbFinder.neighborIndices( 1, 0, 1);

    const caf::SizeTArray8* PJPK     = nbFinder.neighborIndices( 0, 1, 1);
    const caf::SizeTArray8* NINJNK   = nbFinder.neighborIndices(-1,-1,-1);
    const caf::SizeTArray8* PINJNK   = nbFinder.neighborIndices( 1,-1,-1);
    const caf::SizeTArray8* PIPJNK   = nbFinder.neighborIndices( 1, 1,-1);
    const caf::SizeTArray8* NIPJNK   = nbFinder.neighborIndices(-1, 1,-1);
    const caf::SizeTArray8* NINJPK   = nbFinder.neighborIndices(-1,-1, 1);
    const caf::SizeTArray8* PINJPK   = nbFinder.neighborIndices( 1,-1, 1);
    const caf::SizeTArray8* PIPJPK   = nbFinder.neighborIndices( 1, 1, 1);
    const caf::SizeTArray8* NIPJPK   = nbFinder.neighborIndices(-1, 1, 1);

    std::vector<size_t> contributingNodeIndicesPrCellCorner[8];

    if (IJK   ) contributingNodeIndicesPrCellCorner[0].push_back((*IJK   )[0]);
    if (NI    ) contributingNodeIndicesPrCellCorner[0].push_back((*NI    )[1]);
    if (NINJ  ) contributingNodeIndicesPrCellCorner[0].push_back((*NINJ  )[2]);
    if (NJ    ) contributingNodeIndicesPrCellCorner[0].push_back((*NJ    )[3]);
    if (NK    ) contributingNodeIndicesPrCellCorner[0].push_back((*NK    )[4]);
    if (NINK  ) contributingNodeIndicesPrCellCorner[0].push_back((*NINK  )[5]);
    if (NINJNK) contributingNodeIndicesPrCellCorner[0].push_back((*NINJNK)[6]);
    if (NJNK  ) contributingNodeIndicesPrCellCorner[0].push_back((*NJNK  )[7]);

    if (IJK   ) contributingNodeIndicesPrCellCorner[1].push_back((*IJK   )[1]);
    if (NJ    ) contributingNodeIndicesPrCellCorner[1].push_back((*NJ    )[2]);
    if (PINJ  ) contributingNodeIndicesPrCellCorner[1].push_back((*PINJ  )[3]);
    if (PI    ) contributingNodeIndicesPrCellCorner[1].push_back((*PI    )[0]);
    if (NK    ) contributingNodeIndicesPrCellCorner[1].push_back((*NK    )[5]);
    if (NJNK  ) contributingNodeIndicesPrCellCorner[1].push_back((*NJNK  )[6]);
    if (PINJNK) contributingNodeIndicesPrCellCorner[1].push_back((*PINJNK)[7]);
    if (PINK  ) contributingNodeIndicesPrCellCorner[1].push_back((*PINK  )[4]);

    if (IJK   ) contributingNodeIndicesPrCellCorner[2].push_back((*IJK   )[2]);
    if (PI    ) contributingNodeIndicesPrCellCorner[2].push_back((*PI    )[3]);
    if (PIPJ  ) contributingNodeIndicesPrCellCorner[2].push_back((*PIPJ  )[0]);
    if (PJ    ) contributingNodeIndicesPrCellCorner[2].push_back((*PJ    )[1]);
    if (NK    ) contributingNodeIndicesPrCellCorner[2].push_back((*NK    )[6]);
    if (PINK  ) contributingNodeIndicesPrCellCorner[2].push_back((*PINK  )[7]);
    if (PIPJNK) contributingNodeIndicesPrCellCorner[2].push_back((*PIPJNK)[4]);
    if (PJNK  ) contributingNodeIndicesPrCellCorner[2].push_back((*PJNK  )[5]);

    if (IJK   ) contributingNodeIndicesPrCellCorner[3].push_back((*IJK   )[3]);
    if (PJ    ) contributingNodeIndicesPrCellCorner[3].push_back((*PJ    )[0]);
    if (NIPJ  ) contributingNodeIndicesPrCellCorner[3].push_back((*NIPJ  )[1]);
    if (NI    ) contributingNodeIndicesPrCellCorner[3].push_back((*NI    )[2]);
    if (NK    ) contributingNodeIndicesPrCellCorner[3].push_back((*NK    )[7]);
    if (PJNK  ) contributingNodeIndicesPrCellCorner[3].push_back((*PJNK  )[4]);
    if (NIPJNK) contributingNodeIndicesPrCellCorner[3].push_back((*NIPJNK)[5]);
    if (NINK  ) contributingNodeIndicesPrCellCorner[3].push_back((*NINK  )[6]);

        // 4 <- NI[5] NINJ[6] NJ[7] PK[0] NIPK[1] NINJPK[2] NJPK[3]

    if (IJK   ) contributingNodeIndicesPrCellCorner[4].push_back((*IJK   )[4]);
    if (NI    ) contributingNodeIndicesPrCellCorner[4].push_back((*NI    )[5]);
    if (NINJ  ) contributingNodeIndicesPrCellCorner[4].push_back((*NINJ  )[6]);
    if (NJ    ) contributingNodeIndicesPrCellCorner[4].push_back((*NJ    )[7]);
    if (PK    ) contributingNodeIndicesPrCellCorner[4].push_back((*PK    )[0]);
    if (NIPK  ) contributingNodeIndicesPrCellCorner[4].push_back((*NIPK  )[1]);
    if (NINJPK) contributingNodeIndicesPrCellCorner[4].push_back((*NINJPK)[2]);
    if (NJPK  ) contributingNodeIndicesPrCellCorner[4].push_back((*NJPK  )[3]);

    if (IJK   ) contributingNodeIndicesPrCellCorner[5].push_back((*IJK   )[5]);
    if (NJ    ) contributingNodeIndicesPrCellCorner[5].push_back((*NJ    )[6]);
    if (PINJ  ) contributingNodeIndicesPrCellCorner[5].push_back((*PINJ  )[7]);
    if (PI    ) contributingNodeIndicesPrCellCorner[5].push_back((*PI    )[4]);
    if (PK    ) contributingNodeIndicesPrCellCorner[5].push_back((*PK    )[1]);
    if (NJPK  ) contributingNodeIndicesPrCellCorner[5].push_back((*NJPK  )[2]);
    if (PINJPK) contributingNodeIndicesPrCellCorner[5].push_back((*PINJPK)[3]);
    if (PIPK  ) contributingNodeIndicesPrCellCorner[5].push_back((*PIPK  )[0]);
 
    // 6 <- PI[7] PIPJ[4] PJ[5] PK[2] PIPK[3] PIPJPK[0] PJPK[1]

    if (IJK   ) contributingNodeIndicesPrCellCorner[6].push_back((*IJK   )[6]);
    if (PI    ) contributingNodeIndicesPrCellCorner[6].push_back((*PI    )[7]);
    if (PIPJ  ) contributingNodeIndicesPrCellCorner[6].push_back((*PIPJ  )[4]);
    if (PJ    ) contributingNodeIndicesPrCellCorner[6].push_back((*PJ    )[5]);
    if (PK    ) contributingNodeIndicesPrCellCorner[6].push_back((*PK    )[2]);
    if (PIPK  ) contributingNodeIndicesPrCellCorner[6].push_back((*PIPK  )[3]);
    if (PIPJPK) contributingNodeIndicesPrCellCorner[6].push_back((*PIPJPK)[0]);
    if (PJPK  ) contributingNodeIndicesPrCellCorner[6].push_back((*PJPK  )[1]);

    if (IJK   ) contributingNodeIndicesPrCellCorner[7].push_back((*IJK   )[7]);
    if (PJ    ) contributingNodeIndicesPrCellCorner[7].push_back((*PJ    )[4]);
    if (NIPJ  ) contributingNodeIndicesPrCellCorner[7].push_back((*NIPJ  )[5]);
    if (NI    ) contributingNodeIndicesPrCellCorner[7].push_back((*NI    )[6]);
    if (PK    ) contributingNodeIndicesPrCellCorner[7].push_back((*PK    )[3]);
    if (PJPK  ) contributingNodeIndicesPrCellCorner[7].push_back((*PJPK  )[0]);
    if (NIPJPK) contributingNodeIndicesPrCellCorner[7].push_back((*NIPJPK)[1]);
    if (NIPK  ) contributingNodeIndicesPrCellCorner[7].push_back((*NIPK  )[2]);

    // Average the nodes
    for (size_t cornIdx = 0; cornIdx < 8; ++cornIdx)
    {
        estimatedElmCorners[cornIdx] = cvf::Vec3d::ZERO;
        size_t contribCount = contributingNodeIndicesPrCellCorner[cornIdx].size();
        for (size_t ctnIdx = 0; ctnIdx < contribCount; ++ctnIdx)
        {
            estimatedElmCorners[cornIdx] += eclNodes[contributingNodeIndicesPrCellCorner[cornIdx][ctnIdx]];
        }
        estimatedElmCorners[cornIdx] /= contribCount;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void rotateQuad(cvf::Vec3d quad[4], int idxToNewStart)
{
    if (idxToNewStart == 0) return;
    cvf::Vec3d tmpQuad[4];
    tmpQuad[0] = quad[0];
    tmpQuad[1] = quad[1];
    tmpQuad[2] = quad[2];
    tmpQuad[3] = quad[3];

    quad[0] = tmpQuad[idxToNewStart];
    ++idxToNewStart; if (idxToNewStart > 3) idxToNewStart = 0;
    quad[1] = tmpQuad[idxToNewStart];
    ++idxToNewStart; if (idxToNewStart > 3) idxToNewStart = 0;
    quad[2] = tmpQuad[idxToNewStart];
    ++idxToNewStart; if (idxToNewStart > 3) idxToNewStart = 0;
    quad[3] = tmpQuad[idxToNewStart];
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void flipQuadWinding(cvf::Vec3d quad[4])
{
    cvf::Vec3d temp = quad[1];
    quad[1] = quad[3];
    quad[3] = temp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

int quadVxClosestToXYOfPoint( const cvf::Vec3d point, const cvf::Vec3d quad[4])
{
    double minSqDist = HUGE_VAL;
    int quadVxIdxClosestToPoint = cvf::UNDEFINED_INT;

    for (int i = 0; i < 4; ++i)
    {
        cvf::Vec3d diff = quad[i]- point;
        diff[2] = 0.0;

        double sqDist = diff.lengthSquared();
        if (sqDist < minSqDist)
        {
            minSqDist = sqDist;
            quadVxIdxClosestToPoint = i;
        }
    }
    
    return quadVxIdxClosestToPoint;
}


enum RigHexIntersectResult
{
    MATCH, 
    UNRELATED
};


//RigHexIntersectResult isEclFemCellsMatching(cvf::Vec3d eclCorners[8], cvf::Vec3d elmCorners[8])
bool isEclFemCellsMatching(RigMainGrid* eclGrid, size_t reservoirCellIndex,  RigFemPart* femPart, int elmIdx,
                                            double xyTolerance, double zTolerance)
{
    cvf::Vec3d gomConvertedEclCell[8];
    estimatedFemCellFromEclCell(eclGrid, reservoirCellIndex,  gomConvertedEclCell);

    // Find the element top and bottom 
    int femDeepZFaceIdx = 4;
    int femShallowZFaceIdx = 5;

    {
        cvf::Vec3i mainAxisFaces = femPart->structGrid()->findMainIJKFaces(elmIdx);
        femDeepZFaceIdx = mainAxisFaces[2];
        femShallowZFaceIdx = RigFemTypes::oppositeFace(HEX8, femDeepZFaceIdx);
    }

    cvf::Vec3d femDeepestQuad[4];
    cvf::Vec3d femShallowQuad[4];

    {
        const int* cornerIndices = femPart->connectivities(elmIdx);
        const std::vector<cvf::Vec3f>& femNodes =  femPart->nodes().coordinates;
        int faceNodeCount;
        const int*  localElmNodeIndicesForTopZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, femDeepZFaceIdx, &faceNodeCount);
        const int*  localElmNodeIndicesForBotZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, femShallowZFaceIdx, &faceNodeCount);

        femDeepestQuad[0] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForTopZFace[0]]]);
        femDeepestQuad[1] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForTopZFace[1]]]);
        femDeepestQuad[2] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForTopZFace[2]]]);
        femDeepestQuad[3] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForTopZFace[3]]]);
        femShallowQuad[0] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForBotZFace[0]]]);
        femShallowQuad[1] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForBotZFace[1]]]);
        femShallowQuad[2] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForBotZFace[2]]]);
        femShallowQuad[3] = cvf::Vec3d(femNodes[cornerIndices[localElmNodeIndicesForBotZFace[3]]]);
    }

    cvf::Vec3d eclDeepestQuad[4];
    cvf::Vec3d eclShallowQuad[4];


    {
        int faceNodeCount;
        const int*  localElmNodeIndicesForTopZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 4, &faceNodeCount);
        const int*  localElmNodeIndicesForBotZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 5, &faceNodeCount);

        eclDeepestQuad[0] = gomConvertedEclCell[localElmNodeIndicesForTopZFace[0]];
        eclDeepestQuad[1] = gomConvertedEclCell[localElmNodeIndicesForTopZFace[1]];
        eclDeepestQuad[2] = gomConvertedEclCell[localElmNodeIndicesForTopZFace[2]];
        eclDeepestQuad[3] = gomConvertedEclCell[localElmNodeIndicesForTopZFace[3]];

        eclShallowQuad[0] = gomConvertedEclCell[localElmNodeIndicesForBotZFace[0]];
        eclShallowQuad[1] = gomConvertedEclCell[localElmNodeIndicesForBotZFace[1]];
        eclShallowQuad[2] = gomConvertedEclCell[localElmNodeIndicesForBotZFace[2]];
        eclShallowQuad[3] = gomConvertedEclCell[localElmNodeIndicesForBotZFace[3]];
    }

    if (!eclGrid->isFaceNormalsOutwards())
    {
        flipQuadWinding(femShallowQuad);
        flipQuadWinding(femDeepestQuad);
    }

    // Now the top/bottom have opposite winding. To make the comparisons and index rotations simpler
    // flip the winding of the bottom face:
    
    flipQuadWinding(eclShallowQuad);
    flipQuadWinding(femShallowQuad);

    // We now need to rotate the fem quads to be alligned with the ecl quads
    // Since the start point of the quad always is aligned with the opposite face-quad start
    // we can find the rotation for the top, and apply it to both top and bottom

    int femQuadStartIdx = quadVxClosestToXYOfPoint(eclDeepestQuad[0], femDeepestQuad);
    rotateQuad(femDeepestQuad, femQuadStartIdx);
    rotateQuad(femShallowQuad, femQuadStartIdx);

    // Now we should be able to compare vertex for vertex between the ecl and fem quads.

    bool isMatching = true;

    for (int i = 0; i < 4 ; ++i)
    {
        cvf::Vec3d diff = femDeepestQuad[i] - eclDeepestQuad[i];

        if (!(fabs(diff.x()) < xyTolerance &&   fabs(diff.y()) < xyTolerance && fabs(diff.z()) < zTolerance))
        {
            isMatching = false;
            break;
        }
    }

    if (isMatching)
    {
        for (int i = 0; i < 4 ; ++i)
        {
            cvf::Vec3d diff = femShallowQuad[i] - eclShallowQuad[i];

            if (!(fabs(diff.x()) < xyTolerance &&   fabs(diff.y()) < xyTolerance && fabs(diff.z()) < zTolerance))
             {
                isMatching = false;
                break;
            }
        }
    }

    return isMatching;
}

bool elementCorners(RigFemPart* femPart, int elmIdx, cvf::Vec3d elmCorners[8])
{
    if (femPart->elementType(elmIdx) != HEX8) return false;

    const std::vector<cvf::Vec3f>& nodeCoords =  femPart->nodes().coordinates;
    const int* cornerIndices = femPart->connectivities(elmIdx);

    elmCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
    elmCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
    elmCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
    elmCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
    elmCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
    elmCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
    elmCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
    elmCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(NULL),
      m_masterFemPart(dependentFemPart),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
    
    #if 0
    // First search K=1 diagonally for a seed cell; A cell without collapsings, and without faults

    size_t minIJCount = masterEclGrid->cellCountI();
    if (minIJCount > masterEclGrid->cellCountJ())
        minIJCount =  masterEclGrid->cellCountJ();

    for (size_t ij = 0; ij < minIJCount; ++ij )
    {
        size_t localCellIdx = masterEclGrid->cellIndexFromIJK(ij, ij, 0);
        size_t reservoirCellIdx = masterEclGrid->reservoirCellIndex(localCellIdx);

        cvf::Vec3d vertices[8];
        masterEclGrid->cellCornerVertices(localCellIdx, vertices);
        if (!isCellNormal(vertices))
            continue;
        
        const RigFault* fault = masterEclGrid->findFaultFromCellIndexAndCellFace(reservoirCellIdx, cvf::StructGridInterface::POS_I);

    }
    #endif

    // Brute force:
    const std::vector<cvf::Vec3f>& nodeCoords =  dependentFemPart->nodes().coordinates;

    double cellSizeI, cellSizeJ, cellSizeK;
    masterEclGrid->characteristicCellSizes(&cellSizeI, &cellSizeJ, &cellSizeK);
    
    double xyTolerance  = cellSizeI* 0.1;
    double zTolerance   = cellSizeK* 0.1;

    int elementCount = dependentFemPart->elementCount();
    cvf::Vec3d elmCorners[8];
    for (int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
    {
        #ifdef _DEBUG
        { 
            // For debugging 
            size_t i, j, k;
            dependentFemPart->structGrid()->ijkFromCellIndex(elmIdx, &i, &j, &k);
        }
        #endif  
          
        if (!elementCorners(dependentFemPart, elmIdx, elmCorners)) continue;

        cvf::BoundingBox elmBBox;
        for (int i = 0; i < 8 ; ++i) elmBBox.add(elmCorners[i]);

        std::vector<size_t> closeCells;
        masterEclGrid->findIntersectingCells(elmBBox, &closeCells);

        std::vector<int> matchingCells;

        for (size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx)
        {
            cvf::Vec3d cellCorners[8];
            size_t localCellIdx = masterEclGrid->cells()[closeCells[ccIdx]].gridLocalCellIndex();
            masterEclGrid->cellCornerVertices(localCellIdx, cellCorners);

            bool isMatching = isEclFemCellsMatching(masterEclGrid, closeCells[ccIdx], dependentFemPart, elmIdx, xyTolerance, zTolerance);

            if (isMatching)
            {
                matchingCells.push_back(static_cast<int>(closeCells[ccIdx]));
            }
            else
            {
                // Try zero volume correction
            }
        }


        storeMapping(elmIdx, matchingCells);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapper::storeMapping(int depCaseCellIdx, const std::vector<int>& masterCaseMatchingCells)
{
    if (masterCaseMatchingCells.size() == 1)
    {
        m_masterCellOrIntervalIndex[depCaseCellIdx] = masterCaseMatchingCells[0];
    }
    else if (masterCaseMatchingCells.size() > 1)
    {
        m_masterCellOrIntervalIndex[depCaseCellIdx] = -((int)(m_masterCellIndexSeries.size()));
        m_masterCellIndexSeries.push_back(masterCaseMatchingCells);
    }
}


#if 0
//--------------------------------------------------------------------------------------------------
/// Follows the HEX8 type in both RigFemTypes and cvf::StructGridInterface
/// Normals given in POSX, NEGX, POSY, NEGY, POSZ, NEGZ order. Same as face order in the above HEX-es
//--------------------------------------------------------------------------------------------------
cvf::Vec3i findMainXYZFacesOfHex(const cvf::Vec3f normals[6] )
{
    cvf::Vec3i ijkMainFaceIndices = cvf::Vec3i(-1, -1, -1);

    // Record three independent main direction vectors for the element, and what face they are created from

    cvf::Vec3f mainElmDirections[3];
    int mainElmDirOriginFaces[3];

    mainElmDirections[0] = normals[0] - normals[1]; // To get a better "average" direction vector
    mainElmDirections[1] = normals[2] - normals[3];
    mainElmDirections[2] = normals[4] - normals[5];

    mainElmDirOriginFaces[0] = 0;
    mainElmDirOriginFaces[1] = 2;
    mainElmDirOriginFaces[2] = 4;

    
    // Match the element main directions with best XYZ match (IJK respectively)
    // Find the max component of a mainElmDirection. 
    // Assign the index of that mainElmDirection to the mainElmDirectionIdxForIJK at the index of the max component.

    int mainElmDirectionIdxForIJK[3] ={ -1, -1, -1 };
    for (int dIdx = 0; dIdx < 3; ++dIdx)
    {
        double maxAbsComp = 0;
        for (int cIdx = 2; cIdx >= 0 ; --cIdx)
        {
            float absComp = fabs(mainElmDirections[dIdx][cIdx]);
            if (absComp > maxAbsComp)
            {
                maxAbsComp = absComp;
                mainElmDirectionIdxForIJK[cIdx] = dIdx;
            }
        }
    }

    // make sure all the main directions are used

    bool mainDirsUsed[3] ={ false, false, false };
    mainDirsUsed[mainElmDirectionIdxForIJK[0]] = true;
    mainDirsUsed[mainElmDirectionIdxForIJK[1]] = true;
    mainDirsUsed[mainElmDirectionIdxForIJK[2]] = true;

    int unusedDir = -1;
    if (!mainDirsUsed[0]) unusedDir = 0;
    if (!mainDirsUsed[1]) unusedDir = 1;
    if (!mainDirsUsed[2]) unusedDir = 2;

    if (unusedDir >= 0)
    {
        if (mainElmDirectionIdxForIJK[0] == mainElmDirectionIdxForIJK[1]) mainElmDirectionIdxForIJK[0] = unusedDir;
        else if (mainElmDirectionIdxForIJK[1] == mainElmDirectionIdxForIJK[2]) mainElmDirectionIdxForIJK[1] = unusedDir;
        else if (mainElmDirectionIdxForIJK[2] == mainElmDirectionIdxForIJK[0]) mainElmDirectionIdxForIJK[2] = unusedDir;
    }

    // Assign the correct face based on the main direction

    ijkMainFaceIndices[0] = (mainElmDirections[mainElmDirectionIdxForIJK[0]] * cvf::Vec3f::X_AXIS > 0) ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[0]]:  RigFemTypes::oppositeFace(HEX8, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[0]]);
    ijkMainFaceIndices[1] = (mainElmDirections[mainElmDirectionIdxForIJK[1]] * cvf::Vec3f::Y_AXIS > 0) ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[1]]:  RigFemTypes::oppositeFace(HEX8, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[1]]);
    ijkMainFaceIndices[2] = (mainElmDirections[mainElmDirectionIdxForIJK[2]] * -cvf::Vec3f::Z_AXIS > 0) ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[2]]:  RigFemTypes::oppositeFace(HEX8, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[2]]);

    return ijkMainFaceIndices;
}
#endif

#if 0 // Inside Bounding box test
cvf::BoundingBox cellBBox;
for (int i = 0; i < 8 ; ++i) cellBBox.add(cellCorners[i]);

cvf::Vec3d cs = cellBBox.min();
cvf::Vec3d cl = cellBBox.max();
cvf::Vec3d es = elmBBox.min();
cvf::Vec3d el = elmBBox.max();

if (   ( (cs.x() + xyTolerance) >= es.x() && (cl.x() - xyTolerance) <= el.x())
    && ( (cs.y() + xyTolerance) >= es.y() && (cl.y() - xyTolerance) <= el.y())
    && ( (cs.z() + zTolerance ) >= es.z() && (cl.z() - zTolerance ) <= el.z()) )
{
    // Cell bb equal or inside Elm bb   
    isMatching = true;
}

if (   ( (es.x() + xyTolerance)  >= cs.x() && (el.x() - xyTolerance) <= cl.x())
    && ( (es.y() + xyTolerance)  >= cs.y() && (el.y() - xyTolerance) <= cl.y())
    && ( (es.z() + zTolerance )  >= cs.z() && (el.z() - zTolerance ) <= cl.z()) )
{
    // Elm bb equal or inside Cell bb   
    isMatching = true;
}
#endif

#if 0
    {
        const std::vector<cvf::Vec3d>& eclNodes = eclGrid->nodes();
        const RigCell& cell = eclGrid->cells()[reservoirCellIndex];
        const caf::SizeTArray8& cornerIndices = cell.cornerIndices();
        int faceNodeCount;
        const int*  localElmNodeIndicesForTopZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 4, &faceNodeCount);
        const int*  localElmNodeIndicesForBotZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 5, &faceNodeCount);

        eclDeepestQuad[0] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[0]]];
        eclDeepestQuad[1] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[1]]];
        eclDeepestQuad[2] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[2]]];
        eclDeepestQuad[3] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[3]]];

        eclShallowQuad[0] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[0]]];
        eclShallowQuad[1] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[1]]];
        eclShallowQuad[2] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[2]]];
        eclShallowQuad[3] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[3]]];
    }
#endif