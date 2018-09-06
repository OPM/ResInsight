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
#include "RiaWeightedAverageCalculator.h"
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

#include <type_traits>

const double RigGeoMechWellLogExtractor::UNIT_WEIGHT_OF_WATER = 9.81 * 1000.0; // N / m^3

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
float RigGeoMechWellLogExtractor::calculatePorePressureInSegment(int64_t intersectionIdx, float averageSegmentPorePressureBar, double hydroStaticPorePressureBar, double effectiveDepthMeters, const std::vector<float>& poreElementPressuresPascal) const
{
    double porePressure = hydroStaticPorePressureBar;

    // 1: Try pore pressure from the grid
    if (porePressure == hydroStaticPorePressureBar && averageSegmentPorePressureBar != std::numeric_limits<float>::infinity())
    {
        porePressure = averageSegmentPorePressureBar;
    }

    // 2: Try mud weight from LAS-file to generate pore pressure
    if (porePressure == hydroStaticPorePressureBar && !m_wellLogMdAndMudWeightKgPerM3.empty())
    {
        double lasMudWeightKgPerM3 = getWellLogSegmentValue(intersectionIdx, m_wellLogMdAndMudWeightKgPerM3);
        if (lasMudWeightKgPerM3 != std::numeric_limits<double>::infinity())
        {
            double specificMudWeightNPerM3 = lasMudWeightKgPerM3 * 9.81;
            double porePressurePascal = specificMudWeightNPerM3 * effectiveDepthMeters;
            porePressure = pascalToBar(porePressurePascal);
        }
    }
    size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
    // 3: Try pore pressure from element property tables
    if (porePressure == hydroStaticPorePressureBar && elmIdx < poreElementPressuresPascal.size())
    {
        // Pore pressure from element property tables are in pascal.
        porePressure = pascalToBar(poreElementPressuresPascal[elmIdx]);
    }
    // 4: If no pore-pressure was found, the default value of hydrostatic pore pressure is used.
    return porePressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigGeoMechWellLogExtractor::calculatePoissonRatio(int64_t intersectionIdx, const std::vector<float>& poissonRatios) const
{
    const double defaultPoissonRatio = 0.25;

    double poissonRatio = defaultPoissonRatio;

    if (!m_wellLogMdAndPoissonRatios.empty())
    {
        double lasPoissionRatio = getWellLogSegmentValue(intersectionIdx, m_wellLogMdAndPoissonRatios);
        if (lasPoissionRatio != std::numeric_limits<double>::infinity())
        {
            poissonRatio = lasPoissionRatio;
        }
    }

    size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
    if (poissonRatio == defaultPoissonRatio && elmIdx < poissonRatios.size())
    {
        poissonRatio = poissonRatios[elmIdx];
    }
    return poissonRatio;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigGeoMechWellLogExtractor::calculateUcs(int64_t intersectionIdx, const std::vector<float>& ucsValuesPascal) const
{
    // Typical UCS: http://ceae.colorado.edu/~amadei/CVEN5768/PDF/NOTES8.pdf
    // Typical UCS for Shale is 5 - 100 MPa -> 50 - 1000 bar.
    const double defaultUniaxialStrengthInBar = 100.0;

    double uniaxialStrengthInBar = defaultUniaxialStrengthInBar;
    if (!m_wellLogMdAndUcsBar.empty())
    {
        double lasUniaxialStrengthInBar = getWellLogSegmentValue(intersectionIdx, m_wellLogMdAndUcsBar);
        if (lasUniaxialStrengthInBar != std::numeric_limits<double>::infinity())
        {
            uniaxialStrengthInBar = lasUniaxialStrengthInBar;
        }
    }

    size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
    if (uniaxialStrengthInBar == defaultUniaxialStrengthInBar && elmIdx < ucsValuesPascal.size())
    {
        // Read UCS from element table in Pascal
        uniaxialStrengthInBar = pascalToBar(ucsValuesPascal[elmIdx]);
    }
    return uniaxialStrengthInBar;
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

        cvf::Vec3d wellPathTangent = calculateWellPathTangent(intersectionIdx, TangentFollowWellPathSegments);
   
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

                // Do tangentXY to true north for clockwise angles.
                double dotProduct = projectedTangentXY * trueNorth;
                double crossProduct = (projectedTangentXY ^ trueNorth) * up;
                // http://www.glossary.oilfield.slb.com/Terms/a/azimuth.aspx
                azimuth = cvf::Math::toDegrees(std::atan2(crossProduct, dotProduct));
                if (azimuth < 0.0)
                {
                    // Straight atan2 gives angle from -PI to PI yielding angles from -180 to 180
                    // where the negative angles are counter clockwise.
                    // To get all positive clockwise angles, we add 360 degrees to negative angles.
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
    const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::string nativeFieldName;
    std::string nativeCompName;
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
    RigFemResultAddress porElementResAddr(RIG_ELEMENT, "POR", "");

    std::vector<float> unscaledResultValues = resultCollection->resultValues(nativeAddr, 0, frameIndex);
    std::vector<float> poreElementPressuresPascal = resultCollection->resultValues(porElementResAddr, 0, frameIndex);

    std::vector<float> interpolatedInterfaceValues;
    interpolatedInterfaceValues.resize(m_intersections.size(), std::numeric_limits<double>::infinity());

#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx)
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);
        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        interpolatedInterfaceValues[intersectionIdx] = interpolateGridResultValue<float>(nativeAddr.resultPosType, unscaledResultValues, intersectionIdx, false);
    }

    values->resize(m_intersections.size(), 0.0f);

#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx)
    {
        // Set the value to invalid by default
        (*values)[intersectionIdx] = std::numeric_limits<double>::infinity();

        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);

        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        cvf::Vec3f centroid = cellCentroid(intersectionIdx);

        double trueVerticalDepth = -centroid.z();
        
        double effectiveDepthMeters = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressureBar = pascalToBar(effectiveDepthMeters * UNIT_WEIGHT_OF_WATER);

        float averageUnscaledValue = std::numeric_limits<float>::infinity();
        bool validAverage = averageIntersectionValuesToSegmentValue(intersectionIdx, interpolatedInterfaceValues, std::numeric_limits<float>::infinity(), &averageUnscaledValue);

        if (resAddr.fieldName == "PP")
        {
            double segmentPorePressureFromGrid = averageUnscaledValue;
            averageUnscaledValue = calculatePorePressureInSegment(intersectionIdx, segmentPorePressureFromGrid, hydroStaticPorePressureBar, effectiveDepthMeters, poreElementPressuresPascal);

        }

        (*values)[intersectionIdx] = static_cast<double>(averageUnscaledValue) / hydroStaticPorePressureBar;            
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellBoreWallCurveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values)
{
    CVF_ASSERT(values);
    CVF_ASSERT(resAddr.fieldName == RiaDefines::wellPathFGResultName().toStdString() || resAddr.fieldName == RiaDefines::wellPathSFGResultName().toStdString());

    // The result addresses needed
    RigFemResultAddress stressResAddr(RIG_ELEMENT_NODAL, "ST", "");
    RigFemResultAddress porBarResAddr(RIG_ELEMENT_NODAL, "POR-Bar", "");
    // Allow POR as an element property value
    RigFemResultAddress porElementResAddr(RIG_ELEMENT, "POR", "");
    RigFemResultAddress poissonResAddr(RIG_ELEMENT, "POISSONS_RATIO", "");
    RigFemResultAddress ucsResAddr(RIG_ELEMENT, "UCS", "");
    
    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    // Load results
    std::vector<caf::Ten3f> vertexStressesFloat = resultCollection->tensors(stressResAddr, 0, frameIndex);   
    if (!vertexStressesFloat.size()) return;

    std::vector<caf::Ten3d> vertexStresses; vertexStresses.reserve(vertexStressesFloat.size());
    for (const caf::Ten3f& floatTensor : vertexStressesFloat)
    {
        vertexStresses.push_back(caf::Ten3d(floatTensor));
    }
    std::vector<float> porePressures              = resultCollection->resultValues(porBarResAddr, 0, frameIndex);
    std::vector<float> poreElementPressuresPascal = resultCollection->resultValues(porElementResAddr, 0, frameIndex);
    std::vector<float> poissonRatios              = resultCollection->resultValues(poissonResAddr, 0, frameIndex);
    std::vector<float> ucsValuesPascal            = resultCollection->resultValues(ucsResAddr, 0, frameIndex);

    std::vector<float> interpolatedInterfacePorePressureBar;
    interpolatedInterfacePorePressureBar.resize(m_intersections.size(), std::numeric_limits<double>::infinity());

    std::vector<caf::Ten3d> interpolatedInterfaceStressBar;
    interpolatedInterfaceStressBar.resize(m_intersections.size());
#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx)
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);
        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        interpolatedInterfacePorePressureBar[intersectionIdx]     = interpolateGridResultValue(porBarResAddr.resultPosType, porePressures, intersectionIdx, false);
        interpolatedInterfaceStressBar[intersectionIdx] = interpolateGridResultValue(stressResAddr.resultPosType, vertexStresses, intersectionIdx, false);
    }

    values->resize(m_intersections.size(), 0.0f);
    
