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

//==================================================================================================
/// 
//==================================================================================================
#include "RigGeoMechWellLogExtractor.h"

#include "RiaDefines.h"
#include "RigFemTypes.h"
#include "RigGeoMechBoreHoleStressCalculator.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartResultsCollection.h"

#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "cafTensor3.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::RigGeoMechWellLogExtractor(RigGeoMechCaseData* aCase,
                                                       const RigWellPath*  wellpath,
                                                       const std::string&  wellCaseErrorMsgName)
    : RigWellLogExtractor(wellpath, wellCaseErrorMsgName)
    , m_caseData(aCase)
{
    calculateIntersection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::curveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values)
{   
    CVF_TIGHT_ASSERT(values);
    
    if (resAddr.resultPosType == RIG_WELLPATH_DERIVED)
    {
        if (resAddr.fieldName == RiaDefines::wellPathFGResultName().toStdString() || resAddr.fieldName == RiaDefines::wellPathSFGResultName().toStdString())
        {
            wellBoreWallCurveData(resAddr, frameIndex, values);
            return;
        }
        else if (resAddr.fieldName == "PP" || resAddr.fieldName == "OBG" || resAddr.fieldName == "SH")
        {
            wellPathScaledCurveData(resAddr, frameIndex, values);
            return;
        }
        else if (resAddr.fieldName == "Azimuth" || resAddr.fieldName == "Inclination")
        {
            wellPathAngles(resAddr, values);
            return;
        }

    }

    if (!resAddr.isValid()) return;

    RigFemResultAddress convResAddr = resAddr;

    // When showing POR results, always use the element nodal result, 
    // to get correct handling of elements without POR results
     
    if (convResAddr.fieldName == "POR-Bar") convResAddr.resultPosType = RIG_ELEMENT_NODAL;

    CVF_ASSERT(resAddr.resultPosType != RIG_WELLPATH_DERIVED);

    const RigFemPart* femPart                 = m_caseData->femParts()->part(0);
    const std::vector<float>& resultValues    = m_caseData->femPartResults()->resultValues(convResAddr, 0, frameIndex);

    if (!resultValues.size()) return;

    values->resize(m_intersections.size());

    for (size_t intersectionIdx = 0; intersectionIdx < m_intersections.size(); ++intersectionIdx)
    {
        (*values)[intersectionIdx] = static_cast<double>(interpolateGridResultValue<float>(convResAddr.resultPosType, resultValues, intersectionIdx, false));
    }
  
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellPathAngles(const RigFemResultAddress& resAddr, std::vector<double>* values)
{
    CVF_ASSERT(values);
    CVF_ASSERT(resAddr.fieldName == "Azimuth" || resAddr.fieldName == "Inclination");
    values->resize(m_intersections.size(), 0.0f);
    const double epsilon = 1.0e-6 * 360;
    const cvf::Vec3d trueNorth(0.0, 1.0, 0.0);
    const cvf::Vec3d up(0.0, 0.0, 1.0);
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx)
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];

        cvf::Vec3d wellPathTangent = calculateWellPathTangent(intersectionIdx);
   
        // Deviation from vertical. Since well path is tending downwards we compare with negative z.
        double inclination = cvf::Math::toDegrees(std::acos(cvf::Vec3d(0.0, 0.0, -1.0) * wellPathTangent.getNormalized()));       

        if (resAddr.fieldName == "Azimuth")
        {
            double azimuth = HUGE_VAL;
            
            // Azimuth is not defined when well path is vertical. We define it as infinite to avoid it showing up in the plot.
            if (cvf::Math::valueInRange(inclination, epsilon, 180.0 - epsilon))
            {
                cvf::Vec3d projectedTangentXY = wellPathTangent;
                projectedTangentXY.z() = 0.0;

                // Do tangentXY to true north for counter clockwise angles.
                double dotProduct = projectedTangentXY * trueNorth;
                double crossProduct = (projectedTangentXY ^ trueNorth) * up;
                // http://www.glossary.oilfield.slb.com/Terms/a/azimuth.aspx
                azimuth = cvf::Math::toDegrees(std::atan2(crossProduct, dotProduct));
                if (azimuth < 0.0)
                {
                    // Straight atan2 gives angle from -PI to PI yielding angles from -180 to 180.
                    // We want angles from 0 to 360, so add 180 degrees.
                    azimuth = azimuth + 360.0;
                }
            }

            (*values)[intersectionIdx] = azimuth;
        }
        else
        {
            (*values)[intersectionIdx] = inclination;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellPathScaledCurveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values)
{
    CVF_ASSERT(values);

    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::string nativeFieldName;
    std::string nativeCompName;
    double scalingFactor = 1000 * 9.81 / 1.0e5;
    if (resAddr.fieldName == "PP")
    {
        nativeFieldName = "POR-Bar"; // More likely to be in memory than POR
    }
    else if (resAddr.fieldName == "OBG")
    {
        nativeFieldName = "ST";
        nativeCompName = "S33";
    }
    else if (resAddr.fieldName == "SH")
    {
        nativeFieldName = "ST";
        nativeCompName = "S3";
    }

    RigFemResultAddress nativeAddr(RIG_ELEMENT_NODAL, nativeFieldName, nativeCompName);
    std::vector<float> unscaledResult = resultCollection->resultValues(nativeAddr, 0, frameIndex);

    values->resize(m_intersections.size(), 0.0f);

#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx)
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);

        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        double trueVerticalDepth = -m_intersections[intersectionIdx].z();
        double effectiveDepth = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressure = effectiveDepth * 9.81 / 100.0;

        double unscaledValue = static_cast<double>(interpolateGridResultValue<float>(nativeAddr.resultPosType, unscaledResult, intersectionIdx, false));
        if (resAddr.fieldName == "PP" && (unscaledValue == std::numeric_limits<float>::infinity() ||
                                          unscaledValue == -std::numeric_limits<float>::infinity()))
        {
            unscaledValue = hydroStaticPorePressure;
        }
        double scaledValue = unscaledValue / (scalingFactor * effectiveDepth);
        (*values)[intersectionIdx] = scaledValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellBoreWallCurveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values)
{
    // TODO: Read in these values:
    const double poissonRatio = 0.25; // TODO: Read this in.
    // Typical UCS: http://ceae.colorado.edu/~amadei/CVEN5768/PDF/NOTES8.pdf
    // Typical UCS for Shale is 5 - 100 MPa -> 50 - 1000 bar.
    const double uniaxialStrengthInBars = 100.0;

    CVF_ASSERT(values);
    CVF_ASSERT(resAddr.fieldName == RiaDefines::wellPathFGResultName().toStdString() || resAddr.fieldName == RiaDefines::wellPathSFGResultName().toStdString());

    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    RigFemResultAddress stressResAddr(RIG_ELEMENT_NODAL, std::string("ST"), "");
    stressResAddr.fieldName = std::string("ST");

    RigFemResultAddress porBarResAddr(RIG_ELEMENT_NODAL, std::string("POR-Bar"), "");

    std::vector<caf::Ten3f> vertexStressesFloat = resultCollection->tensors(stressResAddr, 0, frameIndex);
    
    if (!vertexStressesFloat.size()) return;

    std::vector<caf::Ten3d> vertexStresses; vertexStresses.reserve(vertexStressesFloat.size());
    for (const caf::Ten3f& floatTensor : vertexStressesFloat)
    {
        vertexStresses.push_back(caf::Ten3d(floatTensor));
    }

    values->resize(m_intersections.size(), 0.0f);

    std::vector<float> porePressures = resultCollection->resultValues(porBarResAddr, 0, frameIndex);

#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t) m_intersections.size(); ++intersectionIdx)
    {        
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);

        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        double trueVerticalDepth = -m_intersections[intersectionIdx].z();
        double porePressure = trueVerticalDepth * 9.81 / 100.0;
        if (!porePressures.empty())
        {
            float interpolatedPorePressure = interpolateGridResultValue(porBarResAddr.resultPosType, porePressures, intersectionIdx, false);
            if (interpolatedPorePressure != std::numeric_limits<float>::infinity() &&
                interpolatedPorePressure != -std::numeric_limits<float>::infinity())
            {
                porePressure = static_cast<double>(interpolatedPorePressure);
            }
        }

        caf::Ten3d interpolatedStress = interpolateGridResultValue(stressResAddr.resultPosType, vertexStresses, intersectionIdx, false);
        cvf::Vec3d wellPathTangent = calculateWellPathTangent(intersectionIdx);
        caf::Ten3d wellPathStressFloat = transformTensorToWellPathOrientation(wellPathTangent, interpolatedStress);
        caf::Ten3d wellPathStressDouble(wellPathStressFloat);

        RigGeoMechBoreHoleStressCalculator sigmaCalculator(wellPathStressDouble, porePressure, poissonRatio, uniaxialStrengthInBars, 32);
        double resultValue = 0.0;
        if (resAddr.fieldName == RiaDefines::wellPathFGResultName().toStdString())
        {
            resultValue = sigmaCalculator.solveFractureGradient();
        }
        else
        {
            CVF_ASSERT(resAddr.fieldName == RiaDefines::wellPathSFGResultName().toStdString());
            resultValue = sigmaCalculator.solveStassiDalia();
        }
        double effectiveDepth = -m_intersections[intersectionIdx].z() + m_rkbDiff;
        if (effectiveDepth > 1.0e-8)
        {
            resultValue *= 100.0 / (effectiveDepth * 9.81);
        }
        else
        {
            resultValue = std::numeric_limits<double>::infinity();
        }

        (*values)[intersectionIdx] = resultValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGeoMechCaseData* RigGeoMechWellLogExtractor::caseData()
{
    return m_caseData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setRkbDiff(double rkbDiff)
{
    m_rkbDiff = rkbDiff;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename T>
T RigGeoMechWellLogExtractor::interpolateGridResultValue(RigFemResultPosEnum   resultPosType,
                                                      const std::vector<T>& gridResultValues,
                                                      int64_t               intersectionIdx,
                                                      bool                  averageNodeElementResults) const
{
    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
    RigElementType elmType = femPart->elementType(elmIdx);

    if (!(elmType == HEX8 || elmType == HEX8P)) return T();


    if (resultPosType == RIG_ELEMENT)
    {
        return gridResultValues[elmIdx];        
    }

    cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[intersectionIdx];

    if (cellFace == cvf::StructGridInterface::NO_FACE)
    {
        // TODO: Should interpolate within the whole hexahedron. This requires converting to locals coordinates.
        // For now just pick the average value for the cell.
        T sumOfVertexValues = gridResultValues[femPart->elementNodeResultIdx(static_cast<int>(elmIdx), 0)];
        for (int i = 1; i < 8; ++i)
        {
            sumOfVertexValues = sumOfVertexValues + gridResultValues[femPart->elementNodeResultIdx(static_cast<int>(elmIdx), i)];
        }
        return sumOfVertexValues * (1.0 / 8.0);
    }

    int faceNodeCount = 0;
    const int* faceLocalIndices = RigFemTypes::localElmNodeIndicesForFace(elmType, cellFace, &faceNodeCount);
    const int* elmNodeIndices = femPart->connectivities(elmIdx);

    cvf::Vec3d v0(nodeCoords[elmNodeIndices[faceLocalIndices[0]]]);
    cvf::Vec3d v1(nodeCoords[elmNodeIndices[faceLocalIndices[1]]]);
    cvf::Vec3d v2(nodeCoords[elmNodeIndices[faceLocalIndices[2]]]);
    cvf::Vec3d v3(nodeCoords[elmNodeIndices[faceLocalIndices[3]]]);

    std::vector<size_t> nodeResIdx(4, cvf::UNDEFINED_SIZE_T);

    if (resultPosType == RIG_NODAL)
    {
        for (size_t i = 0; i < nodeResIdx.size(); ++i)
        {
            nodeResIdx[i] = elmNodeIndices[faceLocalIndices[i]];
        }
    }
    else
    {
        for (size_t i = 0; i < nodeResIdx.size(); ++i)
        {
            nodeResIdx[i] = (size_t)femPart->elementNodeResultIdx((int)elmIdx, faceLocalIndices[i]);
        }
    }

    std::vector<T> nodeResultValues;
    nodeResultValues.reserve(4);
    if (resultPosType == RIG_ELEMENT_NODAL && averageNodeElementResults)
    {
        // Estimate nodal values as the average of the node values from each connected element.
        for (size_t i = 0; i < nodeResIdx.size(); ++i)
        {
            int nodeIndex = femPart->nodeIdxFromElementNodeResultIdx(nodeResIdx[i]);
            const std::vector<int>& elements = femPart->elementsUsingNode(nodeIndex);
            const std::vector<unsigned char>& localIndices = femPart->elementLocalIndicesForNode(nodeIndex);
            size_t otherNodeResIdx = femPart->elementNodeResultIdx(elements[0], static_cast<int>(localIndices[0]));
            T nodeResultValue = gridResultValues[otherNodeResIdx];
            for (size_t j = 1; j < elements.size(); ++j)
            {
                otherNodeResIdx = femPart->elementNodeResultIdx(elements[j], static_cast<int>(localIndices[j]));
                nodeResultValue = nodeResultValue + gridResultValues[otherNodeResIdx];
            }
            nodeResultValue = nodeResultValue * (1.0 / elements.size());
            nodeResultValues.push_back(nodeResultValue);
        }
    }
    else {
        for (size_t i = 0; i < nodeResIdx.size(); ++i)
        {
            nodeResultValues.push_back(gridResultValues[nodeResIdx[i]]);
        }
    }

    T interpolatedValue = cvf::GeometryTools::interpolateQuad<T>(
        v0, nodeResultValues[0],
        v1, nodeResultValues[1],
        v2, nodeResultValues[2],
        v3, nodeResultValues[3],
        m_intersections[intersectionIdx]
    );

    return interpolatedValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::calculateIntersection()
{
    CVF_ASSERT(m_caseData->femParts()->partCount() == 1);

    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo > uniqueIntersections;

    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords =  femPart->nodes().coordinates;

    for (size_t wpp = 0; wpp < m_wellPath->m_wellPathPoints.size() - 1; ++wpp)
    {
        std::vector<HexIntersectionInfo> intersections;
        cvf::Vec3d p1 = m_wellPath->m_wellPathPoints[wpp];
        cvf::Vec3d p2 = m_wellPath->m_wellPathPoints[wpp+1];

        cvf::BoundingBox bb;

        bb.add(p1);
        bb.add(p2);

        std::vector<size_t> closeCells = findCloseCells(bb);

        cvf::Vec3d hexCorners[8];
        for (size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx)
        {
            RigElementType elmType = femPart->elementType(closeCells[ccIdx]);
            if (!(elmType == HEX8 || elmType == HEX8P)) continue;

            const int* cornerIndices = femPart->connectivities(closeCells[ccIdx]);

            hexCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
            hexCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
            hexCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
            hexCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
            hexCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
            hexCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
            hexCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
            hexCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

            //int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[ccIdx], &intersections);
            RigHexIntersectionTools::lineHexCellIntersection(p1, p2, hexCorners, closeCells[ccIdx], &intersections);
        }

        // Now, with all the intersections of this piece of line, we need to 
        // sort them in order, and set the measured depth and corresponding cell index

        // Inserting the intersections in this map will remove identical intersections
        // and sort them according to MD, CellIdx, Leave/enter

        double md1 = m_wellPath->m_measuredDepths[wpp];
        double md2 = m_wellPath->m_measuredDepths[wpp+1];

        insertIntersectionsInMap(intersections,
                                 p1, md1, p2, md2,
                                 &uniqueIntersections);
    }

    this->populateReturnArrays(uniqueIntersections);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigGeoMechWellLogExtractor::findCloseCells(const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;

    if (m_caseData->femParts()->partCount())
    {
        m_caseData->femParts()->part(0)->findIntersectingCells(bb, &closeCells);
    }
    return closeCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGeoMechWellLogExtractor::calculateLengthInCell(size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords =  femPart->nodes().coordinates;
    const int* cornerIndices = femPart->connectivities(cellIndex);

    hexCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
    hexCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
    hexCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
    hexCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
    hexCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
    hexCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
    hexCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
    hexCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

    return RigWellPathIntersectionTools::calculateLengthInCell(hexCorners, startPoint, endPoint); 
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGeoMechWellLogExtractor::calculateWellPathTangent(int64_t intersectionIdx) const
{
    cvf::Vec3d segmentStart, segmentEnd;
    m_wellPath->twoClosestPoints(m_intersections[intersectionIdx], &segmentStart, &segmentEnd);
    return (segmentEnd - segmentStart).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Ten3d RigGeoMechWellLogExtractor::transformTensorToWellPathOrientation(const cvf::Vec3d& wellPathTangent,
                                                                            const caf::Ten3d& tensor)
{
    // Create local coordinate system for well path segment
    cvf::Vec3d local_z = wellPathTangent;
    cvf::Vec3d local_x = local_z.perpendicularVector().getNormalized();
    cvf::Vec3d local_y = (local_z ^ local_x).getNormalized();
    // Calculate the rotation matrix from global i, j, k to local x, y, z.
    cvf::Mat4d rotationMatrix = cvf::Mat4d::fromCoordSystemAxes(&local_x, &local_y, &local_z);

    return tensor.rotated(rotationMatrix.toMatrix3());
}

