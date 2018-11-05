#include "Rim2dGridProjection.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
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
    void Rim2dGridProjection::ResultAggregation::setUp()
    {
        addItem(Rim2dGridProjection::RESULTS_TOP_VALUE, "TOP_VALUE", "Top  Value");
        addItem(Rim2dGridProjection::RESULTS_MEAN_VALUE, "MEAN_VALUE", "Arithmetic Mean");
        addItem(Rim2dGridProjection::RESULTS_HARM_VALUE, "HARM_VALUE", "Harmonic Mean");
        addItem(Rim2dGridProjection::RESULTS_GEOM_VALUE, "GEOM_VALUE", "Geometric Mean");
        addItem(Rim2dGridProjection::RESULTS_MIN_VALUE, "MIN_VALUE", "Min Value");
        addItem(Rim2dGridProjection::RESULTS_MAX_VALUE, "MAX_VALUE", "Max Value");
        addItem(Rim2dGridProjection::RESULTS_SUM, "SUM", "Sum");
        addItem(Rim2dGridProjection::RESULTS_OIL_COLUMN, "OIL_COLUMN", "Oil Column");
        addItem(Rim2dGridProjection::RESULTS_GAS_COLUMN, "GAS_COLUMN", "Gas Column");
        addItem(Rim2dGridProjection::RESULTS_HC_COLUMN,  "HC_COLUMN", "Hydrocarbon Column");

        setDefault(Rim2dGridProjection::RESULTS_TOP_VALUE);
    }
}
CAF_PDM_SOURCE_INIT(Rim2dGridProjection, "Rim2dGridProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dGridProjection::Rim2dGridProjection()
{
    CAF_PDM_InitObject("Rim2dGridProjection", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_sampleSpacing, "SampleSpacing", -1.0, "Sample Spacing", "", "", "");
    m_sampleSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_resultAggregation, "ResultAggregation", "Result Aggregation", "", "", "");

    CAF_PDM_InitField(&m_showContourLines, "ContourLines", true, "Show Contour Lines", "", "", "");

    setName("2d Grid Projection");
    nameField()->uiCapability()->setUiReadOnly(true);

    m_resultAccessor = new RigHugeValResultAccessor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dGridProjection::~Rim2dGridProjection()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox Rim2dGridProjection::expandedBoundingBox() const
{
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
    boundingBox.expand(m_sampleSpacing * 0.5);
    return boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::generateGridMapping()
{
    updateDefaultSampleSpacingFromGrid();

    calculateCellRangeVisibility();
    calculatePropertyFilterVisibility();
    
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
void Rim2dGridProjection::generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform)
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
Rim2dGridProjection::ContourPolygons Rim2dGridProjection::generateContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform)
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
void Rim2dGridProjection::generateResults()
{
    generateGridMapping();
    int nVertices = vertexCount();
    m_aggregatedResults.resize(nVertices, std::numeric_limits<double>::infinity());

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

#pragma omp parallel for
            for (int index = 0; index < nVertices; ++index)
            {
                cvf::Vec2ui ij = ijFromGridIndex(index);
                m_aggregatedResults[index] = calculateValue(ij.x(), ij.y());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::maxValue() const
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
double Rim2dGridProjection::minValue() const
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
double Rim2dGridProjection::sampleSpacing() const
{
    return m_sampleSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim2dGridProjection::showContourLines() const
{
    return m_showContourLines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::updateDefaultSampleSpacingFromGrid()
{
    if (m_sampleSpacing < 0.0)
    {
        m_sampleSpacing = mainGrid()->characteristicIJCellSize() * 0.5;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& Rim2dGridProjection::aggregatedResults() const
{
    return m_aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim2dGridProjection::isSummationResult() const
{
    return m_resultAggregation() == RESULTS_OIL_COLUMN ||
           m_resultAggregation() == RESULTS_GAS_COLUMN ||
           m_resultAggregation() == RESULTS_HC_COLUMN ||
           m_resultAggregation() == RESULTS_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::value(uint i, uint j) const
{
    return m_aggregatedResults.at(gridIndex(i, j));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::calculateValue(uint i, uint j) const
{
    const std::vector<std::pair<size_t, float>>& matchingCells = cellsAtPos2d(i, j);
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
            RiaWeightedMeanCalculator<float> calculator;
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
bool Rim2dGridProjection::hasResultAt(uint i, uint j) const
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
cvf::Vec2ui Rim2dGridProjection::surfaceGridSize() const
{
    cvf::BoundingBox boundingBox = expandedBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();

    uint projectionSizeX = static_cast<uint>(std::ceil(gridExtent.x() / m_sampleSpacing)) + 1u;
    uint projectionSizeY = static_cast<uint>(std::ceil(gridExtent.y() / m_sampleSpacing)) + 1u;

    return cvf::Vec2ui(projectionSizeX, projectionSizeY);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint Rim2dGridProjection::vertexCount() const
{
    cvf::Vec2ui gridSize2d = surfaceGridSize();
    return gridSize2d.x() * gridSize2d.y();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* Rim2dGridProjection::legendConfig() const
{
    RimEclipseView* view = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(view);
    return view->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::calculateCellRangeVisibility()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    RimCellRangeFilterCollection* rangeFilterCollection = view->rangeFilterCollection();
    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    for (size_t gridIndex = 0u; gridIndex < mainGrid()->gridCount(); ++gridIndex)
    {
        const RigGridBase* grid = mainGrid()->gridByIndex(gridIndex);

        cvf::ref<cvf::UByteArray> parentGridVisibilities;
        bool isSubGrid = false;
        if (!grid->isMainGrid())
        {
            size_t parentGridIndex = static_cast<const RigLocalGrid*>(grid)->parentGrid()->gridIndex();
            parentGridVisibilities = m_cellGridIdxVisibilityMap[parentGridIndex];
            isSubGrid = true;
        }

        m_cellGridIdxVisibilityMap[gridIndex] = new cvf::UByteArray(grid->cellCount());

#pragma omp parallel for
        for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
        {
            size_t globalCellIdx = grid->reservoirCellIndex(cellIndex);
            (*m_cellGridIdxVisibilityMap[gridIndex])[cellIndex] = activeCellInfo->isActive(globalCellIdx);
        }

        if (rangeFilterCollection && rangeFilterCollection->isActive())
        {
            cvf::CellRangeFilter cellRangeFilter;
            rangeFilterCollection->compoundCellRangeFilter(&cellRangeFilter, gridIndex);

            if (rangeFilterCollection->hasActiveIncludeFilters())
            {
#pragma omp parallel for
                for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
                {
                    size_t i, j, k;
                    grid->ijkFromCellIndex(cellIndex, &i, &j, &k);
                    if ((*m_cellGridIdxVisibilityMap[gridIndex])[cellIndex])
                    {
                        const RigCell& cell = grid->cell(cellIndex);
                        bool visibleDueToParent = false;
                        if (isSubGrid)
                        {
                            size_t parentGridCellIndex = cell.parentCellIndex();
                            visibleDueToParent = parentGridVisibilities->get(parentGridCellIndex);
                        }

                        (*m_cellGridIdxVisibilityMap[gridIndex])[cellIndex] =
                            visibleDueToParent || cellRangeFilter.isCellVisible(i, j, k, isSubGrid);
                    }
                }
            }

#pragma omp parallel for
            for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
            {
                size_t i, j, k;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);
                (*m_cellGridIdxVisibilityMap[gridIndex])[cellIndex] =
                    (*m_cellGridIdxVisibilityMap[gridIndex])[cellIndex] && !cellRangeFilter.isCellExcluded(i, j, k, isSubGrid);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::calculatePropertyFilterVisibility()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    int timeStep = view->currentTimeStep();

    for (size_t gridIndex = 0u; gridIndex < mainGrid()->gridCount(); ++gridIndex)
    {
        const RigGridBase* grid = mainGrid()->gridByIndex(gridIndex);
        RivReservoirViewPartMgr::computePropertyVisibility(m_cellGridIdxVisibilityMap[gridIndex].p(), grid, timeStep, m_cellGridIdxVisibilityMap[gridIndex].p(), view->eclipsePropertyFilterCollection());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d Rim2dGridProjection::globalPos2d(uint i, uint j) const
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
const std::vector<std::pair<size_t, float>>& Rim2dGridProjection::cellsAtPos2d(uint i, uint j) const
{
    return m_projected3dGridIndices[gridIndex(i, j)];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, float>> Rim2dGridProjection::visibleCellsAndWeightMatching2dPoint(const cvf::Vec2d& globalPos2d) const
{   
    cvf::BoundingBox gridBoundingBox = expandedBoundingBox();
    cvf::Vec3d top2dElementCentroid(globalPos2d, gridBoundingBox.max().z());
    cvf::Vec3d bottom2dElementCentroid(globalPos2d, gridBoundingBox.min().z());
    cvf::Vec3d planarDiagonalVector(0.5 * m_sampleSpacing, 0.5 * m_sampleSpacing, 0.0);
    cvf::Vec3d topNECorner = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement;
    bbox2dElement.add(topNECorner);
    bbox2dElement.add(bottomSWCorner);

    std::vector<size_t> allCellIndices;
    mainGrid()->findIntersectingCells(bbox2dElement, &allCellIndices);

    std::vector<std::tuple<size_t, float, float>> matchingVisibleCellsWeightAndHeight;

    double maxHeight = -std::numeric_limits<double>::infinity();
    double minHeight = std::numeric_limits<double>::infinity();

    double totalOverlapVolume = 0.0;
    cvf::Vec3d hexCorners[8];
    for (size_t globalCellIdx : allCellIndices)
    {
        size_t       localCellIdx = 0u;
        RigGridBase* localGrid    = mainGrid()->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIdx, &localCellIdx);
        if ((*m_cellGridIdxVisibilityMap.at(localGrid->gridIndex()))[localCellIdx])
        {
            localGrid->cellCornerVertices(localCellIdx, hexCorners);
            std::vector<HexIntersectionInfo> intersections;
            cvf::BoundingBox cellBBox;
            for (const cvf::Vec3d& corner : hexCorners)
            {
                cellBBox.add(corner);
            }
            cvf::Vec3d overlapMin, overlapMax;
            for (int i = 0; i < 3; ++i)
            {
                overlapMin[i] = std::max(cellBBox.min()[i], bbox2dElement.min()[i]);
                overlapMax[i] = std::min(cellBBox.max()[i], bbox2dElement.max()[i]);
            }
            cvf::Vec3d overlap = overlapMax - overlapMin;
            double overlapVolume = 0.0;
            if (overlap.x() > 0.0 && overlap.y() > 0.0 && overlap.z() > 0.0)
            {
                overlapVolume = overlap.x() * overlap.y() * overlap.z();
            
                double height = cellBBox.max().z();
                matchingVisibleCellsWeightAndHeight.push_back(std::make_tuple(globalCellIdx, overlapVolume, height));
                totalOverlapVolume += overlapVolume;

                maxHeight = std::max(cellBBox.max().z(), maxHeight);
                minHeight = std::min(cellBBox.min().z(), minHeight);
            }
        }
    }

    double chopped2dBBoxVolume = std::fabs(maxHeight - minHeight) * m_sampleSpacing * m_sampleSpacing;

    std::vector<std::pair<size_t, float>> matchingVisibleCellsAndWeight;
    if (totalOverlapVolume > 0.0)
    {
        std::sort(matchingVisibleCellsWeightAndHeight.begin(),
            matchingVisibleCellsWeightAndHeight.end(),
            [](const std::tuple<size_t, float, float>& lhs, const std::tuple<size_t, float, float>& rhs) {
            return std::get<2>(lhs) > std::get<2>(rhs);
        });

        float adjustmentFactor = static_cast<float>(chopped2dBBoxVolume / totalOverlapVolume);
        CVF_ASSERT(adjustmentFactor > 0.0f);
        for (const auto& visWeightHeight : matchingVisibleCellsWeightAndHeight)
        {
            matchingVisibleCellsAndWeight.push_back(std::make_pair(std::get<0>(visWeightHeight), std::get<1>(visWeightHeight) * adjustmentFactor));
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const
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
const RimEclipseResultCase* Rim2dGridProjection::eclipseCase() const
{
    const RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t Rim2dGridProjection::gridIndex(uint i, uint j) const
{
    cvf::Vec2ui gridSize2d = surfaceGridSize();

    CVF_ASSERT(i < gridSize2d.x());
    CVF_ASSERT(j < gridSize2d.y());

    return i + j * gridSize2d.x();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui Rim2dGridProjection::ijFromGridIndex(size_t index) const
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
void Rim2dGridProjection::updateLegend()
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
std::vector<double> Rim2dGridProjection::xPositions() const
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
std::vector<double> Rim2dGridProjection::yPositions() const
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
RigMainGrid* Rim2dGridProjection::mainGrid() const
{
    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    return eclipseCase->eclipseCaseData()->mainGrid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
void Rim2dGridProjection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{    
    if (&m_sampleSpacing == field)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            double characteristicSize = mainGrid()->characteristicIJCellSize();
            myAttr->m_minimum = 0.3333 * characteristicSize;
            myAttr->m_maximum = 2.0 * characteristicSize;
        }        
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::initAfterRead()
{
    legendConfig()->disableAllTimeStepsRange(isSummationResult());
}
