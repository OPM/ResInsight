#include "Rim2dGridProjection.h"

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

    CAF_PDM_InitField(&m_relativeSampleSpacing, "SampleSpacing", 0.75, "Sample Spacing Factor", "", "", "");
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

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
    boundingBox.expand(sampleSpacing() * 0.5);
    return boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::generateGridMapping()
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
double Rim2dGridProjection::meanValue() const
{
    return sumAllValues() / validVertexCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim2dGridProjection::sumAllValues() const
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
double Rim2dGridProjection::sampleSpacing() const
{
    return m_relativeSampleSpacing * mainGrid()->characteristicIJCellSize();
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
const std::vector<double>& Rim2dGridProjection::aggregatedResults() const
{
    return m_aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim2dGridProjection::isSummationResult() const
{
    return isColumnResult() || m_resultAggregation() == RESULTS_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim2dGridProjection::isColumnResult() const
{
    return m_resultAggregation() == RESULTS_OIL_COLUMN ||
           m_resultAggregation() == RESULTS_GAS_COLUMN ||
           m_resultAggregation() == RESULTS_HC_COLUMN;
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
double Rim2dGridProjection::calculateVolumeSum(uint i, uint j) const
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
double Rim2dGridProjection::calculateSoilSum(uint i, uint j) const
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

    uint projectionSizeX = static_cast<uint>(std::ceil(gridExtent.x() / sampleSpacing())) + 1u;
    uint projectionSizeY = static_cast<uint>(std::ceil(gridExtent.y() / sampleSpacing())) + 1u;

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
uint Rim2dGridProjection::validVertexCount() const
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
RimRegularLegendConfig* Rim2dGridProjection::legendConfig() const
{
    RimEclipseView* view = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(view);
    return view->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dGridProjection::calculateTotalCellVisibility()
{
    RimEclipseView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);

    m_cellGridIdxVisibility = view->currentTotalCellVisibility();
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
const std::vector<std::pair<size_t, double>>& Rim2dGridProjection::cellsAtPos2d(uint i, uint j) const
{
    return m_projected3dGridIndices[gridIndex(i, j)];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> Rim2dGridProjection::visibleCellsAndWeightMatching2dPoint(const cvf::Vec2d& globalPos2d) const
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
    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            size_t       localCellIdx = 0u;
            RigGridBase* localGrid = mainGrid()->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIdx, &localCellIdx);

            localGrid->cellCornerVertices(localCellIdx, hexCorners.data());

            cvf::BoundingBox overlapBBox;
            std::array<cvf::Vec3d, 8> overlapCorners = createHexOverlapEstimation(bbox2dElement, hexCorners, &overlapBBox);
            
            double overlapVolume = RigCellGeometryTools::calculateCellVolume(overlapCorners);

            if (overlapVolume > 0.0)
            {            
                double height = overlapBBox.max().z();
                matchingVisibleCellsWeightAndHeight.push_back(std::make_tuple(globalCellIdx, overlapVolume, height));
            }
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
            matchingVisibleCellsAndWeight.push_back(std::make_pair(std::get<0>(visWeightHeight), std::get<1>(visWeightHeight)));
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
double Rim2dGridProjection::findSoilResult(size_t cellGlobalIdx) const
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
std::array<cvf::Vec3d, 8> Rim2dGridProjection::createHexOverlapEstimation(const cvf::BoundingBox& bbox2dElement, const std::array<cvf::Vec3d, 8>& hexCorners, cvf::BoundingBox* overlapBoundingBox) const
{
    CVF_ASSERT(overlapBoundingBox);
    *overlapBoundingBox = cvf::BoundingBox();
    std::array<cvf::Vec3d, 8> overlapCorners = hexCorners;
    // A reasonable approximation to the overlap volume
    cvf::Plane topPlane;    topPlane.setFromPoints(hexCorners[0], hexCorners[1], hexCorners[2]);
    cvf::Plane bottomPlane; bottomPlane.setFromPoints(hexCorners[4], hexCorners[5], hexCorners[6]);

    for (size_t i = 0; i < 4; ++i)
    {
        cvf::Vec3d& corner = overlapCorners[i];
        corner.x() = cvf::Math::clamp(corner.x(), bbox2dElement.min().x(), bbox2dElement.max().x());
        corner.y() = cvf::Math::clamp(corner.y(), bbox2dElement.min().y(), bbox2dElement.max().y());
        cvf::Vec3d maxZCorner = corner; maxZCorner.z() = bbox2dElement.max().z();
        cvf::Vec3d minZCorner = corner; minZCorner.z() = bbox2dElement.min().z();
        topPlane.intersect(maxZCorner, minZCorner, &corner);
        overlapBoundingBox->add(corner);
    }
    for (size_t i = 4; i < 8; ++i)
    {
        cvf::Vec3d& corner = overlapCorners[i];
        corner.x() = cvf::Math::clamp(corner.x(), bbox2dElement.min().x(), bbox2dElement.max().x());
        corner.y() = cvf::Math::clamp(corner.y(), bbox2dElement.min().y(), bbox2dElement.max().y());
        cvf::Vec3d maxZCorner = corner; maxZCorner.z() = bbox2dElement.max().z();
        cvf::Vec3d minZCorner = corner; minZCorner.z() = bbox2dElement.min().z();
        bottomPlane.intersect(maxZCorner, minZCorner, &corner);
        overlapBoundingBox->add(corner);
    }
    return overlapCorners;
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
Rim2dGridProjection::ResultAggregation Rim2dGridProjection::resultAggregation() const
{
    return m_resultAggregation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim2dGridProjection::resultAggregationText() const
{
    return m_resultAggregation().uiText();
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
