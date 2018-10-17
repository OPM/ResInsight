#include "Rim2dGridProjection.h"

#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RivReservoirViewPartMgr.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimCellRangeFilterCollection.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfArray.h"
#include "cvfCellRange.h"
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

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendDefinition", "Legend Definition", ":/Legend.png", "", "");
    m_legendConfig = new RimRegularLegendConfig();

    setName("2d Grid Projection");
    nameField()->uiCapability()->setUiReadOnly(true);
    setCheckState(false); // Default is off
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
void Rim2dGridProjection::extractGridData()
{
    updateDefaultSampleSpacingFromGrid();

    cvf::BoundingBox boundingBox = eclipseCase()->activeCellsBoundingBox();
    cvf::Vec3d gridExtent = boundingBox.extent();

    cvf::Vec2ui gridSize2d = surfaceGridSize();    

    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);

    m_projected3dGridIndices.resize(vertexCount());
    for (uint j = 0; j < gridSize2d.y(); ++j)
    {
        for (uint i = 0; i < gridSize2d.x(); ++i)
        {            
            cvf::Vec2d globalPos = globalPos2d(i, j);
            m_projected3dGridIndices[gridIndex(i, j)] = visibleCellsAndWeightMatching2dPoint(globalPos);                
        }
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

    for (uint j = 0; j < gridSize2d.y(); ++j)
    {
        for (uint i = 0; i < gridSize2d.x(); ++i)
        {
            cvf::Vec2d globalPos = globalPos2d(i, j);
            cvf::Vec3d globalVertexPos(globalPos, boundingBox.max().z() + 10.0);
            cvf::Vec3f displayVertexPos(displayCoordTransform->transformToDisplayCoord(globalVertexPos));
            (*vertices)[gridIndex(i, j)] = displayVertexPos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::maxValue() const
{
    double maxV = 0.0;
    cvf::Vec2ui gridSize = surfaceGridSize();
    for (uint i = 0; i < gridSize.x(); ++i)
    {
        for (uint j = 0; j < gridSize.y(); ++j)
        {
            if (hasResultAt(i, j))
            {
                maxV = std::max(maxV, value(i, j));
            }
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
    cvf::Vec2ui gridSize = surfaceGridSize();
    for (uint i = 0; i < gridSize.x(); ++i)
    {
        for (uint j = 0; j < gridSize.y(); ++j)
        {
            if (hasResultAt(i, j))
            {
                minV = std::min(minV, value(i, j));
            }
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
        m_sampleSpacing = mainGrid()->characteristicIJCellSize();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::value(uint i, uint j) const
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    int timeStep = view->currentTimeStep();

    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfTypeAsserted(eclipseCase);
    RimEclipseCellColors* cellColors = view->cellResult();

    if (cellColors->isTernarySaturationSelected())
    {
        return std::numeric_limits<double>::infinity();
    }

    cvf::ref<RigResultAccessor> resultAccessor =
        RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(), 0, timeStep, cellColors);
    
    CVF_ASSERT(resultAccessor.notNull());
    double minValue =  std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();
    double avgValue = 0.0;
    
    std::vector<std::pair<size_t, float>> matchingCells = cellsAtPos2d(i, j);
    if (!matchingCells.empty())
    {
        RiaWeightedMeanCalculator<float> calculator;
        for (auto cellIdxAndWeight : matchingCells)
        {
            size_t cellIdx = cellIdxAndWeight.first;
            double cellValue = resultAccessor->cellScalarGlobIdx(cellIdx);
            minValue = std::min(minValue, cellValue);
            maxValue = std::max(maxValue, cellValue);
            calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
        }
        avgValue = calculator.weightedMean();

        switch (m_resultAggregation())
        {
        case RESULTS_MEAN_VALUE:
            return avgValue;
        case RESULTS_MAX_VALUE:
            return maxValue;
        case RESULTS_MIN_VALUE:
            return minValue;
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
    return m_legendConfig;
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
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    //int timeStep = view->currentTimeStep();

    std::vector<RimCellRangeFilterCollection*> rangeFilterCollections;
    view->descendantsIncludingThisOfType(rangeFilterCollections);

    bool hasActiveFilters = rangeFilterCollections.front()->hasActiveFilters();
    cvf::CellRangeFilter cellRangeFilter;
    if (hasActiveFilters)
    {
        rangeFilterCollections.front()->compoundCellRangeFilter(&cellRangeFilter, mainGrid()->gridIndex());
    }

    const RigActiveCellInfo* activeCellInfo = eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    cvf::BoundingBox boundingBox = eclipseCase()->allCellsBoundingBox();
    cvf::Vec3d highestPoint(globalPos2d, boundingBox.max().z());
    cvf::Vec3d lowestPoint(globalPos2d, boundingBox.min().z());

    cvf::BoundingBox rayBBox;
    rayBBox.add(highestPoint);
    rayBBox.add(lowestPoint);

    std::vector<size_t> allCellIndices;
    mainGrid()->findIntersectingCells(rayBBox, &allCellIndices);

    std::vector<std::pair<size_t, float>> visibleCellIndices;
    cvf::Vec3d hexCorners[8];
    for (size_t globalCellIdx : allCellIndices)
    {
        if (activeCellInfo->isActive(globalCellIdx))
        {
            size_t localCellIdx = 0u;
            RigGridBase* localGrid = mainGrid()->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIdx, &localCellIdx);
            size_t i, j, k;
            localGrid->ijkFromCellIndex(localCellIdx, &i, &j, &k);
            if (!hasActiveFilters || cellRangeFilter.isCellVisible(i, j, k, !localGrid->isMainGrid()))
            {
                localGrid->cellCornerVertices(localCellIdx, hexCorners);
                std::vector<HexIntersectionInfo> intersections;
                float weight = 1.0f;
                if (RigHexIntersectionTools::lineHexCellIntersection(highestPoint, lowestPoint, hexCorners, 0, &intersections))
                {
                    weight = std::max(1.0, (intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint).length());                    
                }
                visibleCellIndices.push_back(std::make_pair(globalCellIdx, weight));
            }
        }
    }

    return visibleCellIndices;
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
void Rim2dGridProjection::updateLegendData()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    RimEclipseCellColors* cellColors = view->cellResult();

    extractGridData();
    m_legendConfig->setAutomaticRanges(minValue(), maxValue(), minValue(), maxValue());
    m_legendConfig->setTitle(QString("2d Projection:\n%1").arg(cellColors->resultVariableUiShortName()));
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
            myAttr->m_minimum = 0.1 * characteristicSize;
            myAttr->m_maximum = 5.0 * characteristicSize;
        }        
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(legendConfig());
    uiTreeOrdering.skipRemainingChildren(true);
}
