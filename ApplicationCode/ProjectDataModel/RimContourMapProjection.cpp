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
#include "RimContourMapView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"

#include "cafContourLines.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfGeometryTools.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"

#include <algorithm>

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
}
CAF_PDM_SOURCE_INIT(RimContourMapProjection, "RimContourMapProjection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::RimContourMapProjection()
    : m_pickPoint(cvf::Vec2d::UNDEFINED),
      m_mapSize(cvf::Vec2ui(0u, 0u)),
      m_sampleSpacing(-1.0)
{
    CAF_PDM_InitObject("RimContourMapProjection", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_relativeSampleSpacing, "SampleSpacing", 0.75, "Sample Spacing Factor", "", "", "");
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_resultAggregation, "ResultAggregation", "Result Aggregation", "", "", "");

    CAF_PDM_InitField(&m_showContourLines, "ContourLines", true, "Show Contour Lines", "", "", "");

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
RimContourMapProjection::~RimContourMapProjection()
{

}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform)
{
    CVF_ASSERT(vertices);
    size_t nVertices = numberOfVertices();
    vertices->resize(nVertices);

#pragma omp parallel for
    for (int index = 0; index < static_cast<int>(nVertices); ++index)
    {
        cvf::Vec2ui ij = ijFromVertexIndex(index);
        cvf::Vec2d globalPos = globalCellCenterPosition(ij.x(), ij.y());
        // Shift away from sample point to vertex
        globalPos.x() -= m_sampleSpacing * 0.5;
        globalPos.y() -= m_sampleSpacing * 0.5;

        cvf::Vec3d globalVertexPos(globalPos, m_fullBoundingBox.min().z() - 1.0);
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
    if (minValue() != std::numeric_limits<double>::infinity() &&
        maxValue() != -std::numeric_limits<double>::infinity() &&
        std::fabs(maxValue() - minValue()) > 1.0e-8)
    {
        std::vector<double> contourLevels;
        if (legendConfig()->mappingMode() != RimRegularLegendConfig::CATEGORY_INTEGER)
        {
            legendConfig()->scalarMapper()->majorTickValues(&contourLevels);
            int nContourLevels = static_cast<int>(contourLevels.size());
            if (nContourLevels > 2)
            {
                if (legendConfig()->mappingMode() == RimRegularLegendConfig::LINEAR_CONTINUOUS || legendConfig()->mappingMode() == RimRegularLegendConfig::LINEAR_DISCRETE)
                {
                    // Slight fudge to avoid very jagged contour lines at the very edge
                    // Shift the contour levels inwards.
                    contourLevels[0] += (contourLevels[1] - contourLevels[0]) * 0.1;
                    contourLevels[nContourLevels - 1] -= (contourLevels[nContourLevels - 1] - contourLevels[nContourLevels - 2]) * 0.1;
                }
                std::vector<std::vector<cvf::Vec2d>> contourLines;
                caf::ContourLines::create(m_aggregatedVertexResults, xVertexPositions(), yVertexPositions(), contourLevels, &contourLines);

                contourPolygons.reserve(contourLines.size());
                for (size_t i = 0; i < contourLines.size(); ++i)
                {
                    if (!contourLines[i].empty())
                    {
                        cvf::ref<cvf::Vec3fArray> contourPolygon = new cvf::Vec3fArray(contourLines[i].size());
                        for (size_t j = 0; j < contourLines[i].size(); ++j)
                        {
                            cvf::Vec3d contourPoint3d = cvf::Vec3d(contourLines[i][j], m_fullBoundingBox.min().z());
                            cvf::Vec3d displayPoint3d = displayCoordTransform->transformToDisplayCoord(contourPoint3d);
                            (*contourPolygon)[j] = cvf::Vec3f(displayPoint3d);
                        }
                        contourPolygons.push_back(contourPolygon);
                    }
                }
            }
        }
    }
    return contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec3fArray>
RimContourMapProjection::generatePickPointPolygon(const caf::DisplayCoordTransform* displayCoordTransform)
{
    cvf::ref<cvf::Vec3fArray> pickPolygon;
    if (!m_pickPoint.isUndefined())
    {
        double zPos = m_fullBoundingBox.min().z();

        std::vector<cvf::Vec3d> points;
        {
            cvf::Vec2d gridorigin(m_fullBoundingBox.min().x(), m_fullBoundingBox.min().y());

            cvf::Vec2d localPickPoint = m_pickPoint - gridorigin;

            cvf::Vec2d  cellDiagonal(m_sampleSpacing*0.5, m_sampleSpacing*0.5);
            cvf::Vec2ui pickedCell = ijFromLocalPos(localPickPoint);
            cvf::Vec2d  cellCenter = globalCellCenterPosition(pickedCell.x(), pickedCell.y());
            cvf::Vec2d  cellCorner = cellCenter - cellDiagonal;
#ifndef NDEBUG
            points.push_back(cvf::Vec3d(cellCorner, zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, 0.0), zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, 0.0), zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, m_sampleSpacing), zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(m_sampleSpacing, m_sampleSpacing), zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(0.0, m_sampleSpacing), zPos));
            points.push_back(cvf::Vec3d(cellCorner + cvf::Vec2d(0.0, m_sampleSpacing), zPos));
            points.push_back(cvf::Vec3d(cellCorner, zPos));            
#endif
            points.push_back(cvf::Vec3d(m_pickPoint - cvf::Vec2d(0.5 * m_sampleSpacing, 0.0), zPos));
            points.push_back(cvf::Vec3d(m_pickPoint + cvf::Vec2d(0.5 * m_sampleSpacing, 0.0), zPos));
            points.push_back(cvf::Vec3d(m_pickPoint - cvf::Vec2d(0.0, 0.5 * m_sampleSpacing), zPos));
            points.push_back(cvf::Vec3d(m_pickPoint + cvf::Vec2d(0.0, 0.5 * m_sampleSpacing), zPos));
        }

        pickPolygon = new cvf::Vec3fArray(points.size());

        for (size_t i = 0; i < points.size(); ++i)
        {
            cvf::Vec3d displayPoint = displayCoordTransform->transformToDisplayCoord(points[i]);
            (*pickPolygon)[i] = cvf::Vec3f(displayPoint);
        }
    }
    return pickPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateResults()
{
    updateGridInformation();

    generateGridMapping();

    size_t nCells = numberOfCells();
    size_t nVertices = numberOfVertices();
    
    m_aggregatedResults = std::vector<double>(nCells, std::numeric_limits<double>::infinity());
    m_aggregatedVertexResults = std::vector<double>(nVertices, std::numeric_limits<double>::infinity());
    int timeStep = view()->currentTimeStep();
    RimEclipseCellColors* cellColors = view()->cellResult();

    RimEclipseResultCase* eclipseCase = this->eclipseCase();
    {
        if (!cellColors->isTernarySaturationSelected())
        {
            RigCaseCellResultsData* resultData = eclipseCase->results(RiaDefines::MATRIX_MODEL);

            if (isColumnResult())
            {                
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
                m_resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(), 0, timeStep, cellColors);

                if (m_resultAccessor.isNull())
                {
                    m_resultAccessor = new RigHugeValResultAccessor;
                }
            }

#pragma omp parallel for
            for (int index = 0; index < static_cast<int>(nCells); ++index)
            {
                cvf::Vec2ui ij = ijFromCellIndex(index);
                m_aggregatedResults[index] = calculateValueInCell(ij.x(), ij.y());
            }

#pragma omp parallel for
            for (int index = 0; index < static_cast<int>(nVertices); ++index)
            {
                cvf::Vec2ui ij = ijFromVertexIndex(index);
                m_aggregatedVertexResults[index] = calculateValueAtVertex(ij.x(), ij.y());
            }
        }
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
QString RimContourMapProjection::resultAggregationText() const
{
    return m_resultAggregation().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapProjection::resultDescriptionText() const
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
QString RimContourMapProjection::weightingParameter() const
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
double RimContourMapProjection::maxValue() const
{
    double maxV = -std::numeric_limits<double>::infinity();

    int nVertices = numberOfCells();

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

    int nVertices = numberOfCells();

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
    return sumAllValues() / numberOfValidCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumAllValues() const
{
    double sum = 0.0;

    int nVertices = numberOfCells();

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
    return m_resultAggregation() == RESULTS_OIL_COLUMN ||
           m_resultAggregation() == RESULTS_GAS_COLUMN ||
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
RimRegularLegendConfig* RimContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::updateLegend()
{    
    RimEclipseCellColors* cellColors = view()->cellResult();

    if (getLegendRangeFrom3dGrid())
    {
        cellColors->updateLegendData(view()->currentTimeStep(), legendConfig());
    }
    else
    {
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
void RimContourMapProjection::updatedWeightingResult()
{
    this->updateConnectedEditors();
    this->generateResults();
    this->updateLegend();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::checkForMapIntersection(const cvf::Vec3d& localPoint3d, cvf::Vec2d* contourMapPoint, cvf::Vec2ui* contourMapCell, double* valueAtPoint) const
{
    CVF_TIGHT_ASSERT(contourMapPoint);
    CVF_TIGHT_ASSERT(valueAtPoint);
    cvf::Vec3d localPos3d(localPoint3d.x(), localPoint3d.y(), 0.0);
    cvf::Vec2d localPos2d(localPos3d.x(), localPos3d.y());
    cvf::Vec2ui pickedCell = ijFromLocalPos(localPos2d);
    *contourMapCell = pickedCell;
    
    if (true || hasResultInCell(pickedCell.x(), pickedCell.y()))
    {
        cvf::Vec2d gridorigin(m_fullBoundingBox.min().x(), m_fullBoundingBox.min().y());
        cvf::Vec2d cellCenter = globalCellCenterPosition(pickedCell.x(), pickedCell.y()) - gridorigin;
        std::array <cvf::Vec3d, 4> x;
        x[0] = cvf::Vec3d(cellCenter + cvf::Vec2d(-m_sampleSpacing * 0.5, -m_sampleSpacing * 0.5), 0.0);
        x[1] = cvf::Vec3d(cellCenter + cvf::Vec2d(m_sampleSpacing*0.5, -m_sampleSpacing * 0.5), 0.0);
        x[2] = cvf::Vec3d(cellCenter + cvf::Vec2d(m_sampleSpacing*0.5, m_sampleSpacing * 0.5), 0.0);
        x[3] = cvf::Vec3d(cellCenter + cvf::Vec2d(-m_sampleSpacing * 0.5, m_sampleSpacing * 0.5), 0.0);
        cvf::Vec4d baryCentricCoords = cvf::GeometryTools::barycentricCoords(x[0], x[1], x[2], x[3], localPos3d);

        std::array<cvf::Vec2ui, 4> v;
        v[0] = pickedCell;
        v[1] = cvf::Vec2ui(pickedCell.x() + 1u, pickedCell.y());
        v[2] = cvf::Vec2ui(pickedCell.x() + 1u, pickedCell.y() + 1u);
        v[3] = cvf::Vec2ui(pickedCell.x(), pickedCell.y() + 1u);

        double value = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            value += baryCentricCoords[i] * valueAtVertex(v[i].x(), v[i].y());
        }

        *valueAtPoint = value;
        *contourMapPoint = localPos2d + gridorigin;

        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::setPickPoint(cvf::Vec2d pickedPoint)
{
    m_pickPoint = pickedPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue)
{
    legendConfig()->disableAllTimeStepsRange(!getLegendRangeFrom3dGrid());

    m_weightingResult->loadResult();

    view()->updateConnectedEditors();

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
            myAttr->m_minimum = 0.25;
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
    mainGroup->add(&m_resultAggregation);

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
void RimContourMapProjection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::initAfterRead()
{
    legendConfig()->disableAllTimeStepsRange(!getLegendRangeFrom3dGrid());
    if (eclipseCase())
    {
        m_weightingResult->setEclipseCase(eclipseCase());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateGridMapping()
{
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
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos            = globalCellCenterPosition(ij.x(), ij.y());
            m_projected3dGridIndices[index] = visibleCellsAndLengthInCellFrom2dPoint(globalPos, weightingResultValues);
        }
    }
    else
    {
#pragma omp parallel for
        for (int index = 0; index < nCells; ++index)
        {
            cvf::Vec2ui ij = ijFromCellIndex(index);

            cvf::Vec2d globalPos            = globalCellCenterPosition(ij.x(), ij.y());
            m_projected3dGridIndices[index] = visibleCellsAndOverlapVolumeFrom2dPoint(globalPos, weightingResultValues);
        }
    }
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
    RimEclipseCellColors* cellColors = view()->cellResult();

    if (cellColors->isTernarySaturationSelected())
    {
        return false;
    }
    return !cellsAtIJ(i, j).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValueInCell(uint i, uint j) const
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
std::vector<std::pair<size_t, double>>
    RimContourMapProjection::visibleCellsAndOverlapVolumeFrom2dPoint(const cvf::Vec2d&          globalPos2d,
                                                                     const std::vector<double>* weightingResultValues) const
{
    cvf::Vec3d top2dElementCentroid(globalPos2d, m_fullBoundingBox.max().z());
    cvf::Vec3d bottom2dElementCentroid(globalPos2d, m_fullBoundingBox.min().z());
    cvf::Vec3d planarDiagonalVector(0.5 * m_sampleSpacing, 0.5 * m_sampleSpacing, 0.0);
    cvf::Vec3d topNECorner    = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement(bottomSWCorner, topNECorner);

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

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;
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
std::vector<std::pair<size_t, double>> RimContourMapProjection::visibleCellsAndLengthInCellFrom2dPoint(
    const cvf::Vec2d&          globalPos2d,
    const std::vector<double>* weightingResultValues /*= nullptr*/) const
{
    cvf::Vec3d highestPoint(globalPos2d, m_fullBoundingBox.max().z());
    cvf::Vec3d lowestPoint(globalPos2d, m_fullBoundingBox.min().z());

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

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;
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
double RimContourMapProjection::findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const
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
cvf::Vec2d RimContourMapProjection::globalCellCenterPosition(uint i, uint j) const
{
    cvf::Vec3d gridExtent = m_fullBoundingBox.extent();
    cvf::Vec2d origin(m_fullBoundingBox.min().x(), m_fullBoundingBox.min().y());

    cvf::Vec2d cellCorner = origin + cvf::Vec2d((i * gridExtent.x()) / (m_mapSize.x()), (j * gridExtent.y()) / (m_mapSize.y()));

    return cellCorner + cvf::Vec2d(m_sampleSpacing * 0.5, m_sampleSpacing * 0.5);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::xVertexPositions() const
{
    double gridExtent = m_fullBoundingBox.extent().x();
    double origin     = m_fullBoundingBox.min().x();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve(gridSize.x());
    for (uint i = 0; i < gridSize.x(); ++i)
    {
        positions.push_back(origin + (i * gridExtent) / (gridSize.x() - 1));
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::yVertexPositions() const
{
    double gridExtent = m_fullBoundingBox.extent().y();
    double origin     = m_fullBoundingBox.min().y();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve(gridSize.y());
    for (uint j = 0; j < gridSize.y(); ++j)
    {
        positions.push_back(origin + (j * gridExtent) / (gridSize.y() - 1));
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::getLegendRangeFrom3dGrid() const
{
    if (isMeanResult())
    {
        return true;
    }
    else if (m_resultAggregation == RESULTS_TOP_VALUE)
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::updateGridInformation()
{
    m_mainGrid        = eclipseCase()->eclipseCaseData()->mainGrid();
    m_sampleSpacing   = m_relativeSampleSpacing * m_mainGrid->characteristicIJCellSize();
    m_fullBoundingBox = eclipseCase()->activeCellsBoundingBox();
    m_mapSize         = calculateMapSize();

    // Re-jig max point to be an exact multiple of cell size
    cvf::Vec3d minPoint = m_fullBoundingBox.min();
    cvf::Vec3d maxPoint = m_fullBoundingBox.max();
    maxPoint.x() = minPoint.x() + m_mapSize.x() * m_sampleSpacing;
    maxPoint.y() = minPoint.y() + m_mapSize.y() * m_sampleSpacing;
    m_fullBoundingBox = cvf::BoundingBox(minPoint, maxPoint);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::calculateMapSize() const
{
    cvf::Vec3d       gridExtent  = m_fullBoundingBox.extent();

    uint projectionSizeX = static_cast<uint>(std::ceil(gridExtent.x() / m_sampleSpacing));
    uint projectionSizeY = static_cast<uint>(std::ceil(gridExtent.y() / m_sampleSpacing));

    return cvf::Vec2ui(projectionSizeX, projectionSizeY);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimContourMapProjection::eclipseCase() const
{
    RimEclipseResultCase* eclipseCase = nullptr;
    firstAncestorOrThisOfType(eclipseCase);
    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapView* RimContourMapProjection::view() const
{
    RimContourMapView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted(view);
    return view;
}
