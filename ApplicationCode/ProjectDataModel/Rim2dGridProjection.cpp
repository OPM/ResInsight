#include "Rim2dGridProjection.h"

#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
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
        addItem(Rim2dGridProjection::RESULTS_MEAN_VALUE, "MEAN_VALUE", "Mean Value");
        addItem(Rim2dGridProjection::RESULTS_MIN_VALUE, "MIN_VALUE", "Min Value");
        addItem(Rim2dGridProjection::RESULTS_MAX_VALUE, "MAX_VALUE", "Max Value");

        setDefault(Rim2dGridProjection::RESULTS_MEAN_VALUE);
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

    setName("2d Grid Projection");
    nameField()->uiCapability()->setUiReadOnly(true);
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
void Rim2dGridProjection::generateGridMapping()
{
    updateDefaultSampleSpacingFromGrid();

    calculateCellRangeVisibility();
    calculatePropertyFilterVisibility();
    
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();

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
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();

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

    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();

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

    m_resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(), 0, timeStep, cellColors);
    if (m_resultAccessor.notNull())
    {
        if (!cellColors->isTernarySaturationSelected())
        {
#pragma omp parallel for
            for (int index = 0; index < nVertices; ++index)
            {
                cvf::Vec2ui ij = ijFromGridIndex(index);
                m_aggregatedResults[index] = value(ij.x(), ij.y());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::maxValue() const
{
    double maxV = 0.0;

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
double Rim2dGridProjection::value(uint i, uint j) const
{
    const std::vector<std::pair<size_t, float>>& matchingCells = cellsAtPos2d(i, j);
    if (!matchingCells.empty())
    {
        switch (m_resultAggregation())
        {
        case RESULTS_MEAN_VALUE:
        {
            RiaWeightedMeanCalculator<float> calculator;
            for (auto cellIdxAndWeight : matchingCells)
            {
                size_t cellIdx = cellIdxAndWeight.first;
                double cellValue = m_resultAccessor->cellScalarGlobIdx(cellIdx);
                calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
            }
            return calculator.weightedMean();
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
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
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
    const RigMainGrid* grid = mainGrid();
    m_cellVisibility = new cvf::UByteArray(grid->cellCount());

    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    
    RimCellRangeFilterCollection* rangeFilterCollection = view->rangeFilterCollection();

    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

#pragma omp parallel for
    for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
    {
        (*m_cellVisibility)[cellIndex] = activeCellInfo->isActive(cellIndex);
    }

    if (rangeFilterCollection && rangeFilterCollection->isActive())
    {
        cvf::CellRangeFilter cellRangeFilter;
        rangeFilterCollection->compoundCellRangeFilter(&cellRangeFilter, grid->gridIndex());

        if (cellRangeFilter.hasIncludeRanges())
        {
#pragma omp parallel for
            for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
            {
                size_t i, j, k;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);
                (*m_cellVisibility)[cellIndex] = (*m_cellVisibility)[cellIndex] && cellRangeFilter.isCellVisible(i, j, k, false);
            }
        }
        else
        {
#pragma omp parallel for
            for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); ++cellIndex)
            {
                size_t i, j, k;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);
                (*m_cellVisibility)[cellIndex] = (*m_cellVisibility)[cellIndex] && !cellRangeFilter.isCellExcluded(i, j, k, false);
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

    RivReservoirViewPartMgr::computePropertyVisibility(m_cellVisibility.p(), mainGrid(), timeStep, m_cellVisibility.p(), view->eclipsePropertyFilterCollection());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d Rim2dGridProjection::globalPos2d(uint i, uint j) const
{
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
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
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
    cvf::Vec3d highestPoint(globalPos2d, boundingBox.max().z());
    cvf::Vec3d lowestPoint(globalPos2d, boundingBox.min().z());

    cvf::BoundingBox rayBBox;
    rayBBox.add(highestPoint);
    rayBBox.add(lowestPoint);

    std::vector<size_t> allCellIndices;
    mainGrid()->findIntersectingCells(rayBBox, &allCellIndices);

    std::vector<std::pair<size_t, float>> matchingVisibleCellsAndWeight;

    cvf::Vec3d hexCorners[8];
    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellVisibility)[globalCellIdx])
        {
            size_t localCellIdx = 0u;
            RigGridBase* localGrid = mainGrid()->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIdx, &localCellIdx);

            localGrid->cellCornerVertices(localCellIdx, hexCorners);
            std::vector<HexIntersectionInfo> intersections;
            float weight = 1.0f;
            if (false && RigHexIntersectionTools::lineHexCellIntersection(highestPoint, lowestPoint, hexCorners, 0, &intersections))
            {
                weight = std::max(1.0, (intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint).length());
            }
            matchingVisibleCellsAndWeight.push_back(std::make_pair(globalCellIdx, weight));
        }
    }

    return matchingVisibleCellsAndWeight;
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

    generateResults();
    legendConfig()->setAutomaticRanges(minValue(), maxValue(), minValue(), maxValue());
    legendConfig()->setTitle(QString("2d Projection:\n%1").arg(cellColors->resultVariableUiShortName()));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> Rim2dGridProjection::xPositions() const
{
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
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
    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
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
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &m_isChecked || changedField == &m_sampleSpacing || changedField == &m_resultAggregation)
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
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
            myAttr->m_minimum = 0.2 * characteristicSize;
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
