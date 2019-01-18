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

#include "RimContourMapProjection.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCellGeometryTools.h"
#include "RigHexIntersectionTools.h"

#include "RimGridView.h"
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

#include <QDebug>

namespace caf
{
template<>
void RimContourMapProjection::ResultAggregation::setUp()
{
    addItem(RimContourMapProjection::RESULTS_OIL_COLUMN, "OIL_COLUMN", "Oil Column");
    addItem(RimContourMapProjection::RESULTS_GAS_COLUMN, "GAS_COLUMN", "Gas Column");
    addItem(RimContourMapProjection::RESULTS_HC_COLUMN, "HC_COLUMN", "Hydrocarbon Column");

    addItem(RimContourMapProjection::RESULTS_MEAN_VALUE, "MEAN_VALUE", "Arithmetic Mean");
    addItem(RimContourMapProjection::RESULTS_HARM_VALUE, "HARM_VALUE", "Harmonic Mean");
    addItem(RimContourMapProjection::RESULTS_GEOM_VALUE, "GEOM_VALUE", "Geometric Mean");
    addItem(RimContourMapProjection::RESULTS_VOLUME_SUM, "VOLUME_SUM", "Volume Weighted Sum");
    addItem(RimContourMapProjection::RESULTS_SUM, "SUM", "Sum");

    addItem(RimContourMapProjection::RESULTS_TOP_VALUE, "TOP_VALUE", "Top  Value");
    addItem(RimContourMapProjection::RESULTS_MIN_VALUE, "MIN_VALUE", "Min Value");
    addItem(RimContourMapProjection::RESULTS_MAX_VALUE, "MAX_VALUE", "Max Value");

    setDefault(RimContourMapProjection::RESULTS_MEAN_VALUE);
}
} // namespace caf
CAF_PDM_ABSTRACT_SOURCE_INIT(RimContourMapProjection, "RimContourMapProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::RimContourMapProjection()
    : m_pickPoint(cvf::Vec2d::UNDEFINED)
    , m_mapSize(cvf::Vec2ui(0u, 0u))
    , m_sampleSpacing(-1.0)
    , m_currentResultTimestep(-1)
{
    CAF_PDM_InitObject("RimContourMapProjection", ":/2DMapProjection16x16.png", "", "");

    CAF_PDM_InitField(&m_relativeSampleSpacing, "SampleSpacing", 0.75, "Sample Spacing Factor", "", "", "");
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_resultAggregation, "ResultAggregation", "Result Aggregation", "", "", "");

    CAF_PDM_InitField(&m_showContourLines, "ContourLines", true, "Show Contour Lines", "", "", "");
    CAF_PDM_InitField(&m_showContourLabels, "ContourLabels", true, "Show Contour Labels", "", "", "");
    CAF_PDM_InitField(&m_smoothContourLines, "SmoothContourLines", true, "Smooth Contour Lines", "", "", "");

    setName("Map Projection");
    nameField()->uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::~RimContourMapProjection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateResultsIfNecessary(int timeStep)
{
    updateGridInformation();

    if (gridMappingNeedsUpdating())
    {
        generateGridMapping();
    }

    if (resultsNeedsUpdating(timeStep))
    {
        generateResults(timeStep);
        generateVertexResults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateGeometryIfNecessary()
{
    if (geometryNeedsUpdating())
    {
        generateContourPolygons();
        generateTrianglesWithVertexValues();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimContourMapProjection::generatePickPointPolygon()
{
    std::vector<cvf::Vec3d> points;

    if (!m_pickPoint.isUndefined())
    {
        {
            cvf::Vec2d  cellDiagonal(m_sampleSpacing * 0.5, m_sampleSpacing * 0.5);
            cvf::Vec2ui pickedCell = ijFromLocalPos(m_pickPoint);
            cvf::Vec2d  cellCenter = cellCenterPosition(pickedCell.x(), pickedCell.y());
            cvf::Vec2d  cellCorner = cellCenter - cellDiagonal;
#ifndef NDEBUG
            points.push_back(cvf::Vec3d(cellCorner, 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, 0.0), 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, 0.0), 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, m_sampleSpacing), 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, m_sampleSpacing), 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(0.0, m_sampleSpacing), 0.0));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(0.0, m_sampleSpacing), 0.0));
            points.push_back(cvf::Vec3d(cellCorner, 0.0));
#endif
            points.push_back(cvf::Vec3d(m_pickPoint - cvf::Vec2d(0.5 * m_sampleSpacing, 0.0), 0.0));
            points.push_back(cvf::Vec3d(m_pickPoint + cvf::Vec2d(0.5 * m_sampleSpacing, 0.0), 0.0));
            points.push_back(cvf::Vec3d(m_pickPoint - cvf::Vec2d(0.0, 0.5 * m_sampleSpacing), 0.0));
            points.push_back(cvf::Vec3d(m_pickPoint + cvf::Vec2d(0.0, 0.5 * m_sampleSpacing), 0.0));
        }
    }
    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::clearGeometry()
{
    m_contourPolygons.clear();
    m_trianglesWithVertexValues.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RimContourMapProjection::ContourPolygons>& RimContourMapProjection::contourPolygons() const
{
    return m_contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec4d>& RimContourMapProjection::trianglesWithVertexValues()
{
    return m_trianglesWithVertexValues;
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
double RimContourMapProjection::sampleSpacing() const
{
    return m_sampleSpacing;
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
bool RimContourMapProjection::showContourLabels() const
{
    return m_showContourLines() && m_showContourLabels();
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
double RimContourMapProjection::maxValue() const
{
    double maxV = -std::numeric_limits<double>::infinity();

    for (size_t index = 0; index < m_aggregatedResults.size(); ++index)
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

    for (size_t index = 0; index < m_aggregatedResults.size(); ++index)
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
    return sumAllValues() / numberOfValidCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumAllValues() const
{
    double sum = 0.0;

    for (size_t index = 0; index < m_aggregatedResults.size(); ++index)
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
cvf::Vec2ui RimContourMapProjection::numberOfElementsIJ() const
{
    return m_mapSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::numberOfVerticesIJ() const
{
    cvf::Vec2ui mapSize = this->numberOfElementsIJ();
    mapSize.x() += 1u;
    mapSize.y() += 1u;
    return mapSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isColumnResult() const
{
    return m_resultAggregation() == RESULTS_OIL_COLUMN || m_resultAggregation() == RESULTS_GAS_COLUMN ||
           m_resultAggregation() == RESULTS_HC_COLUMN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::valueAtVertex(uint i, uint j) const
{
    size_t index = vertexIndexFromIJ(i, j);
    if (index < numberOfVertices())
    {
        return m_aggregatedVertexResults.at(index);
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::hasResultAtVertex(uint i, uint j) const
{
    size_t index = vertexIndexFromIJ(i, j);
    return m_aggregatedVertexResults[index] != std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint RimContourMapProjection::numberOfCells() const
{
    return m_mapSize.x() * m_mapSize.y();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint RimContourMapProjection::numberOfValidCells() const
{
    uint validCount = 0u;
    for (uint i = 0; i < numberOfCells(); ++i)
    {
        cvf::Vec2ui ij = ijFromCellIndex(i);
        if (hasResultInCell(ij.x(), ij.y()))
        {
            validCount++;
        }
    }
    return validCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::numberOfVertices() const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();
    return static_cast<size_t>(gridSize.x()) * static_cast<size_t>(gridSize.y());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::checkForMapIntersection(const cvf::Vec3d& localPoint3d,
                                                             cvf::Vec2d*       contourMapPoint,
                                                             double*           valueAtPoint) const
{
    CVF_TIGHT_ASSERT(contourMapPoint);
    CVF_TIGHT_ASSERT(valueAtPoint);

    cvf::Vec3d localPos3d(localPoint3d.x() + gridEdgeOffset(), localPoint3d.y() + gridEdgeOffset(), 0.0);
    cvf::Vec2d gridPos2d(localPos3d.x(), localPos3d.y());
    cvf::Vec2d gridorigin(m_expandedBoundingBox.min().x(), m_expandedBoundingBox.min().y());

    double value = interpolateValue(gridPos2d);
    if (value != std::numeric_limits<double>::infinity())
    {
        *valueAtPoint    = value;
        *contourMapPoint = gridPos2d + gridorigin;

        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::setPickPoint(cvf::Vec2d globalPickPoint)
{
    m_pickPoint = globalPickPoint - origin2d();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimContourMapProjection::origin3d() const
{
    return m_expandedBoundingBox.min();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValueInMapCell(uint i, uint j) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtIJ(i, j);
    if (!matchingCells.empty())
    {
        switch (m_resultAggregation())
        {
            case RESULTS_TOP_VALUE:
            {
                size_t cellIdx   = matchingCells.front().first;
                double cellValue = gridCellValue(cellIdx);
                return cellValue;
            }
            case RESULTS_MEAN_VALUE:
            {
                RiaWeightedMeanCalculator<double> calculator;
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = gridCellValue(cellIdx);
                    if (cellValue != std::numeric_limits<double>::infinity())
                    {
                        calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                    }
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
                    double cellValue = gridCellValue(cellIdx);
                    if (cellValue < 1.0e-8)
                    {
                        return 0.0;
                    }
                    if (cellValue != std::numeric_limits<double>::infinity())
                    {
                        calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                    }
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
                    double cellValue = gridCellValue(cellIdx);
                    if (std::fabs(cellValue) < 1.0e-8)
                    {
                        return 0.0;
                    }
                    if (cellValue != std::numeric_limits<double>::infinity())
                    {
                        calculator.addValueAndWeight(cellValue, cellIdxAndWeight.second);
                    }
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
                    double cellValue = gridCellValue(cellIdx);
                    if (cellValue != std::numeric_limits<double>::infinity())
                    {
                        maxValue = std::max(maxValue, cellValue);
                    }                    
                }
                return maxValue;
            }
            case RESULTS_MIN_VALUE:
            {
                double minValue = std::numeric_limits<double>::infinity();
                for (auto cellIdxAndWeight : matchingCells)
                {
                    size_t cellIdx   = cellIdxAndWeight.first;
                    double cellValue = gridCellValue(cellIdx);
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
                    double cellValue = gridCellValue(cellIdx);
                    if (cellValue != std::numeric_limits<double>::infinity())
                    {
                        sum += cellValue * cellIdxAndWeight.second;
                    }
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
bool RimContourMapProjection::gridMappingNeedsUpdating() const
{
    if (m_projected3dGridIndices.size() != numberOfCells())
    {
        return true;
    }

    if (m_cellGridIdxVisibility.isNull())
    {
        return true;
    }
    cvf::ref<cvf::UByteArray> currentVisibility = getCellVisibility();

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
bool RimContourMapProjection::resultsNeedsUpdating(int timeStep) const
{
    if (m_aggregatedResults.size() != numberOfCells())
    {
        return true;
    }

    if (m_aggregatedVertexResults.size() != numberOfVertices())
    {
        return true;
    }

    if (timeStep != m_currentResultTimestep)
    {
        return true;
    }
    return resultVariableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::geometryNeedsUpdating() const
{
    return m_contourPolygons.empty() || m_trianglesWithVertexValues.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::clearGridMapping()
{
    clearResults();

    m_projected3dGridIndices.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::clearResults()
{
    clearGeometry();

    m_aggregatedResults.clear();
    m_aggregatedVertexResults.clear();
    m_currentResultTimestep = -1;

    clearResultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimContourMapProjection::getCellVisibility() const
{
    return baseView()->currentTotalCellVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateGridMapping()
{
    clearResults();

    m_cellGridIdxVisibility = getCellVisibility();

    int nCells = numberOfCells();
    m_projected3dGridIndices.resize(nCells);

    std::vector<double> weightingResultValues = retrieveParameterWeights();

    if (isStraightSummationResult())
    {
#pragma omp parallel for
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos            = cellCenterPosition(ij.x(), ij.y()) + origin2d();
            m_projected3dGridIndices[index] = cellRayIntersectionAndResults(globalPos, weightingResultValues);
        }
    }
    else
    {
#pragma omp parallel for
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos            = cellCenterPosition(ij.x(), ij.y()) + origin2d();
            m_projected3dGridIndices[index] = cellOverlapVolumesAndResults(globalPos, weightingResultValues);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateVertexResults()
{
    size_t nCells = numberOfCells();
    if (nCells != m_aggregatedResults.size())
        return;

    size_t nVertices          = numberOfVertices();
    m_aggregatedVertexResults = std::vector<double>(nVertices, std::numeric_limits<double>::infinity());
#pragma omp parallel for
    for (int index = 0; index < static_cast<int>(nVertices); ++index)
    {
        cvf::Vec2ui ij                   = ijFromVertexIndex(index);
        m_aggregatedVertexResults[index] = calculateValueAtVertex(ij.x(), ij.y());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateTrianglesWithVertexValues()
{
    std::vector<cvf::Vec3d> vertices = generateVertices();

    cvf::Vec2ui              patchSize = numberOfVerticesIJ();
    cvf::ref<cvf::UIntArray> faceList  = new cvf::UIntArray;
    cvf::GeometryUtils::tesselatePatchAsTriangles(patchSize.x(), patchSize.y(), 0u, true, faceList.p());

    bool                discrete = false;
    std::vector<double> contourLevels;
    if (legendConfig()->mappingMode() != RimRegularLegendConfig::CATEGORY_INTEGER)
    {
        legendConfig()->scalarMapper()->majorTickValues(&contourLevels);
        if (legendConfig()->mappingMode() == RimRegularLegendConfig::LINEAR_DISCRETE ||
            legendConfig()->mappingMode() == RimRegularLegendConfig::LOG10_DISCRETE)
        {
            discrete = true;
        }
    }

    const double cellArea = m_sampleSpacing * m_sampleSpacing;
    const double areaThreshold = 1.0e-5 * 0.5 * cellArea;

    std::vector<std::vector<std::vector<cvf::Vec3d>>> subtractPolygons;
    if (!m_contourPolygons.empty())
    {
        subtractPolygons.resize(m_contourPolygons.size());
        for (size_t i = 0; i < m_contourPolygons.size() - 1; ++i)
        {
            for (size_t j = 0; j < m_contourPolygons[i + 1].size(); ++j)
            {
                subtractPolygons[i].push_back(m_contourPolygons[i + 1][j].vertices);
            }
        }
    }
    std::vector<std::vector<std::vector<cvf::Vec4d>>> threadTriangles(omp_get_max_threads());

#pragma omp parallel
    {
        int myThread = omp_get_thread_num();
        threadTriangles[myThread].resize(std::max((size_t) 1, m_contourPolygons.size()));

#pragma omp for schedule(dynamic)
        for (int64_t i = 0; i < (int64_t)faceList->size(); i += 3)
        {
            std::vector<cvf::Vec3d> triangle(3);
            std::vector<cvf::Vec4d> triangleWithValues(3);
            bool anyValidVertex = false;
            for (size_t n = 0; n < 3; ++n)
            {
                uint   vn             = (*faceList)[i + n];
                double value = vn < m_aggregatedVertexResults.size() ? m_aggregatedVertexResults[vn]
                                                                     : std::numeric_limits<double>::infinity();
                triangle[n]           = vertices[vn];
                triangleWithValues[n] = cvf::Vec4d(vertices[vn], value);
                if (value != std::numeric_limits<double>::infinity())
                {
                    anyValidVertex = true;
                }
            }

            if (!anyValidVertex)
            {
                continue;
            }

            if (m_contourPolygons.empty())
            {
                threadTriangles[myThread][0].insert(
                    threadTriangles[myThread][0].end(), triangleWithValues.begin(), triangleWithValues.end());
                continue;
            }

            bool outsideOuterLimit = false;
            for (size_t c = 0; c < m_contourPolygons.size() && !outsideOuterLimit; ++c)
            {
                std::vector<std::vector<cvf::Vec3d>> intersectPolygons;
                for (size_t j = 0; j < m_contourPolygons[c].size(); ++j)
                {
                    bool containsAtLeastOne = false;
                    for (size_t t = 0; t < 3; ++t)
                    {
                        if (m_contourPolygons[c][j].bbox.contains(triangle[t]))
                        {
                            containsAtLeastOne = true;
                        }
                    }
                    if (containsAtLeastOne)
                    {
                        std::vector<std::vector<cvf::Vec3d>> clippedPolygons =
                            RigCellGeometryTools::intersectPolygons(triangle, m_contourPolygons[c][j].vertices);
                        intersectPolygons.insert(intersectPolygons.end(), clippedPolygons.begin(), clippedPolygons.end());
                    }
                }

                if (intersectPolygons.empty())
                {
                    outsideOuterLimit = true;
                    continue;
                }
                
                std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

                if (!subtractPolygons[c].empty())
                {
                    for (const std::vector<cvf::Vec3d>& polygon : intersectPolygons)
                    {
                        std::vector<std::vector<cvf::Vec3d>> fullyClippedPolygons =
                            RigCellGeometryTools::subtractPolygons(polygon, subtractPolygons[c]);
                        clippedPolygons.insert(clippedPolygons.end(), fullyClippedPolygons.begin(), fullyClippedPolygons.end());
                    }
                }
                else
                {
                    clippedPolygons.swap(intersectPolygons);
                }

                {
                    std::vector<cvf::Vec4d> clippedTriangles;
                    for (std::vector<cvf::Vec3d>& clippedPolygon : clippedPolygons)
                    {
                        std::vector<std::vector<cvf::Vec3d>> polygonTriangles;
                        if (clippedPolygon.size() == 3u)
                        {
                            polygonTriangles.push_back(clippedPolygon);
                        }
                        else
                        {
                            cvf::Vec3d baryCenter = cvf::Vec3d::ZERO;
                            for (size_t v = 0; v < clippedPolygon.size(); ++v)
                            {
                                cvf::Vec3d& clippedVertex = clippedPolygon[v];
                                baryCenter += clippedVertex;
                            }
                            baryCenter /= clippedPolygon.size();
                            for (size_t v = 0; v < clippedPolygon.size(); ++v)
                            {
                                std::vector<cvf::Vec3d> clippedTriangle;
                                if (v == clippedPolygon.size() - 1)
                                {
                                    clippedTriangle = {clippedPolygon[v], clippedPolygon[0], baryCenter};
                                }
                                else
                                {
                                    clippedTriangle = {clippedPolygon[v], clippedPolygon[v + 1], baryCenter};
                                }
                                polygonTriangles.push_back(clippedTriangle);
                            }
                        }
                        for (const std::vector<cvf::Vec3d>& polygonTriangle : polygonTriangles)
                        {
                            // Check triangle area
                            double area = 0.5 * ((polygonTriangle[1] - polygonTriangle[0]) ^ (polygonTriangle[2] - polygonTriangle[0])).length();
                            if (area < areaThreshold)
                                continue;
                            for (const cvf::Vec3d& localVertex : polygonTriangle)
                            {
                                double value = std::numeric_limits<double>::infinity();
                                if (discrete)
                                {
                                    value = contourLevels[c] +
                                            0.01 * (contourLevels.back() - contourLevels.front()) / contourLevels.size();
                                }
                                else
                                {
                                    for (size_t n = 0; n < 3; ++n)
                                    {
                                        if ((triangle[n] - localVertex).length() < m_sampleSpacing * 0.01 &&
                                            triangleWithValues[n].w() != std::numeric_limits<double>::infinity())
                                        {
                                            value = triangleWithValues[n].w();
                                            break;
                                        }
                                    }
                                    if (value == std::numeric_limits<double>::infinity())
                                    {
                                        value = interpolateValue(cvf::Vec2d(localVertex.x(), localVertex.y()));
                                        if (value == std::numeric_limits<double>::infinity())
                                        {
                                            value = contourLevels[c];
                                        }
                                    }
                                }

                                cvf::Vec4d globalVertex(localVertex, value);
                                clippedTriangles.push_back(globalVertex);
                            }
                        }
                    }
                    threadTriangles[myThread][c].insert(
                        threadTriangles[myThread][c].end(), clippedTriangles.begin(), clippedTriangles.end());
                }
            }
        }
    }

    std::vector<std::vector<cvf::Vec4d>> trianglesPerLevel(std::max((size_t)1, contourLevels.size()));
    for (size_t c = 0; c < trianglesPerLevel.size(); ++c)
    {
        std::vector<cvf::Vec4d> allTrianglesThisLevel;
        for (size_t i = 0; i < threadTriangles.size(); ++i)
        {
            allTrianglesThisLevel.insert(allTrianglesThisLevel.end(), threadTriangles[i][c].begin(), threadTriangles[i][c].end());
        }

        double triangleAreasThisLevel = sumTriangleAreas(allTrianglesThisLevel);
        if (triangleAreasThisLevel > 1.0e-3 * m_contourLevelCumulativeAreas[c])
        {
            trianglesPerLevel[c] = allTrianglesThisLevel;
        }
        else
        {
            m_contourPolygons[c].clear();
        }
    }

    std::vector<cvf::Vec4d> finalTriangles;
    for (size_t i = 0; i < trianglesPerLevel.size(); ++i)
    {
        finalTriangles.insert(finalTriangles.end(), trianglesPerLevel[i].begin(), trianglesPerLevel[i].end());
    }

    m_trianglesWithVertexValues = finalTriangles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimContourMapProjection::generateVertices() const
{
    size_t                  nVertices = numberOfVertices();
    std::vector<cvf::Vec3d> vertices(nVertices, cvf::Vec3d::ZERO);

#pragma omp parallel for
    for (int index = 0; index < static_cast<int>(nVertices); ++index)
    {
        cvf::Vec2ui ij     = ijFromVertexIndex(index);
        cvf::Vec2d  mapPos = cellCenterPosition(ij.x(), ij.y());
        // Shift away from sample point to vertex
        mapPos.x() -= m_sampleSpacing * 0.5;
        mapPos.y() -= m_sampleSpacing * 0.5;

        cvf::Vec3d vertexPos(mapPos, 0.0);
        vertices[index] = vertexPos;
    }
    return vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateContourPolygons()
{
    std::vector<ContourPolygons> contourPolygons;

    const double areaTreshold = (m_sampleSpacing * m_sampleSpacing) / (sampleSpacingFactor() * sampleSpacingFactor());

    if (minValue() != std::numeric_limits<double>::infinity() && maxValue() != -std::numeric_limits<double>::infinity() &&
        std::fabs(maxValue() - minValue()) > 1.0e-8)
    {
        std::vector<double> contourLevels;
        if (legendConfig()->mappingMode() != RimRegularLegendConfig::CATEGORY_INTEGER)
        {
            legendConfig()->scalarMapper()->majorTickValues(&contourLevels);
            int nContourLevels = static_cast<int>(contourLevels.size());
            if (nContourLevels > 2)
            {
                if (legendConfig()->mappingMode() == RimRegularLegendConfig::LINEAR_DISCRETE ||
                    legendConfig()->mappingMode() == RimRegularLegendConfig::LINEAR_CONTINUOUS)
                {
                    contourLevels.front() -= 0.01 * (contourLevels.back() - contourLevels.front());
                }
                else
                {
                    contourLevels.front() *= 0.5;
                }

                std::vector<caf::ContourLines::ListOfLineSegments> unorderedLineSegmentsPerLevel = caf::ContourLines::create(
                    m_aggregatedVertexResults, xVertexPositions(), yVertexPositions(), contourLevels);

                std::vector<ContourPolygons>(unorderedLineSegmentsPerLevel.size()).swap(contourPolygons);
                
                for (size_t i = 0; i < unorderedLineSegmentsPerLevel.size(); ++i)
                {
                    std::vector<std::vector<cvf::Vec3d>> polygonsForThisLevel;
                    RigCellGeometryTools::createPolygonFromLineSegments(unorderedLineSegmentsPerLevel[i], polygonsForThisLevel, 1.0e-8);
                    for (size_t j = 0; j < polygonsForThisLevel.size(); ++j)
                    {
                        double signedArea = cvf::GeometryTools::signedAreaPlanarPolygon(cvf::Vec3d::Z_AXIS, polygonsForThisLevel[j]);
                        ContourPolygon contourPolygon;
                        contourPolygon.value = contourLevels[i];
                        if (signedArea < 0.0)
                        {
                            contourPolygon.vertices.insert(contourPolygon.vertices.end(), polygonsForThisLevel[j].rbegin(), polygonsForThisLevel[j].rend());
                        }
                        else
                        {
                            contourPolygon.vertices = polygonsForThisLevel[j];
                        }

                        contourPolygon.area = cvf::GeometryTools::signedAreaPlanarPolygon(cvf::Vec3d::Z_AXIS, contourPolygon.vertices);
                        if (contourPolygon.area > areaTreshold)
                        {
                            for (const cvf::Vec3d& vertex : contourPolygon.vertices)
                            {
                                contourPolygon.bbox.add(vertex);
                            }
                            contourPolygons[i].push_back(contourPolygon);
                        }
                    }
                }
                for (size_t i = 0; i < contourPolygons.size(); ++i)
                {
                    if (i == 0 || m_smoothContourLines())
                    {
                        const ContourPolygons* clipBy = i > 0 ? &contourPolygons[i - 1] : nullptr;
                        smoothContourPolygons(&contourPolygons[i], clipBy, true);
                    }
                }

                double simplifyEpsilon = m_smoothContourLines() ? 5.0e-2 * m_sampleSpacing : 1.0e-3 * m_sampleSpacing;
                for (size_t i = 0; i < contourPolygons.size(); ++i)
                {
                    for (ContourPolygon& polygon : contourPolygons[i])
                    {
                        RigCellGeometryTools::simplifyPolygon(&polygon.vertices, simplifyEpsilon);
                    }
                }
                m_contourLevelCumulativeAreas.resize(contourPolygons.size(), 0.0);
                for (int64_t i = (int64_t) contourPolygons.size() - 1; i >= 0; --i)
                {
                    double levelOuterArea = sumPolygonArea(contourPolygons[i]);
                    m_contourLevelCumulativeAreas[i] = levelOuterArea;
                }
            }
        }
    }
    m_contourPolygons = contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::smoothContourPolygons(ContourPolygons*       contourPolygons,
                                                    const ContourPolygons* clipBy,
                                                    bool                   favourExpansion)
{
    CVF_ASSERT(contourPolygons);
    for (size_t i = 0; i < contourPolygons->size(); ++i)
    {
        ContourPolygon& polygon = contourPolygons->at(i);

        for (size_t n = 0; n < 20; ++n)
        {
            std::vector<cvf::Vec3d> newVertices;
            newVertices.resize(polygon.vertices.size());
            double maxChange = 0.0;
            for (size_t j = 0; j < polygon.vertices.size(); ++j)
            {
                cvf::Vec3d vm1 = polygon.vertices.back();
                cvf::Vec3d v   = polygon.vertices[j];
                cvf::Vec3d vp1 = polygon.vertices.front();
                if (j > 0u)
                {
                    vm1 = polygon.vertices[j - 1];
                }
                if (j < polygon.vertices.size() - 1)
                {
                    vp1 = polygon.vertices[j + 1];
                }
                // Only expand.
                cvf::Vec3d modifiedVertex = 0.5 * (v + 0.5 * (vm1 + vp1));
                cvf::Vec3d delta          = modifiedVertex - v;
                cvf::Vec3d tangent3d      = vp1 - vm1;
                cvf::Vec2d tangent2d(tangent3d.x(), tangent3d.y());
                cvf::Vec3d norm3d(tangent2d.getNormalized().perpendicularVector());
                if (delta * norm3d > 0 && favourExpansion)
                {
                    // Normal is always inwards facing so a positive dot product means inward movement
                    // Favour expansion rather than contraction by only contracting by a fraction.
                    // The fraction is empirically found to give a decent result.
                    modifiedVertex = v + 0.2 * delta;
                }
                newVertices[j] = modifiedVertex;
                maxChange      = std::max(maxChange, (modifiedVertex - v).length());
            }
            polygon.vertices.swap(newVertices);
            if (maxChange < m_sampleSpacing * 1.0e-2) break;
        }
        if (clipBy)
        {
            for (size_t j = 0; j < clipBy->size(); ++j)
            {
                std::vector<std::vector<cvf::Vec3d>> intersections =
                    RigCellGeometryTools::intersectPolygons(polygon.vertices, clipBy->at(j).vertices);
                if (!intersections.empty())
                {
                    polygon.vertices = intersections.front();
                    polygon.area     = std::abs(cvf::GeometryTools::signedAreaPlanarPolygon(cvf::Vec3d::Z_AXIS, polygon.vertices));
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumPolygonArea(const ContourPolygons& contourPolygons)
{
    double sumArea = 0.0;
    for (const ContourPolygon& polygon : contourPolygons)
    {
        sumArea += polygon.area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumTriangleAreas(const std::vector<cvf::Vec4d>& triangles)
{
    double sumArea = 0.0;
    for (size_t i = 0; i < triangles.size(); i += 3)
    {
        cvf::Vec3d v1(triangles[i].x(), triangles[i].y(), triangles[i].z());
        cvf::Vec3d v2(triangles[i + 1].x(), triangles[i + 1].y(), triangles[i + 1].z());
        cvf::Vec3d v3(triangles[i + 2].x(), triangles[i + 2].y(), triangles[i + 2].z());
        double area = 0.5 * ((v3 - v1) ^ (v2 - v1)).length();
        sumArea += area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>>
    RimContourMapProjection::cellOverlapVolumesAndResults(const cvf::Vec2d&          globalPos2d,
                                                                 const std::vector<double>& weightingResultValues) const
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

    std::vector<size_t> allCellIndices = findIntersectingCells(bbox2dElement);

    typedef std::map<size_t, std::vector<std::pair<size_t, double>>> KLayerCellWeightMap;
    KLayerCellWeightMap                                              matchingVisibleCellsWeightPerKLayer;

    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            size_t cellKLayer = 0u;
            double overlapVolume = calculateOverlapVolume(globalCellIdx, bbox2dElement, &cellKLayer);

            if (overlapVolume > 0.0)
            {
                double weight = overlapVolume * getParameterWeightForCell(globalCellIdx, weightingResultValues);

                if (weight > 0.0)
                {
                    matchingVisibleCellsWeightPerKLayer[cellKLayer].push_back(std::make_pair(globalCellIdx, weight));
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
std::vector<std::pair<size_t, double>>
    RimContourMapProjection::cellRayIntersectionAndResults(const cvf::Vec2d&          globalPos2d,
                                                                  const std::vector<double>& weightingResultValues) const
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

    std::vector<size_t> allCellIndices = findIntersectingCells(rayBBox);

    std::map<size_t, std::vector<std::pair<size_t, double>>> matchingVisibleCellsAndWeightPerKLayer;

    for (size_t globalCellIdx : allCellIndices)
    {
        if ((*m_cellGridIdxVisibility)[globalCellIdx])
        {
            size_t cellKLayer = 0u;
            double lengthInCell = calculateRayLengthInCell(globalCellIdx, highestPoint, lowestPoint, &cellKLayer);
            if (lengthInCell > 0.0)
            {
                matchingVisibleCellsAndWeightPerKLayer[cellKLayer].push_back(std::make_pair(globalCellIdx, lengthInCell));
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
bool RimContourMapProjection::isMeanResult() const
{
    return m_resultAggregation() == RESULTS_MEAN_VALUE || m_resultAggregation() == RESULTS_HARM_VALUE ||
           m_resultAggregation() == RESULTS_GEOM_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isSummationResult() const
{
    return isStraightSummationResult() || m_resultAggregation() == RESULTS_VOLUME_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isStraightSummationResult() const
{
    return isStraightSummationResult(m_resultAggregation());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isStraightSummationResult(ResultAggregationEnum aggregationType)
{
    return aggregationType == RESULTS_OIL_COLUMN || aggregationType == RESULTS_GAS_COLUMN ||
           aggregationType == RESULTS_HC_COLUMN || aggregationType == RESULTS_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::interpolateValue(const cvf::Vec2d& gridPos2d) const
{
    cvf::Vec2ui cellContainingPoint = ijFromLocalPos(gridPos2d);
    cvf::Vec2d  cellCenter          = cellCenterPosition(cellContainingPoint.x(), cellContainingPoint.y());

    std::array<cvf::Vec3d, 4> x;
    x[0] = cvf::Vec3d(cellCenter + cvf::Vec2d(-m_sampleSpacing * 0.5, -m_sampleSpacing * 0.5), 0.0);
    x[1] = cvf::Vec3d(cellCenter + cvf::Vec2d(m_sampleSpacing * 0.5, -m_sampleSpacing * 0.5), 0.0);
    x[2] = cvf::Vec3d(cellCenter + cvf::Vec2d(m_sampleSpacing * 0.5, m_sampleSpacing * 0.5), 0.0);
    x[3] = cvf::Vec3d(cellCenter + cvf::Vec2d(-m_sampleSpacing * 0.5, m_sampleSpacing * 0.5), 0.0);

    cvf::Vec4d baryCentricCoords = cvf::GeometryTools::barycentricCoords(x[0], x[1], x[2], x[3], cvf::Vec3d(gridPos2d, 0.0));

    std::array<cvf::Vec2ui, 4> v;
    v[0] = cellContainingPoint;
    v[1] = cvf::Vec2ui(cellContainingPoint.x() + 1u, cellContainingPoint.y());
    v[2] = cvf::Vec2ui(cellContainingPoint.x() + 1u, cellContainingPoint.y() + 1u);
    v[3] = cvf::Vec2ui(cellContainingPoint.x(), cellContainingPoint.y() + 1u);

    std::array<double, 4> vertexValues;
    double                validBarycentricCoordsSum = 0.0;
    for (int i = 0; i < 4; ++i)
    {
        double vertexValue = valueAtVertex(v[i].x(), v[i].y());
        if (vertexValue == std::numeric_limits<double>::infinity())
        {
            baryCentricCoords[i] = 0.0;
            vertexValues[i]      = 0.0;
            return std::numeric_limits<double>::infinity();
        }
        else
        {
            vertexValues[i] = vertexValue;
            validBarycentricCoordsSum += baryCentricCoords[i];
        }
    }

    if (validBarycentricCoordsSum < 1.0e-8)
    {
        return std::numeric_limits<double>::infinity();
    }

    // Calculate final value
    double value = 0.0;
    for (int i = 0; i < 4; ++i)
    {
        value += baryCentricCoords[i] / validBarycentricCoordsSum * vertexValues[i];
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::valueInCell(uint i, uint j) const
{
    size_t index = cellIndexFromIJ(i, j);
    if (index < numberOfCells())
    {
        return m_aggregatedResults.at(index);
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::hasResultInCell(uint i, uint j) const
{    
    return !cellsAtIJ(i, j).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValueAtVertex(uint vi, uint vj) const
{
    std::vector<uint> averageIs;
    std::vector<uint> averageJs;

    if (vi > 0u) averageIs.push_back(vi - 1);
    if (vj > 0u) averageJs.push_back(vj - 1);
    if (vi < m_mapSize.x()) averageIs.push_back(vi);
    if (vj < m_mapSize.y()) averageJs.push_back(vj);

    RiaWeightedMeanCalculator<double> calc;
    for (uint j : averageJs)
    {
        for (uint i : averageIs)
        {
            if (hasResultInCell(i, j))
            {
                calc.addValueAndWeight(valueInCell(i, j), 1.0);
            }
        }
    }
    if (calc.validAggregatedWeight())
    {
        return calc.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RimContourMapProjection::cellsAtIJ(uint i, uint j) const
{
    size_t cellIndex = this->cellIndexFromIJ(i, j);
    if (cellIndex < m_projected3dGridIndices.size())
    {
        return m_projected3dGridIndices[cellIndex];
    }
    return std::vector<std::pair<size_t, double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::cellIndexFromIJ(uint i, uint j) const
{
    CVF_ASSERT(i < m_mapSize.x());
    CVF_ASSERT(j < m_mapSize.y());

    return i + j * m_mapSize.x();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::vertexIndexFromIJ(uint i, uint j) const
{
    return i + j * (m_mapSize.x() + 1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromVertexIndex(size_t gridIndex) const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();

    uint quotientX  = static_cast<uint>(gridIndex) / gridSize.x();
    uint remainderX = static_cast<uint>(gridIndex) % gridSize.x();

    return cvf::Vec2ui(remainderX, quotientX);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromCellIndex(size_t cellIndex) const
{
    CVF_TIGHT_ASSERT(cellIndex < numberOfCells());

    uint quotientX  = static_cast<uint>(cellIndex) / m_mapSize.x();
    uint remainderX = static_cast<uint>(cellIndex) % m_mapSize.x();

    return cvf::Vec2ui(remainderX, quotientX);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromLocalPos(const cvf::Vec2d& localPos2d) const
{
    uint i = localPos2d.x() / m_sampleSpacing;
    uint j = localPos2d.y() / m_sampleSpacing;
    return cvf::Vec2ui(i, j);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::cellCenterPosition(uint i, uint j) const
{
    cvf::Vec3d gridExtent = m_expandedBoundingBox.extent();
    cvf::Vec2d cellCorner = cvf::Vec2d((i * gridExtent.x()) / (m_mapSize.x()), (j * gridExtent.y()) / (m_mapSize.y()));

    return cellCorner + cvf::Vec2d(m_sampleSpacing * 0.5, m_sampleSpacing * 0.5);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::origin2d() const
{
    return cvf::Vec2d(m_expandedBoundingBox.min().x(), m_expandedBoundingBox.min().y());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::xVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().x();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve(gridSize.x());
    for (uint i = 0; i < gridSize.x(); ++i)
    {
        positions.push_back((i * gridExtent) / (gridSize.x() - 1));
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::yVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().y();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve(gridSize.y());
    for (uint j = 0; j < gridSize.y(); ++j)
    {
        positions.push_back((j * gridExtent) / (gridSize.y() - 1));
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::use2dMapLegendRange() const
{
    return !use3dGridLegendRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::use3dGridLegendRange() const
{
    return (isMeanResult() || m_resultAggregation == RESULTS_TOP_VALUE);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::calculateMapSize() const
{
    cvf::Vec3d gridExtent = m_expandedBoundingBox.extent();

    uint projectionSizeX = static_cast<uint>(std::ceil(gridExtent.x() / m_sampleSpacing));
    uint projectionSizeY = static_cast<uint>(std::ceil(gridExtent.y() / m_sampleSpacing));

    return cvf::Vec2ui(projectionSizeX, projectionSizeY);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::gridEdgeOffset() const
{
    return m_sampleSpacing * 2.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue)
{
    legendConfig()->disableAllTimeStepsRange(use2dMapLegendRange());

    if (changedField == &m_resultAggregation)
    {
        ResultAggregation previousAggregation = static_cast<ResultAggregationEnum>(oldValue.toInt());
        if (isStraightSummationResult(previousAggregation) != isStraightSummationResult())
        {
            clearGridMapping();
        }
        else
        {
            clearResults();
        }
    }
    else if (changedField == &m_smoothContourLines)
    {
        clearGeometry();
    }
    else if (changedField == &m_relativeSampleSpacing)
    {
        clearResults();
    }

    baseView()->updateConnectedEditors();

    RimProject* proj;
    firstAncestorOrThisOfTypeAsserted(proj);
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                    QString                    uiConfigName,
                                                    caf::PdmUiEditorAttribute* attribute)
{
    if (&m_relativeSampleSpacing == field)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0.2;
            myAttr->m_maximum = 2.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* mainGroup = uiOrdering.addNewGroup("Projection Settings");
    mainGroup->add(&m_relativeSampleSpacing);
    mainGroup->add(&m_showContourLines);
    mainGroup->add(&m_showContourLabels);
    m_showContourLabels.uiCapability()->setUiReadOnly(!m_showContourLines());
    mainGroup->add(&m_smoothContourLines);
    m_smoothContourLines.uiCapability()->setUiReadOnly(!m_showContourLines());
    mainGroup->add(&m_resultAggregation);

    uiOrdering.skipRemainingFields(true);
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
    legendConfig()->disableAllTimeStepsRange(use2dMapLegendRange());
}