#pragma omp parallel for
    for (int64_t intersectionIdx = 0; intersectionIdx < (int64_t) m_intersections.size(); ++intersectionIdx)
    {        
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType(elmIdx);

        if (!(elmType == HEX8 || elmType == HEX8P)) continue;

        cvf::Vec3f centroid = cellCentroid(intersectionIdx);

        double trueVerticalDepth = -centroid.z();
        double effectiveDepthMeters = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressureBar = pascalToBar(effectiveDepthMeters * UNIT_WEIGHT_OF_WATER);

        float averagePorePressureBar = std::numeric_limits<float>::infinity();
        bool validGridPorePressure = averageIntersectionValuesToSegmentValue(intersectionIdx, interpolatedInterfacePorePressureBar, std::numeric_limits<float>::infinity(), &averagePorePressureBar);

        double porePressureBar = calculatePorePressureInSegment(intersectionIdx, averagePorePressureBar, hydroStaticPorePressureBar, effectiveDepthMeters, poreElementPressuresPascal);
        double poissonRatio    = calculatePoissonRatio(intersectionIdx, poissonRatios);
        double ucsBar          = calculateUcs(intersectionIdx, ucsValuesPascal);

        caf::Ten3d segmentStress;
        bool validSegmentStress = averageIntersectionValuesToSegmentValue(intersectionIdx, interpolatedInterfaceStressBar, caf::Ten3d::invalid(), &segmentStress);

        cvf::Vec3d wellPathTangent = calculateWellPathTangent(intersectionIdx, TangentConstantWithinCell);
        caf::Ten3d wellPathStressFloat = transformTensorToWellPathOrientation(wellPathTangent, segmentStress);
        caf::Ten3d wellPathStressDouble(wellPathStressFloat);

        RigGeoMechBoreHoleStressCalculator sigmaCalculator(wellPathStressDouble, porePressureBar, poissonRatio, ucsBar, 32);
        double resultValue = std::numeric_limits<double>::infinity();
        if (resAddr.fieldName == RiaDefines::wellPathFGResultName().toStdString())
        {
            if (validGridPorePressure)
            {
                resultValue = sigmaCalculator.solveFractureGradient();
            }
        }
        else
        {
            CVF_ASSERT(resAddr.fieldName == RiaDefines::wellPathSFGResultName().toStdString());
            if (!validGridPorePressure)
            {
                resultValue = sigmaCalculator.solveStassiDalia();
            }
        }
        if (resultValue != std::numeric_limits<double>::infinity())
        {
            if (hydroStaticPorePressureBar > 1.0e-8)
            {
                resultValue /= hydroStaticPorePressureBar;
            }
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
void RigGeoMechWellLogExtractor::setWellLogMdAndMudWeightKgPerM3(const std::vector<std::pair<double, double>>& porePressures)
{
    m_wellLogMdAndMudWeightKgPerM3 = porePressures;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWellLogMdAndUcsBar(const std::vector<std::pair<double, double>>& ucsValues)
{
    m_wellLogMdAndUcsBar = ucsValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWellLogMdAndPoissonRatio(const std::vector<std::pair<double, double>>& poissonRatios)
{
    m_wellLogMdAndPoissonRatios = poissonRatios;
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

    if (resultPosType == RIG_FORMATION_NAMES)
    {
        resultPosType = RIG_ELEMENT_NODAL; // formation indices are stored per element node result.
    }

    if (resultPosType == RIG_ELEMENT)
    {
        return gridResultValues[elmIdx];        
    }

    cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[intersectionIdx];

    if (cellFace == cvf::StructGridInterface::NO_FACE)
    {
        if (resultPosType == RIG_ELEMENT_NODAL_FACE)
        {
            return std::numeric_limits<T>::infinity(); // undefined value. ELEMENT_NODAL_FACE values are only defined on a face.
        }
        // TODO: Should interpolate within the whole hexahedron. This requires converting to locals coordinates.
        // For now just pick the average value for the cell.
        size_t gridResultValueIdx = femPart->resultValueIdxFromResultPosType(resultPosType, static_cast<int>(elmIdx), 0);
        T sumOfVertexValues = gridResultValues[gridResultValueIdx];
        for (int i = 1; i < 8; ++i)
        {
            gridResultValueIdx = femPart->resultValueIdxFromResultPosType(resultPosType, static_cast<int>(elmIdx), i);
            sumOfVertexValues = sumOfVertexValues + gridResultValues[gridResultValueIdx];
        }
        return sumOfVertexValues * (1.0 / 8.0);
    }

    int faceNodeCount = 0;
    const int* elementLocalIndicesForFace = RigFemTypes::localElmNodeIndicesForFace(elmType, cellFace, &faceNodeCount);
    const int* elmNodeIndices = femPart->connectivities(elmIdx);

    cvf::Vec3d v0(nodeCoords[elmNodeIndices[elementLocalIndicesForFace[0]]]);
    cvf::Vec3d v1(nodeCoords[elmNodeIndices[elementLocalIndicesForFace[1]]]);
    cvf::Vec3d v2(nodeCoords[elmNodeIndices[elementLocalIndicesForFace[2]]]);
    cvf::Vec3d v3(nodeCoords[elmNodeIndices[elementLocalIndicesForFace[3]]]);

    std::vector<size_t> nodeResIdx(4, cvf::UNDEFINED_SIZE_T);

    for (size_t i = 0; i < nodeResIdx.size(); ++i)
    {
        if (resultPosType == RIG_ELEMENT_NODAL_FACE)
        {
            nodeResIdx[i] = gridResultIndexFace(elmIdx, cellFace, static_cast<int>(i));
        }
        else
        {
            nodeResIdx[i] = femPart->resultValueIdxFromResultPosType(resultPosType, static_cast<int>(elmIdx), elementLocalIndicesForFace[i]);
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
            size_t otherGridResultValueIdx = femPart->resultValueIdxFromResultPosType(resultPosType, elements[0], static_cast<int>(localIndices[0]));
            T nodeResultValue = gridResultValues[otherGridResultValueIdx];
            for (size_t j = 1; j < elements.size(); ++j)
            {
                otherGridResultValueIdx = femPart->resultValueIdxFromResultPosType(resultPosType, elements[j], static_cast<int>(localIndices[j]));
                nodeResultValue = nodeResultValue + gridResultValues[otherGridResultValueIdx];
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
size_t RigGeoMechWellLogExtractor::gridResultIndexFace(size_t elementIdx, cvf::StructGridInterface::FaceType cellFace, int faceLocalNodeIdx) const
{
    CVF_ASSERT(cellFace != cvf::StructGridInterface::NO_FACE && faceLocalNodeIdx < 4);
    return elementIdx * 24 + static_cast<int>(cellFace) * 4 + faceLocalNodeIdx;
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
cvf::Vec3d RigGeoMechWellLogExtractor::calculateWellPathTangent(int64_t                    intersectionIdx,
                                                                WellPathTangentCalculation calculationType) const
{
    if (calculationType == TangentFollowWellPathSegments)
    {
        cvf::Vec3d segmentStart, segmentEnd;
        m_wellPath->twoClosestPoints(m_intersections[intersectionIdx], &segmentStart, &segmentEnd);
        return (segmentEnd - segmentStart).getNormalized();
    }
    else
    {
        cvf::Vec3d wellPathTangent;
        if (intersectionIdx % 2 == 0)
        {
            wellPathTangent = m_intersections[intersectionIdx + 1] - m_intersections[intersectionIdx];
        }
        else
        {
            wellPathTangent = m_intersections[intersectionIdx] - m_intersections[intersectionIdx - 1];
        }
        CVF_ASSERT(wellPathTangent.length() > 1.0e-7);
        return wellPathTangent.getNormalized();
    }
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RigGeoMechWellLogExtractor::cellCentroid(size_t intersectionIdx) const
{
    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
    RigElementType elmType = femPart->elementType(elmIdx);
    int elementNodeCount = RigFemTypes::elmentNodeCount(elmType);

    const int* elmNodeIndices = femPart->connectivities(elmIdx);

    cvf::Vec3f centroid(0.0, 0.0, 0.0);
    for (int i = 0; i < elementNodeCount; ++i)
    {
        centroid += nodeCoords[elmNodeIndices[i]];
    }
    return centroid / elementNodeCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::getWellLogSegmentValue(size_t intersectionIdx, const std::vector<std::pair<double, double>>& wellLogValues) const
{
    const RigFemPart* femPart = m_caseData->femParts()->part(0);

    double startMD, endMD;
    if (intersectionIdx % 2 == 0)
    {
        startMD = m_intersectionMeasuredDepths[intersectionIdx];
        endMD   = m_intersectionMeasuredDepths[intersectionIdx + 1];
    }
    else
    {
        startMD = m_intersectionMeasuredDepths [intersectionIdx - 1];
        endMD = m_intersectionMeasuredDepths[intersectionIdx];
    }

    RiaWeightedAverageCalculator<double> averageCalc;
    for (auto& depthAndValue : wellLogValues)
    {
        if (cvf::Math::valueInRange(depthAndValue.first, startMD, endMD))
        {
            cvf::Vec3d position = m_wellPath->interpolatedPointAlongWellPath(depthAndValue.first);
            cvf::Vec3d centroid(cellCentroid(intersectionIdx));
            double weight = 1.0;
            double dist = (position - centroid).length();
            if (dist > 1.0)
            {
                weight = 1.0 / dist;
            }
            averageCalc.addValueAndWeight(depthAndValue.second, weight);
        }
    }
    if (averageCalc.validAggregatedWeight())
    {
        return averageCalc.weightedAverage();
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::pascalToBar(double pascalValue)
{
    return pascalValue * 1.0e-5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename T>
bool RigGeoMechWellLogExtractor::averageIntersectionValuesToSegmentValue(size_t intersectionIdx, const std::vector<T>& values, const T& invalidValue, T* averagedCellValue) const
{
    CVF_ASSERT(values.size() >= 2);
    
    *averagedCellValue = invalidValue;

    T value1, value2;
    cvf::Vec3d centroid(cellCentroid(intersectionIdx));
    double dist1 = 0.0, dist2 = 0.0;
    if (intersectionIdx % 2 == 0)
    {
        value1 = values[intersectionIdx];
        value2 = values[intersectionIdx + 1];

        dist1 = (centroid - m_intersections[intersectionIdx]).length();
        dist2 = (centroid - m_intersections[intersectionIdx + 1]).length();
    }
    else {
        value1 = values[intersectionIdx - 1];
        value2 = values[intersectionIdx];

        dist1 = (centroid - m_intersections[intersectionIdx - 1]).length();
        dist2 = (centroid - m_intersections[intersectionIdx]).length();
    }

    if (invalidValue == value1 || invalidValue == value2)
    {
        return false;
    }

    RiaWeightedAverageCalculator<T> averageCalc;
    averageCalc.addValueAndWeight(value1, dist2);
    averageCalc.addValueAndWeight(value2, dist1);
    if (averageCalc.validAggregatedWeight())
    {
        *averagedCellValue = averageCalc.weightedAverage();
    }
    return true;
}
