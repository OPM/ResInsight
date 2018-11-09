#include "RimContourMapProjection.h"

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

#include "RivReservoirViewPartMgr.h"

#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"

#include "cafContourLines.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"

#include <QDebug>

#include <algorithm>

namespace caf
{
    template<>
    void RimContourMapProjection::ResultAggregation::setUp()
    {
        addItem(RimContourMapProjection::RESULTS_TOP_VALUE, "TOP_VALUE", "Top  Value");
        addItem(RimContourMapProjection::RESULTS_MEAN_VALUE, "MEAN_VALUE", "Arithmetic Mean");
        addItem(RimContourMapProjection::RESULTS_HARM_VALUE, "HARM_VALUE", "Harmonic Mean");
        addItem(RimContourMapProjection::RESULTS_GEOM_VALUE, "GEOM_VALUE", "Geometric Mean");
        addItem(RimContourMapProjection::RESULTS_MIN_VALUE, "MIN_VALUE", "Min Value");
        addItem(RimContourMapProjection::RESULTS_MAX_VALUE, "MAX_VALUE", "Max Value");
        addItem(RimContourMapProjection::RESULTS_SUM, "SUM", "Sum");
        addItem(RimContourMapProjection::RESULTS_OIL_COLUMN, "OIL_COLUMN", "Oil Column");
        addItem(RimContourMapProjection::RESULTS_GAS_COLUMN, "GAS_COLUMN", "Gas Column");
        addItem(RimContourMapProjection::RESULTS_HC_COLUMN,  "HC_COLUMN", "Hydrocarbon Column");

        setDefault(RimContourMapProjection::RESULTS_OIL_COLUMN);
    }
}
CAF_PDM_SOURCE_INIT(RimContourMapProjection, "RimContourMapProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::RimContourMapProjection()
{
    CAF_PDM_InitObject("RimContourMapProjection", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_relativeSampleSpacing, "SampleSpacing", 0.75, "Sample Spacing Factor", "", "", "");
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_resultAggregation, "ResultAggregation", "Result Aggregation", "", "", "");

    CAF_PDM_InitField(&m_showContourLines, "ContourLines", true, "Show Contour Lines", "", "", "");

    setName("Map Projection");
    nameField()->uiCapability()->setUiReadOnly(true);

    m_resultAccessor = new RigHugeValResultAccessor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::~RimContourMapProjection()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimContourMapProjection::expandedBoundingBox() const
{
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
    boundingBox.expand(sampleSpacing() * 0.5);
    return boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateGridMapping()
{
    calculateTotalCellVisibility();
    
    cvf::Vec3d gridExtent = expandedBoundingBox().extent();

    cvf::Vec2ui gridSize2d = surfaceGridSize();    

    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);

    m_projected3dGridIndices.resize(vertexCount());

    int nVertices = vertexCount();

#pragma omp parallel for
    for (int index = 0; index < nVertices; ++index)
    {
        cvf::Vec2ui ij = ijFromGridIndex(index);

        cvf::Vec2d globalPos = globalPos2d(ij.x(), ij.y());
        m_projected3dGridIndices[index] = visibleCellsAndWeightMatching2dPoint(globalPos);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform)
{
    CVF_ASSERT(vertices);
    vertices->resize(vertexCount());

    cvf::Vec2ui gridSize2d = surfaceGridSize();
    cvf::BoundingBox boundingBox = expandedBoundingBox();

    int nVertices = vertexCount();

#pragma omp parallel for
    for (int index = 0; index < nVertices; ++index)
    {
        cvf::Vec2ui ij = ijFromGridIndex(index);

        cvf::Vec2d globalPos = globalPos2d(ij.x(), ij.y());
        cvf::Vec3d globalVertexPos(globalPos, boundingBox.min().z() - 1.0);
        cvf::Vec3f displayVertexPos(displayCoordTransform->transformToDisplayCoord(globalVertexPos));
        (*vertices)[index] = displayVertexPos;
    }
}



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::ContourPolygons RimContourMapProjection::generateContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform)
{    
    std::vector<cvf::ref<cvf::Vec3fArray>> contourPolygons;
    if (minValue() != std::numeric_limits<double>::infinity() && maxValue() != -std::numeric_limits<double>::infinity())
    {
        cvf::BoundingBox boundingBox = expandedBoundingBox();

        std::vector<double> contourLevels;
        legendConfig()->scalarMapper()->majorTickValues(&contourLevels);
        int nContourLevels = static_cast<int>(contourLevels.size());
        if (nContourLevels > 2)
        {
            contourLevels[0] += (contourLevels[1] - contourLevels[0]) * 0.01;
            contourLevels[nContourLevels - 1] -= (contourLevels[nContourLevels - 1] - contourLevels[nContourLevels - 2]) * 0.01;
            std::vector<std::vector<cvf::Vec2d>> contourLines;
            caf::ContourLines::create(m_aggregatedResults, xPositions(), yPositions(), contourLevels, &contourLines);

            contourPolygons.reserve(contourLines.size());
            for (size_t i = 0; i < contourLines.size(); ++i)
            {
                if (!contourLines[i].empty())
                {
                    cvf::ref<cvf::Vec3fArray> contourPolygon = new cvf::Vec3fArray(contourLines[i].size());
                    for (size_t j = 0; j < contourLines[i].size(); ++j)
                    {
                        cvf::Vec3d contourPoint3d = cvf::Vec3d(contourLines[i][j], boundingBox.min().z());
                        cvf::Vec3d displayPoint3d = displayCoordTransform->transformToDisplayCoord(contourPoint3d);
                        (*contourPolygon)[j] = cvf::Vec3f(displayPoint3d);
                    }
                    contourPolygons.push_back(contourPolygon);
                }
            }
        }
    }
    return contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateResults()
{
    generateGridMapping();
    int nVertices = vertexCount();
    m_aggregatedResults = std::vector<double>(nVertices, std::numeric_limits<double>::infinity());

    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    int timeStep = view->currentTimeStep();

    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    RimEclipseCellColors* cellColors = view->cellResult();

    {
        if (!cellColors->isTernarySaturationSelected())
        {
            RigCaseCellResultsData* resultData = eclipseCase->results(RiaDefines::MATRIX_MODEL);

            if (m_resultAggregation == RESULTS_OIL_COLUMN)
            {
                resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SOIL", timeStep);
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
            }
            else if (m_resultAggregation == RESULTS_GAS_COLUMN)
            {
                resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SGAS", timeStep);
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
            }
            else if (m_resultAggregation == RESULTS_HC_COLUMN)
            {
                resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SOIL", timeStep);
                resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SGAS", timeStep);
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
                resultData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
            }
            else
            {
                m_resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(), 0, timeStep, cellColors);

                if (m_resultAccessor.isNull())
                {
                    m_resultAccessor = new RigHugeValResultAccessor;
                }
            }

            if (m_resultAggregation == RESULTS_SUM && view->cellResult()->resultVariable() == RiaDefines::riCellVolumeResultName())
            {
#pragma omp parallel for
                for (int index = 0; index < nVertices; ++index)
                {
                    cvf::Vec2ui ij = ijFromGridIndex(index);
                    m_aggregatedResults[index] = calculateVolumeSum(ij.x(), ij.y());
                }
            }
            else if (m_resultAggregation == RESULTS_SUM && view->cellResult()->resultVariable() == RiaDefines::riOilVolumeResultName())
            {
                resultData->findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SOIL", timeStep);

#pragma omp parallel for
                for (int index = 0; index < nVertices; ++index)
                {
                    cvf::Vec2ui ij = ijFromGridIndex(index);
                    m_aggregatedResults[index] = calculateSoilSum(ij.x(), ij.y());
                }
            }
            else
            {
#pragma omp parallel for
                for (int index = 0; index < nVertices; ++index)
                {
                    cvf::Vec2ui ij = ijFromGridIndex(index);
                    m_aggregatedResults[index] = calculateValue(ij.x(), ij.y());
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::maxValue() const
{
    double maxV = -std::numeric_limits<double>::infinity();

    int nVertices = vertexCount();

    for (int index = 0; index < nVertices; ++index)
    {
        if (m_aggregatedResults[index] != std::numeric_limits<double>::infinity())
        {
            maxV = std::max(maxV, m_aggregatedResults[index]);
        }
    }
    return maxV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::minValue() const
{
    double minV = std::numeric_limits<double>::infinity();

    int nVertices = vertexCount();

    for (int index = 0; index < nVertices; ++index)
    {
        if (m_aggregatedResults[index] != std::numeric_limits<double>::infinity())
        {
            minV = std::min(minV, m_aggregatedResults[index]);
        }
    }
    return minV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::meanValue() const
{
    return sumAllValues() / validVertexCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumAllValues() const
{
    double sum = 0.0;

    int nVertices = vertexCount();

    for (int index = 0; index < nVertices; ++index)
    {
        if (m_aggregatedResults[index] != std::numeric_limits<double>::infinity())
        {
            sum += m_aggregatedResults[index];
        }
    }
    return sum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sampleSpacing() const
{
    return m_relativeSampleSpacing * mainGrid()->characteristicIJCellSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sampleSpacingFactor() const
{
    return m_relativeSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::showContourLines() const
{
    return m_showContourLines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimContourMapProjection::aggregatedResults() const
{
    return m_aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isSummationResult() const
{
    return isColumnResult() || m_resultAggregation() == RESULTS_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isColumnResult() const
{
    return m_resultAggregation() == RESULTS_OIL_COLUMN ||
           m_resultAggregation() == RESULTS_GAS_COLUMN ||
           m_resultAggregation() == RESULTS_HC_COLUMN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::value(uint i, uint j) const
{
    return m_aggregatedResults.at(gridIndex(i, j));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValue(uint i, uint j) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtPos2d(i, j);
    if (!matchingCells.empty())
    {
        switch (m_resultAggregation())
        {
        case RESULTS_TOP_VALUE:
        {
            size_t cellIdx = matchingCells.front().first;
            double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
            return cellValue;
        }
        case RESULTS_MEAN_VALUE:
        {
            RiaWeightedMeanCalculator<double> calculator;
            for (auto cellIdxAndWeight : matchingCells)
            {
                size_t cellIdx = cellIdxAndWeight.first;
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
                qDebug() << calculator.weightedMean();
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
                size_t cellIdx = cellIdxAndWeight.first;
                double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                maxValue = std::max(maxValue, cellValue);
            }
            return maxValue;
        }
        case RESULTS_MIN_VALUE:
        {
            double minValue = std::numeric_limits<double>::infinity();
            for (auto cellIdxAndWeight : matchingCells)
            {
                size_t cellIdx = cellIdxAndWeight.first;
                double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                minValue = std::min(minValue, cellValue);
            }
            return minValue;
        }
        case RESULTS_SUM:
        {
            double sum = 0.0;
            for (auto cellIdxAndWeight : matchingCells)
            {
                size_t cellIdx = cellIdxAndWeight.first;
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
                size_t cellIdx = cellIdxAndWeight.first;
                double cellValue = findColumnResult(m_resultAggregation(), cellIdx);
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
double RimContourMapProjection::calculateVolumeSum(uint i, uint j) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtPos2d(i, j);
    if (!matchingCells.empty())
    {
        double sum = 0.0;
        for (auto cellIdxAndWeight : matchingCells)
        {
            // Sum only the volume intersection, not the result!
            sum += cellIdxAndWeight.second;
        }
        return sum;
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateSoilSum(uint i, uint j) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtPos2d(i, j);
    if (!matchingCells.empty())
    {
        double sum = 0.0;
        for (auto cellIdxAndWeight : matchingCells)
        {
            size_t cellIdx = cellIdxAndWeight.first;
            double cellValue = findSoilResult(cellIdx);
            sum += cellValue * cellIdxAndWeight.second;
        }
        return sum;
    }
    return std::numeric_limits<double>::infinity();

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::hasResultAt(uint i, uint j) const
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    RimEclipseCellColors* cellColors = view->cellResult();

    if (cellColors->isTernarySaturationSelected())
    {
        return false;
    }
    return !cellsAtPos2d(i, j).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::surfaceGridSize() const
{
    cvf::BoundingBox boundingBox = expandedBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();

    uint projectionSizeX = static_cast<uint>(std::ceil(gridExtent.x() / sampleSpacing())) + 1u;
    uint projectionSizeY = static_cast<uint>(std::ceil(gridExtent.y() / sampleSpacing())) + 1u;

    return cvf::Vec2ui(projectionSizeX, projectionSizeY);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint RimContourMapProjection::vertexCount() const
{
    cvf::Vec2ui gridSize2d = surfaceGridSize();
    return gridSize2d.x() * gridSize2d.y();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint RimContourMapProjection::validVertexCount() const
{
    uint validCount = 0u;
    for (uint i = 0; i < vertexCount(); ++i)
    {
        cvf::Vec2ui ij = ijFromGridIndex(i);
        if (hasResultAt(ij.x(), ij.y()))
        {
            validCount++;
        }
    }
    return validCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimContourMapProjection::legendConfig() const
{
    RimEclipseView* view = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(view);
    return view->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::calculateTotalCellVisibility()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);

    m_cellGridIdxVisibility = view->currentTotalCellVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::globalPos2d(uint i, uint j) const
{
    cvf::BoundingBox boundingBox = expandedBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();
    cvf::Vec2d origin(boundingBox.min().x(), boundingBox.min().y());
    cvf::Vec2ui gridSize2d = surfaceGridSize();

    return origin + cvf::Vec2d((i * gridExtent.x()) / (gridSize2d.x() - 1),
                               (j * gridExtent.y()) / (gridSize2d.y() - 1));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::pair<size_t, double>>& RimContourMapProjection::cellsAtPos2d(uint i, uint j) const
{
    return m_projected3dGridIndices[gridIndex(i, j)];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RimContourMapProjection::visibleCellsAndWeightMatching2dPoint(const cvf::Vec2d& globalPos2d) const
{   
    cvf::BoundingBox gridBoundingBox = expandedBoundingBox();
    cvf::Vec3d top2dElementCentroid(globalPos2d, gridBoundingBox.max().z());
    cvf::Vec3d bottom2dElementCentroid(globalPos2d, gridBoundingBox.min().z());
    cvf::Vec3d planarDiagonalVector(0.5 * sampleSpacing(), 0.5 * sampleSpacing(), 0.0);
    cvf::Vec3d topNECorner = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement(bottomSWCorner, topNECorner);

    std::vector<size_t> allCellIndices;
    mainGrid()->findIntersectingCells(bbox2dElement, &allCellIndices);

    std::vector<std::tuple<size_t, double, double>> matchingVisibleCellsWeightAndHeight;

    std::array<cvf::Vec3d, 8> hexCorners;
    double sumOverlapVolumes = 0.0;
    double maxHeight = -std::numeric_limits<double>::infinity();
    double minHeight = std::numeric_limits<double>::infinity();
    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            size_t       localCellIdx = 0u;
            RigGridBase* localGrid = mainGrid()->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIdx, &localCellIdx);

            localGrid->cellCornerVertices(localCellIdx, hexCorners.data());

            cvf::BoundingBox overlapBBox;
            std::array<cvf::Vec3d, 8> overlapCorners = RigCellGeometryTools::estimateHexOverlapWithBoundingBox(hexCorners, bbox2dElement, &overlapBBox);
            
            double overlapVolume = RigCellGeometryTools::calculateCellVolume(overlapCorners);

            if (overlapVolume > 0.0)
            {            
                double height = overlapBBox.max().z();
                matchingVisibleCellsWeightAndHeight.push_back(std::make_tuple(globalCellIdx, overlapVolume, height));
                sumOverlapVolumes += overlapVolume;
                maxHeight = std::max(maxHeight, height);
                minHeight = std::min(minHeight, overlapBBox.min().z());
            }
        }
    }

    double volAdjustmentFactor = 1.0;

    if (sumOverlapVolumes > 0.0)
    {
        cvf::Vec3d improvedBottomSwCorner = bottomSWCorner;
        improvedBottomSwCorner.z()        = minHeight;

        cvf::Vec3d improvedTopNECorner = topNECorner;
        improvedTopNECorner.z()        = maxHeight;

        cvf::BoundingBox improvedBbox2dElement = cvf::BoundingBox(improvedBottomSwCorner, improvedTopNECorner);
        cvf::Vec3d       improvedBboxExtent    = improvedBbox2dElement.extent();
        double           improvedBboxVolume    = improvedBboxExtent.x() * improvedBboxExtent.y() * improvedBboxExtent.z();

        if (sumOverlapVolumes > improvedBboxVolume)
        {
            // Total volume weights for 2d Element should never be larger than the volume of the extruded 2d element.
            volAdjustmentFactor = improvedBboxVolume / sumOverlapVolumes;
        }
    }

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;
    if (!matchingVisibleCellsWeightAndHeight.empty())
    {
        std::sort(matchingVisibleCellsWeightAndHeight.begin(),
            matchingVisibleCellsWeightAndHeight.end(),
            [](const std::tuple<size_t, double, double>& lhs, const std::tuple<size_t, double, double>& rhs) {
            return std::get<2>(lhs) > std::get<2>(rhs);
        });

        for (const auto& visWeightHeight : matchingVisibleCellsWeightAndHeight)
        {
            matchingVisibleCellsAndWeight.push_back(std::make_pair(std::get<0>(visWeightHeight), std::get<1>(visWeightHeight) * volAdjustmentFactor));
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const
{
    const RigCaseCellResultsData* resultData = eclipseCase()->results(RiaDefines::MATRIX_MODEL);
    size_t poroResultIndex = resultData->findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PORO");
    size_t ntgResultIndex = resultData->findScalarResultIndex(RiaDefines::STATIC_NATIVE, "NTG");

    if (poroResultIndex == cvf::UNDEFINED_SIZE_T || ntgResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return std::numeric_limits<double>::infinity();
    }

    const std::vector<double>& poroResults = resultData->cellScalarResults(poroResultIndex)[0];
    const std::vector<double>& ntgResults  = resultData->cellScalarResults(ntgResultIndex)[0];

    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t cellResultIdx = activeCellInfo->cellResultIndex(cellGlobalIdx);

    if (cellResultIdx >= poroResults.size() || cellResultIdx >= ntgResults.size())
    {
        return std::numeric_limits<double>::infinity();
    }

    double poro = poroResults.at(cellResultIdx);
    double ntg  = ntgResults.at(cellResultIdx);

    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    int timeStep = view->currentTimeStep();

    double resultValue = 0.0;
    if (resultAggregation == RESULTS_OIL_COLUMN || resultAggregation == RESULTS_HC_COLUMN)
    {
        size_t soilResultIndex = resultData->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL");
        const std::vector<double>& soilResults = resultData->cellScalarResults(soilResultIndex)[timeStep];
        if (cellResultIdx < soilResults.size())
        { 
            resultValue = soilResults.at(cellResultIdx);
        }
    }
    if (resultAggregation == RESULTS_GAS_COLUMN || resultAggregation == RESULTS_HC_COLUMN)
    {
        size_t sgasResultIndex = resultData->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SGAS");
        const std::vector<double>& sgasResults = resultData->cellScalarResults(sgasResultIndex)[timeStep];
        if (cellResultIdx < sgasResults.size())
        {
            resultValue += sgasResults.at(cellResultIdx);
        }
    }

    return resultValue * poro * ntg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::findSoilResult(size_t cellGlobalIdx) const
{
    const RigCaseCellResultsData* resultData = eclipseCase()->results(RiaDefines::MATRIX_MODEL);
    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t cellResultIdx = activeCellInfo->cellResultIndex(cellGlobalIdx);

    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    int timeStep = view->currentTimeStep();

    size_t soilResultIndex = resultData->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    const std::vector<double>& soilResults = resultData->cellScalarResults(soilResultIndex)[timeStep];
    if (cellResultIdx < soilResults.size())
    {
        return soilResults.at(cellResultIdx);
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEclipseResultCase* RimContourMapProjection::eclipseCase() const
{
    const RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::gridIndex(uint i, uint j) const
{
    cvf::Vec2ui gridSize2d = surfaceGridSize();

    CVF_ASSERT(i < gridSize2d.x());
    CVF_ASSERT(j < gridSize2d.y());

    return i + j * gridSize2d.x();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromGridIndex(size_t index) const
{
    CVF_TIGHT_ASSERT(index < vertexCount());

    cvf::Vec2ui gridSize2d = surfaceGridSize();

    uint quotientX  = static_cast<uint>(index) / gridSize2d.x();
    uint remainderX = static_cast<uint>(index) % gridSize2d.x();

    return cvf::Vec2ui(remainderX, quotientX);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::updateLegend()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    RimEclipseCellColors* cellColors = view->cellResult();

    if (isSummationResult() || (m_resultAggregation != RESULTS_TOP_VALUE && legendConfig()->rangeMode() != RimLegendConfig::AUTOMATIC_ALLTIMESTEPS))
    {
        double minVal = minValue();
        double maxVal = maxValue();

        legendConfig()->setAutomaticRanges(minVal, maxVal, minVal, maxVal);
    }
    else
    {
        cellColors->updateLegendData(view->currentTimeStep(), legendConfig());
    }

    if (m_resultAggregation() == RESULTS_OIL_COLUMN ||
        m_resultAggregation() == RESULTS_GAS_COLUMN ||
        m_resultAggregation() == RESULTS_HC_COLUMN)
    {
        legendConfig()->setTitle(QString("2d Projection:\n%1").arg(m_resultAggregation().uiText()));
    }
    else
    {
        legendConfig()->setTitle(QString("2d Projection:\n%1: %2").arg(m_resultAggregation().uiText()).arg(cellColors->resultVariableUiShortName()));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::ResultAggregation RimContourMapProjection::resultAggregation() const
{
    return m_resultAggregation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapProjection::resultAggregationText() const
{
    return m_resultAggregation().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::xPositions() const
{
    cvf::BoundingBox boundingBox = expandedBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();
    double origin = boundingBox.min().x();

    cvf::Vec2ui gridSize2d = surfaceGridSize();

    std::vector<double> positions;
    positions.reserve(gridSize2d.x());
    for (uint i = 0; i < gridSize2d.x(); ++i)
    {
        positions.push_back(origin + (i * gridExtent.x()) / (gridSize2d.x() - 1));
    }
    
    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::yPositions() const
{
    cvf::BoundingBox boundingBox = expandedBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();
    double origin = boundingBox.min().y();
    cvf::Vec2ui gridSize2d = surfaceGridSize();

    std::vector<double> positions;
    positions.reserve(gridSize2d.y());
    for (uint j = 0; j < gridSize2d.y(); ++j)
    {
        positions.push_back(origin + (j * gridExtent.y()) / (gridSize2d.y() - 1));
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimContourMapProjection::mainGrid() const
{
    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    return eclipseCase->eclipseCaseData()->mainGrid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_resultAggregation)
    {
        legendConfig()->disableAllTimeStepsRange(isSummationResult());
    }

    RimEclipseView* view = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(view);
    view->updateConnectedEditors();

    RimProject* proj;
    view->firstAncestorOrThisOfTypeAsserted(proj);
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{    
    if (&m_relativeSampleSpacing == field)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0.25;
            myAttr->m_maximum = 2.0;
        }        
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::initAfterRead()
{
    legendConfig()->disableAllTimeStepsRange(isSummationResult());
}
