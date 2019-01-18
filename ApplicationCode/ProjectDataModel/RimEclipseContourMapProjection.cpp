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

    double minVal = minValue(m_aggregatedResults);
    double maxVal = maxValue(m_aggregatedResults);

    std::pair<double, double> minmaxValAllTimeSteps = minmaxValuesAllTimeSteps();

    legendConfig()->setAutomaticRanges(minmaxValAllTimeSteps.first, minmaxValAllTimeSteps.second, minVal, maxVal);

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
std::vector<double> RimEclipseContourMapProjection::generateResults(int timeStep)
{
    m_weightingResult->loadResult();

    size_t nCells    = numberOfCells();

    std::vector<double> aggregatedResults = std::vector<double>(nCells, std::numeric_limits<double>::infinity());

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

            int everyNCells = temporaryResult ? 10 : 1;

#pragma omp parallel for
            for (int index = 0; index < static_cast<int>(nCells); ++index)
            {
                cvf::Vec2ui ij           = ijFromCellIndex(index);
                aggregatedResults[index] = calculateValueInMapCell(ij.x(), ij.y());
            }
        }
    }
    return aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapProjection::resultVariableChanged() const
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
void RimEclipseContourMapProjection::clearResultVariable()
{
    m_currentResultName = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateValueInMapCell(uint i, uint j) const
{
    if (isColumnResult())
    {
        const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtIJ(i, j);
        if (!matchingCells.empty())
        {
            switch (m_resultAggregation())
            {
                case RESULTS_OIL_COLUMN:
                case RESULTS_GAS_COLUMN:
                case RESULTS_HC_COLUMN:
                {
                    double sum = 0.0;
                    for (auto cellIdxAndWeight : matchingCells)
                    {
                        size_t cellIdx = cellIdxAndWeight.first;
                        double cellValue = calculateColumnResult(m_resultAggregation(), cellIdx);
                        sum += cellValue * cellIdxAndWeight.second;
                    }
                    return sum;
                }
                default:
                    CVF_TIGHT_ASSERT(false);
            }
        }
    }
    else
    {
        if (!view()->cellResult()->isFlowDiagOrInjectionFlooding() &&
            view()->cellResult()->scalarResultIndex() == cvf::UNDEFINED_SIZE_T)
        {
            return 0.0; // Special case of NONE-result. Show 0 all over to ensure we see something.
        }
        return RimContourMapProjection::calculateValueInMapCell(i, j);
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
std::vector<double> RimEclipseContourMapProjection::retrieveParameterWeights()
{
    std::vector<double> weights;
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
            weights = m_weightingResult->currentGridCellResults()->cellScalarResults(gridScalarResultIdx)[timeStep];
        }
    }
    return weights;
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
std::vector<size_t> RimEclipseContourMapProjection::findIntersectingCells(const cvf::BoundingBox& bbox) const
{
    std::vector<size_t> allCellIndices;
    m_mainGrid->findIntersectingCells(bbox, &allCellIndices);
    return allCellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateOverlapVolume(size_t globalCellIdx, const cvf::BoundingBox& bbox, size_t* cellKLayerOut) const
{
    CVF_ASSERT(cellKLayerOut != nullptr);

    std::array<cvf::Vec3d, 8> hexCorners;

    RigCell cell = m_mainGrid->globalCellArray()[globalCellIdx];

    size_t mainGridCellIdx = cell.mainGridCellIndex();
    size_t i, j, k;
    m_mainGrid->ijkFromCellIndex(mainGridCellIdx, &i, &j, &k);

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices(localCellIdx, hexCorners.data());

    cvf::BoundingBox          overlapBBox;
    std::array<cvf::Vec3d, 8> overlapCorners =
        RigCellGeometryTools::estimateHexOverlapWithBoundingBox(hexCorners, bbox, &overlapBBox);

    double overlapVolume = RigCellGeometryTools::calculateCellVolume(overlapCorners);

    *cellKLayerOut = k;

    return overlapVolume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateRayLengthInCell(size_t            globalCellIdx,
                                                                const cvf::Vec3d& highestPoint,
                                                                const cvf::Vec3d& lowestPoint,
                                                                size_t*           cellKLayerOut) const
{
    CVF_ASSERT(cellKLayerOut);

    std::array<cvf::Vec3d, 8> hexCorners;

    RigCell cell = m_mainGrid->globalCellArray()[globalCellIdx];

    size_t mainGridCellIdx = cell.mainGridCellIndex();
    size_t i, j, k;
    m_mainGrid->ijkFromCellIndex(mainGridCellIdx, &i, &j, &k);

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices(localCellIdx, hexCorners.data());
    std::vector<HexIntersectionInfo> intersections;

    if (RigHexIntersectionTools::lineHexCellIntersection(highestPoint, lowestPoint, hexCorners.data(), 0, &intersections))
    {
        double lengthInCell = (intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint).length();
        *cellKLayerOut = k;
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::getParameterWeightForCell(size_t globalCellIdx, const std::vector<double>& cellWeights) const
{
    if (cellWeights.empty()) return 1.0;

    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t                   cellResultIdx  = activeCellInfo->cellResultIndex(globalCellIdx);
    double                   result         = std::max(cellWeights[cellResultIdx], 0.0);
    if (result < 1.0e-6)
    {
        result = 0.0;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::gridCellValue(size_t globalCellIdx) const
{
    return m_resultAccessor->cellScalarGlobIdx(globalCellIdx);
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue)
{
    RimContourMapProjection::fieldChangedByUi(changedField, oldValue, newValue);
    if (changedField == &m_weightByParameter || changedField == &m_weightingResult)
    {
        clearGridMapping();
    }
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
