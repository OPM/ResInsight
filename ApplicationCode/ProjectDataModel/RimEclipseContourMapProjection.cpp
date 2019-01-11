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

#include "RimEclipseContourMapProjection.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCellRangeFilterCollection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTextAnnotation.h"

#include "cafContourLines.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfGeometryTools.h"
#include "cvfGeometryUtils.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"

#include <algorithm>
#include <omp.h>

CAF_PDM_SOURCE_INIT(RimEclipseContourMapProjection, "RimEclipseContourMapProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::RimEclipseContourMapProjection()
    : RimContourMapProjection()
{
    CAF_PDM_InitObject("RimEclipseContourMapProjection", ":/2DMapProjection16x16.png", "", "");

    CAF_PDM_InitField(&m_weightByParameter, "WeightByParameter", false, "Weight by Result Parameter", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_weightingResult, "WeightingResult", "", "", "", "");
    m_weightingResult.uiCapability()->setUiHidden(true);
    m_weightingResult.uiCapability()->setUiTreeChildrenHidden(true);
    m_weightingResult = new RimEclipseResultDefinition;
    m_weightingResult->findField("MResultType")->uiCapability()->setUiName("Result Type");

    setName("Map Projection");
    nameField()->uiCapability()->setUiReadOnly(true);

    m_resultAccessor = new RigHugeValResultAccessor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::~RimEclipseContourMapProjection()
{

}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::resultDescriptionText() const
{
    QString resultText = resultAggregationText();
    if (!isColumnResult())
    {
        resultText += QString(", %1").arg(view()->cellResult()->resultVariable());
    }

    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::weightingParameter() const
{
    QString parameter = "None";
    if (m_weightByParameter() && !m_weightingResult->isTernarySaturationSelected())
    {
        parameter = m_weightingResult->resultVariableUiShortName();
    }
    return parameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEclipseContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateLegend()
{    
    RimEclipseCellColors* cellColors = view()->cellResult();

    if (use3dGridLegendRange())
    {
        cellColors->updateLegendData(view()->currentTimeStep(), legendConfig());
    }
    else
    {
        CVF_ASSERT(use2dMapLegendRange());

        double minVal = minValue();
        double maxVal = maxValue();

        legendConfig()->setAutomaticRanges(minVal, maxVal, minVal, maxVal);
    }

    if (m_resultAggregation() == RESULTS_OIL_COLUMN ||
        m_resultAggregation() == RESULTS_GAS_COLUMN ||
        m_resultAggregation() == RESULTS_HC_COLUMN)
    {
        legendConfig()->setTitle(QString("Map Projection\n%1").arg(m_resultAggregation().uiText()));
    }
    else
    {
        QString projectionLegendText = QString("Map Projection\n%1").arg(m_resultAggregation().uiText());
        if (weightingParameter() != "None")
        {
            projectionLegendText += QString("(W: %1)").arg(weightingParameter());
        }
        projectionLegendText += QString("\nResult: %1").arg(cellColors->resultVariableUiShortName());

        legendConfig()->setTitle(projectionLegendText);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updatedWeightingResult()
{
    this->clearGridMapping();
    this->updateConnectedEditors();
    this->generateResultsIfNecessary(view()->currentTimeStep());
    this->updateLegend();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimContourMapProjection::defineUiOrdering(uiConfigName, uiOrdering);

    caf::PdmUiGroup* weightingGroup = uiOrdering.addNewGroup("Mean Weighting Options");
    weightingGroup->add(&m_weightByParameter);
    weightingGroup->setCollapsedByDefault(true);

    m_weightByParameter.uiCapability()->setUiReadOnly(!isMeanResult());
    if (!isMeanResult())
    {
        m_weightByParameter = false;
    }

    if (m_weightByParameter())
    {
        m_weightingResult->uiOrdering(uiConfigName, *weightingGroup);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::initAfterRead()
{
    RimContourMapProjection::initAfterRead();
    if (eclipseCase())
    {
        m_weightingResult->setEclipseCase(eclipseCase());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::generateGridMapping()
{
    clearResults();

    m_cellGridIdxVisibility = view()->currentTotalCellVisibility();

    int nCells = numberOfCells();
    m_projected3dGridIndices.resize(nCells);

    const std::vector<double>* weightingResultValues = nullptr;
    if (m_weightByParameter())
    {
        size_t gridScalarResultIdx = m_weightingResult->scalarResultIndex();
        if (gridScalarResultIdx != cvf::UNDEFINED_SIZE_T)
        {
            m_weightingResult->loadResult();
            int timeStep = 0;
            if (m_weightingResult->hasDynamicResult())
            {
                timeStep = view()->currentTimeStep();
            }
            weightingResultValues =
                &(m_weightingResult->currentGridCellResults()->cellScalarResults(gridScalarResultIdx)[timeStep]);
        }
    }

    if (isStraightSummationResult())
    {
#pragma omp parallel for
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos = cellCenterPosition(ij.x(), ij.y()) + origin2d();
            m_projected3dGridIndices[index] = visibleCellsAndLengthInCellFrom2dPoint(globalPos, weightingResultValues);
        }
    }
    else
    {
#pragma omp parallel for
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos = cellCenterPosition(ij.x(), ij.y()) + origin2d();
            m_projected3dGridIndices[index] = visibleCellsAndOverlapVolumeFrom2dPoint(globalPos, weightingResultValues);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::generateResults(int timeStep)
{
    clearGeometry();

    m_weightingResult->loadResult();

    size_t nCells    = numberOfCells();
    size_t nVertices = numberOfVertices();

    m_aggregatedResults              = std::vector<double>(nCells, std::numeric_limits<double>::infinity());
    m_aggregatedVertexResults        = std::vector<double>(nVertices, std::numeric_limits<double>::infinity());
    RimEclipseCellColors* cellColors = view()->cellResult();

    RimEclipseResultCase* eclipseCase = this->eclipseCase();
    {
        if (!cellColors->isTernarySaturationSelected())
        {
            RigCaseCellResultsData* resultData = eclipseCase->results(RiaDefines::MATRIX_MODEL);

            if (isColumnResult())
            {
                m_currentResultName = "";
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
                if (m_resultAggregation == RESULTS_OIL_COLUMN || m_resultAggregation == RESULTS_HC_COLUMN)
                {
                    resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SOIL", timeStep);
                }
                if (m_resultAggregation == RESULTS_GAS_COLUMN || m_resultAggregation == RESULTS_HC_COLUMN)
                {
                    resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SGAS", timeStep);
                }
            }
            else
            {
                m_currentResultName = cellColors->resultVariable();
                m_resultAccessor =
                    RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(), 0, timeStep, cellColors);

                if (m_resultAccessor.isNull())
                {
                    m_resultAccessor = new RigHugeValResultAccessor;
                }
            }

#pragma omp parallel for
            for (int index = 0; index < static_cast<int>(nCells); ++index)
            {
                cvf::Vec2ui ij             = ijFromCellIndex(index);
                m_aggregatedResults[index] = calculateValueInCell(ij.x(), ij.y());
            }

#pragma omp parallel for
            for (int index = 0; index < static_cast<int>(nVertices); ++index)
            {
                cvf::Vec2ui ij                   = ijFromVertexIndex(index);
                m_aggregatedVertexResults[index] = calculateValueAtVertex(ij.x(), ij.y());
            }
        }
    }
    m_currentResultTimestep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapProjection::gridMappingImplNeedsUpdating() const
{
    if (m_cellGridIdxVisibility.isNull())
    {
        return true;
    }
    cvf::ref<cvf::UByteArray> currentVisibility = view()->currentTotalCellVisibility();
    CVF_ASSERT(currentVisibility->size() == m_cellGridIdxVisibility->size());
    for (size_t i = 0; i < currentVisibility->size(); ++i)
    {
        if ((*currentVisibility)[i] != (*m_cellGridIdxVisibility)[i]) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapProjection::resultsImplNeedsUpdating() const
{   
    if (!m_currentResultName.isEmpty())
    {
        RimEclipseCellColors* cellColors = view()->cellResult();
        if (cellColors->resultVariable() != m_currentResultName)
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::clearImplSpecificResultData()
{
    m_currentResultName = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateValueInCell(uint i, uint j) const
{
    if (!isColumnResult())        
    {
        if (!view()->cellResult()->isFlowDiagOrInjectionFlooding() &&
            view()->cellResult()->scalarResultIndex() == cvf::UNDEFINED_SIZE_T)
        {
            return 0.0; // Special case of NONE-result. Show 0 all over to ensure we see something.
        }
    }
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtIJ(i, j);
    if (!matchingCells.empty())
    {
        switch (m_resultAggregation())
        {
            case RESULTS_TOP_VALUE:
            {
                size_t cellIdx   = matchingCells.front().first;
                double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                return cellValue;
            }
            case RESULTS_MEAN_VALUE:
            {
                RiaWeightedMeanCalculator<double> calculator;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                }
                if (calculator.validAggregatedWeight())
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_GEOM_VALUE:
            {
                RiaWeightedGeometricMeanCalculator calculator;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    if (cellValue < 1.0e-8)
                    {
                        return 0.0;
                    }
                    calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                }
                if (calculator.validAggregatedWeight())
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_HARM_VALUE:
            {
                RiaWeightedHarmonicMeanCalculator calculator;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    if (std::fabs(cellValue) < 1.0e-8)
                    {
                        return 0.0;
                    }
                    calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                }
                if (calculator.validAggregatedWeight())
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_MAX_VALUE:
            {
                double maxValue = -std::numeric_limits<double>::infinity();
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    maxValue         = std::max(maxValue, cellValue);
                }
                return maxValue;
            }
            case RESULTS_MIN_VALUE:
            {
                double minValue = std::numeric_limits<double>::infinity();
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    minValue         = std::min(minValue, cellValue);
                }
                return minValue;
            }
            case RESULTS_VOLUME_SUM:
            case RESULTS_SUM:
            {
                double sum = 0.0;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                    sum += cellValue * cellIdxAndWeight.second;
                }
                return sum;
            }
            case RESULTS_OIL_COLUMN:
            case RESULTS_GAS_COLUMN:
            case RESULTS_HC_COLUMN:
            {
                double sum = 0.0;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = calculateColumnResult(m_resultAggregation(), cellIdx);
                    sum += cellValue * cellIdxAndWeight.second;
                }
                return sum;
            }
            default:
                CVF_TIGHT_ASSERT(false);
        }
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const
{
    const RigCaseCellResultsData* resultData      = eclipseCase()->results(RiaDefines::MATRIX_MODEL);
    size_t                        poroResultIndex = resultData->findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PORO");
    size_t                        ntgResultIndex  = resultData->findScalarResultIndex(RiaDefines::STATIC_NATIVE, "NTG");
    size_t                        dzResultIndex   = resultData->findScalarResultIndex(RiaDefines::STATIC_NATIVE, "DZ");

    if (poroResultIndex == cvf::UNDEFINED_SIZE_T || ntgResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return std::numeric_limits<double>::infinity();
    }

    const std::vector<double>& poroResults = resultData->cellScalarResults(poroResultIndex)[0];
    const std::vector<double>& ntgResults  = resultData->cellScalarResults(ntgResultIndex)[0];
    const std::vector<double>& dzResults   = resultData->cellScalarResults(dzResultIndex)[0];

    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t                   cellResultIdx  = activeCellInfo->cellResultIndex(cellGlobalIdx);

    if (cellResultIdx >= poroResults.size() || cellResultIdx >= ntgResults.size())
    {
        return std::numeric_limits<double>::infinity();
    }

    double poro = poroResults.at(cellResultIdx);
    double ntg  = ntgResults.at(cellResultIdx);
    double dz   = dzResults.at(cellResultIdx);

    int timeStep = view()->currentTimeStep();

    double resultValue = 0.0;
    if (resultAggregation == RESULTS_OIL_COLUMN || resultAggregation == RESULTS_HC_COLUMN)
    {
        size_t                     soilResultIndex = resultData->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL");
        const std::vector<double>& soilResults     = resultData->cellScalarResults(soilResultIndex)[timeStep];
        if (cellResultIdx < soilResults.size())
        {
            resultValue = soilResults.at(cellResultIdx);
        }
    }
    if (resultAggregation == RESULTS_GAS_COLUMN || resultAggregation == RESULTS_HC_COLUMN)
    {
        size_t                     sgasResultIndex = resultData->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SGAS");
        const std::vector<double>& sgasResults     = resultData->cellScalarResults(sgasResultIndex)[timeStep];
        if (cellResultIdx < sgasResults.size())
        {
            resultValue += sgasResults.at(cellResultIdx);
        }
    }

    return resultValue * poro * ntg * dz;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>>
    RimEclipseContourMapProjection::visibleCellsAndOverlapVolumeFrom2dPoint(const cvf::Vec2d&          globalPos2d,
                                                                     const std::vector<double>* weightingResultValues) const
{
    cvf::Vec3d top2dElementCentroid(globalPos2d, m_expandedBoundingBox.max().z());
    cvf::Vec3d bottom2dElementCentroid(globalPos2d, m_expandedBoundingBox.min().z());
    cvf::Vec3d planarDiagonalVector(0.5 * m_sampleSpacing, 0.5 * m_sampleSpacing, 0.0);
    cvf::Vec3d topNECorner    = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement(bottomSWCorner, topNECorner);

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    // Bounding box has been expanded, so 2d element may be outside actual 3d grid
    if (!bbox2dElement.intersects(m_gridBoundingBox))
    {
        return matchingVisibleCellsAndWeight;
    }

    std::vector<size_t> allCellIndices;
    m_mainGrid->findIntersectingCells(bbox2dElement, &allCellIndices);

    typedef std::map<size_t, std::vector<std::pair<size_t, double>>> KLayerCellWeightMap;
    KLayerCellWeightMap                                              matchingVisibleCellsWeightPerKLayer;

    std::array<cvf::Vec3d, 8> hexCorners;
    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            RigCell cell = m_mainGrid->globalCellArray()[globalCellIdx];

            size_t mainGridCellIdx = cell.mainGridCellIndex();
            size_t i, j, k;
            m_mainGrid->ijkFromCellIndex(mainGridCellIdx, &i, &j, &k);

            size_t       localCellIdx = cell.gridLocalCellIndex();
            RigGridBase* localGrid    = cell.hostGrid();

            localGrid->cellCornerVertices(localCellIdx, hexCorners.data());

            cvf::BoundingBox          overlapBBox;
            std::array<cvf::Vec3d, 8> overlapCorners =
                RigCellGeometryTools::estimateHexOverlapWithBoundingBox(hexCorners, bbox2dElement, &overlapBBox);

            double overlapVolume = RigCellGeometryTools::calculateCellVolume(overlapCorners);

            if (overlapVolume > 0.0)
            {
                double weight = overlapVolume;
                if (weightingResultValues)
                {
                    const RigActiveCellInfo* activeCellInfo =
                        eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
                    size_t cellResultIdx = activeCellInfo->cellResultIndex(globalCellIdx);
                    double result        = std::max((*weightingResultValues)[cellResultIdx], 0.0);
                    if (result < 1.0e-6)
                    {
                        result = 0.0;
                    }
                    weight *= result;
                }
                if (weight > 0.0)
                {
                    matchingVisibleCellsWeightPerKLayer[k].push_back(std::make_pair(globalCellIdx, weight));
                }
            }
        }
    }

    for (auto kLayerCellWeight : matchingVisibleCellsWeightPerKLayer)
    {
        for (auto cellWeight : kLayerCellWeight.second)
        {
            matchingVisibleCellsAndWeight.push_back(std::make_pair(cellWeight.first, cellWeight.second));
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RimEclipseContourMapProjection::visibleCellsAndLengthInCellFrom2dPoint(
    const cvf::Vec2d&          globalPos2d,
    const std::vector<double>* weightingResultValues /*= nullptr*/) const
{
    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    cvf::Vec3d highestPoint(globalPos2d, m_expandedBoundingBox.max().z());
    cvf::Vec3d lowestPoint(globalPos2d, m_expandedBoundingBox.min().z());

    // Bounding box has been expanded, so ray may be outside actual grid
    if (!m_gridBoundingBox.contains(highestPoint))
    {
        return matchingVisibleCellsAndWeight;
    }

    cvf::BoundingBox rayBBox;
    rayBBox.add(highestPoint);
    rayBBox.add(lowestPoint);

    std::vector<size_t> allCellIndices;
    m_mainGrid->findIntersectingCells(rayBBox, &allCellIndices);

    std::map<size_t, std::vector<std::pair<size_t, double>>> matchingVisibleCellsAndWeightPerKLayer;

    cvf::Vec3d hexCorners[8];
    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            RigCell cell = m_mainGrid->globalCellArray()[globalCellIdx];

            size_t mainGridCellIdx = cell.mainGridCellIndex();
            size_t i, j, k;
            m_mainGrid->ijkFromCellIndex(mainGridCellIdx, &i, &j, &k);

            size_t       localCellIdx = cell.gridLocalCellIndex();
            RigGridBase* localGrid    = cell.hostGrid();

            localGrid->cellCornerVertices(localCellIdx, hexCorners);
            std::vector<HexIntersectionInfo> intersections;

            if (RigHexIntersectionTools::lineHexCellIntersection(highestPoint, lowestPoint, hexCorners, 0, &intersections))
            {
                double lengthInCell =
                    (intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint).length();
                matchingVisibleCellsAndWeightPerKLayer[k].push_back(std::make_pair(globalCellIdx, lengthInCell));
            }
        }
    }

    for (auto kLayerCellWeight : matchingVisibleCellsAndWeightPerKLayer)
    {
        // Make sure the sum of all weights in the same K-layer is 1.
        double weightSumThisKLayer = 0.0;
        for (auto cellWeight : kLayerCellWeight.second)
        {
            weightSumThisKLayer += cellWeight.second;
        }

        for (auto cellWeight : kLayerCellWeight.second)
        {
            matchingVisibleCellsAndWeight.push_back(std::make_pair(cellWeight.first, cellWeight.second / weightSumThisKLayer));
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateGridInformation()
{
    m_mainGrid        = eclipseCase()->eclipseCaseData()->mainGrid();
    m_sampleSpacing   = m_relativeSampleSpacing * m_mainGrid->characteristicIJCellSize();
    m_gridBoundingBox = eclipseCase()->activeCellsBoundingBox();
    cvf::Vec3d minExpandedPoint = m_gridBoundingBox.min() - cvf::Vec3d(gridEdgeOffset(), gridEdgeOffset(), 0.0);
    cvf::Vec3d maxExpandedPoint = m_gridBoundingBox.max() + cvf::Vec3d(gridEdgeOffset(), gridEdgeOffset(), 0.0);
    m_expandedBoundingBox = cvf::BoundingBox(minExpandedPoint, maxExpandedPoint);

    m_mapSize         = calculateMapSize();

    // Re-jig max point to be an exact multiple of cell size
    cvf::Vec3d minPoint = m_expandedBoundingBox.min();
    cvf::Vec3d maxPoint = m_expandedBoundingBox.max();
    maxPoint.x() = minPoint.x() + m_mapSize.x() * m_sampleSpacing;
    maxPoint.y() = minPoint.y() + m_mapSize.y() * m_sampleSpacing;
    m_expandedBoundingBox = cvf::BoundingBox(minPoint, maxPoint);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimEclipseContourMapProjection::eclipseCase() const
{
    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfType(eclipseCase);
    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimEclipseContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RimEclipseContourMapProjection::view() const
{
    RimEclipseContourMapView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    return view;
}
