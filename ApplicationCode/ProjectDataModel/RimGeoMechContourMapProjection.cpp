/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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
#include "RimGeoMechContourMapProjection.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCellGeometryTools.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigHexIntersectionTools.h"

#include "RimCellRangeFilterCollection.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"

#include "RivFemElmVisibilityCalculator.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfGeometryTools.h"
#include "cvfGeometryUtils.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfVector3.h"

#include <algorithm>
#include <array>
#include <omp.h>
#include <QDebug>

CAF_PDM_SOURCE_INIT(RimGeoMechContourMapProjection, "RimGeoMechContourMapProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::RimGeoMechContourMapProjection()
{
    CAF_PDM_InitObject("RimContourMapProjection", ":/2DMapProjection16x16.png", "", "");

    setName("Map Projection");
    nameField()->uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::~RimGeoMechContourMapProjection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechContourMapProjection::resultDescriptionText() const
{
    QString resultText = QString("%1, %2").arg(resultAggregationText()).arg(view()->cellResult()->resultFieldUiName());
    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimGeoMechContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateLegend()
{
    RimGeoMechCellColors* cellColors = view()->cellResult();

    if (use3dGridLegendRange())
    {
        view()->updateLegendTextAndRanges(cellColors->legendConfig(), view()->currentTimeStep());
    }
    else
    {
        CVF_ASSERT(use2dMapLegendRange());

        double minVal = minValue();
        double maxVal = maxValue();
        legendConfig()->setAutomaticRanges(minVal, maxVal, minVal, maxVal);
    }

    QString projectionLegendText = QString("Map Projection\n%1").arg(m_resultAggregation().uiText());
    projectionLegendText += QString("\nResult: %1").arg(cellColors->resultFieldUiName());

    legendConfig()->setTitle(projectionLegendText);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimGeoMechContourMapProjection::getCellVisibility() const
{
    cvf::ref<cvf::UByteArray> cellGridIdxVisibility = new cvf::UByteArray(m_femPart->elementCount());
    RivFemElmVisibilityCalculator::computeAllVisible(cellGridIdxVisibility.p(), m_femPart.p());

    cvf::CellRangeFilter cellRangeFilter;
    view()->rangeFilterCollection()->compoundCellRangeFilter(&cellRangeFilter, 0);
    RivFemElmVisibilityCalculator::computeRangeVisibility(cellGridIdxVisibility.p(), m_femPart.p(), cellRangeFilter);
    RivFemElmVisibilityCalculator::computePropertyVisibility(cellGridIdxVisibility.p(), m_femPart.p(), view()->currentTimeStep(), cellGridIdxVisibility.p(), view()->geoMechPropertyFilterCollection());

    ensureOnlyValidPorBarVisible(cellGridIdxVisibility.p(), view()->currentTimeStep());
    return cellGridIdxVisibility;
}



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::ensureOnlyValidPorBarVisible(cvf::UByteArray* visibility, int timeStep) const
{
    RigFemResultAddress porBarAddr(RigFemResultPosEnum::RIG_ELEMENT_NODAL, "POR-Bar", view()->cellResult()->resultComponentName().toStdString());
    RigGeoMechCaseData*          caseData         = geoMechCase()->geoMechData();
    RigFemPartResultsCollection* resultCollection = caseData->femPartResults();

    const std::vector<float>& resultValues = resultCollection->resultValues(porBarAddr, 0, timeStep);
    for (int i = 0; i < static_cast<int>(visibility->size()); ++i)
    {
        size_t resValueIdx = m_femPart->elementNodeResultIdx((int) i, 0);
        double scalarValue = resultValues[resValueIdx];
        (*visibility)[i] &= scalarValue != std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateGridInformation()
{
    RimGeoMechCase* geoMechCase = this->geoMechCase();
    m_femPart                   = geoMechCase->geoMechData()->femParts()->part(0);    
    m_femPartGrid               = m_femPart->getOrCreateStructGrid();
    m_sampleSpacing             = m_relativeSampleSpacing * geoMechCase->characteristicCellSize();
    m_gridBoundingBox           = geoMechCase->activeCellsBoundingBox();
    m_femPart->ensureIntersectionSearchTreeIsBuilt();

    cvf::Vec3d minExpandedPoint = m_gridBoundingBox.min() - cvf::Vec3d(gridEdgeOffset(), gridEdgeOffset(), 0.0);
    cvf::Vec3d maxExpandedPoint = m_gridBoundingBox.max() + cvf::Vec3d(gridEdgeOffset(), gridEdgeOffset(), 0.0);
    m_expandedBoundingBox       = cvf::BoundingBox(minExpandedPoint, maxExpandedPoint);

    m_mapSize = calculateMapSize();

    // Re-jig max point to be an exact multiple of cell size
    cvf::Vec3d minPoint   = m_expandedBoundingBox.min();
    cvf::Vec3d maxPoint   = m_expandedBoundingBox.max();
    maxPoint.x()          = minPoint.x() + m_mapSize.x() * m_sampleSpacing;
    maxPoint.y()          = minPoint.y() + m_mapSize.y() * m_sampleSpacing;
    m_expandedBoundingBox = cvf::BoundingBox(minPoint, maxPoint);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::retrieveParameterWeights()
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::generateResults(int timeStep)
{
    clearGeometry();

    RigGeoMechCaseData*          caseData         = geoMechCase()->geoMechData();
    RigFemPartResultsCollection* resultCollection = caseData->femPartResults();
    RimGeoMechCellColors*        cellColors       = view()->cellResult();
    RigFemResultAddress          resAddr          = cellColors->resultAddress();
    if (!resAddr.isValid())
        return;

    if (resAddr.fieldName == "PP")
    {
        resAddr.fieldName     = "POR-Bar"; // More likely to be in memory than POR
    }
    if (resAddr.fieldName == "POR-Bar") resAddr.resultPosType = RIG_ELEMENT_NODAL;

    m_resultValues                                = resultCollection->resultValues(resAddr, 0, timeStep);

    size_t nCells = numberOfCells();
    m_aggregatedResults              = std::vector<double>(nCells, std::numeric_limits<double>::infinity());

#pragma omp parallel for
    for (int index = 0; index < static_cast<int>(nCells); ++index)
    {
        cvf::Vec2ui ij             = ijFromCellIndex(index);
        m_aggregatedResults[index] = calculateValueInMapCell(ij.x(), ij.y());
    }

    m_currentResultAddr = resAddr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapProjection::resultVariableChanged() const
{
    RimGeoMechCellColors* cellColors = view()->cellResult();
    RigFemResultAddress   resAddr    = cellColors->resultAddress();

    if (resAddr.fieldName == "PP")
    {
        resAddr.fieldName = "POR-Bar"; // More likely to be in memory than POR
    }
    if (resAddr.fieldName == "POR-Bar") resAddr.resultPosType = RIG_ELEMENT_NODAL;

    return !(m_currentResultAddr == resAddr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::clearResultVariable()
{
    m_currentResultAddr = RigFemResultAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimGeoMechContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimGeoMechContourMapProjection::findIntersectingCells(const cvf::BoundingBox& bbox) const
{
    std::vector<size_t> allCellIndices;
    m_femPart->findIntersectingCellsWithExistingSearchTree(bbox, &allCellIndices);
    return allCellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::calculateOverlapVolume(size_t                  globalCellIdx,
                                                              const cvf::BoundingBox& bbox,
                                                              size_t*                 cellKLayerOut) const
{
    CVF_ASSERT(cellKLayerOut != nullptr);

    std::array<cvf::Vec3d, 8> hexCorners;

    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities(globalCellIdx);

    hexCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
    hexCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
    hexCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
    hexCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
    hexCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
    hexCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
    hexCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
    hexCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

    cvf::BoundingBox          overlapBBox;
    std::array<cvf::Vec3d, 8> overlapCorners =
        RigCellGeometryTools::estimateHexOverlapWithBoundingBox(hexCorners, bbox, &overlapBBox);

    double overlapVolume = RigCellGeometryTools::calculateCellVolume(overlapCorners);
    if (overlapVolume > 0.0)
    {
        size_t i, j, k;
        m_femPartGrid->ijkFromCellIndex(globalCellIdx, &i, &j, &k);
        *cellKLayerOut = k;

        return overlapVolume;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::calculateRayLengthInCell(size_t            globalCellIdx,
                                                                const cvf::Vec3d& highestPoint,
                                                                const cvf::Vec3d& lowestPoint,
                                                                size_t*           cellKLayerOut) const
{
    CVF_ASSERT(cellKLayerOut != nullptr);

    std::array<cvf::Vec3d, 8> hexCorners;

    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities(globalCellIdx);

    hexCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
    hexCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
    hexCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
    hexCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
    hexCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
    hexCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
    hexCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
    hexCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

    std::vector<HexIntersectionInfo> intersections;

    if (RigHexIntersectionTools::lineHexCellIntersection(highestPoint, lowestPoint, hexCorners.data(), 0, &intersections))
    {
        double lengthInCell = (intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint).length();

        size_t i, j, k;
        m_femPartGrid->ijkFromCellIndex(globalCellIdx, &i, &j, &k);
        *cellKLayerOut = k;
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::getParameterWeightForCell(size_t                     globalCellIdx,
                                                                 const std::vector<double>& parameterWeights) const
{
    if (parameterWeights.empty()) return 1.0;

    return parameterWeights[globalCellIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::gridCellValue(size_t globalCellIdx) const
{
    RimGeoMechCellColors* cellColors    = view()->cellResult();
    RigFemResultAddress   resAddr       = cellColors->resultAddress();

    if (resAddr.fieldName == "PP")
    {
        resAddr.fieldName = "POR-Bar"; // More likely to be in memory than POR
    }
    if (resAddr.fieldName == "POR-Bar") resAddr.resultPosType = RIG_ELEMENT_NODAL;

    RigFemResultPosEnum   resultPosType = resAddr.resultPosType;

    RigElementType elmType = m_femPart->elementType(globalCellIdx);

    if (!(elmType == HEX8 || elmType == HEX8P)) return 0.0;

    if (resultPosType == RIG_FORMATION_NAMES)
    {
        resultPosType = RIG_ELEMENT_NODAL; // formation indices are stored per element node result.
    }

    if (resultPosType == RIG_ELEMENT)
    {
        return m_resultValues[globalCellIdx];
    }
    else if (resultPosType == RIG_ELEMENT_NODAL)
    {
        RiaWeightedMeanCalculator<float> cellAverage;
        for (int i = 0; i < 8; ++i)
        {
            size_t gridResultValueIdx = m_femPart->resultValueIdxFromResultPosType(resultPosType, static_cast<int>(globalCellIdx), i);
            cellAverage.addValueAndWeight(m_resultValues[gridResultValueIdx], 1.0);
        }
        
        return cellAverage.weightedMean();
    }
    else
    {
        RiaWeightedMeanCalculator<float> cellAverage;
        const int* elmNodeIndices = m_femPart->connectivities(globalCellIdx);
        for (int i = 0; i < 8; ++i)
        {
            cellAverage.addValueAndWeight(m_resultValues[elmNodeIndices[i]], 1.0);
        }
        return cellAverage.weightedMean();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechContourMapProjection::geoMechCase() const
{
    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType(geoMechCase);
    return geoMechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RimGeoMechContourMapProjection::view() const
{
    RimGeoMechContourMapView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue)
{
    RimContourMapProjection::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGeoMechContourMapProjection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_resultAggregation)
    {
        std::vector<ResultAggregationEnum> validOptions = {RESULTS_TOP_VALUE,
                                                           RESULTS_MEAN_VALUE,
                                                           RESULTS_GEOM_VALUE,
                                                           RESULTS_HARM_VALUE,
                                                           RESULTS_MIN_VALUE,
                                                           RESULTS_MAX_VALUE,
                                                           RESULTS_SUM};

        for (ResultAggregationEnum option : validOptions)
        {
            options.push_back(caf::PdmOptionItemInfo(ResultAggregation::uiText(option), option));
        }
    }
    return options;
}
