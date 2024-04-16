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

#include "RiaOpenMPTools.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCellGeometryTools.h"
#include "RigHexIntersectionTools.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTextAnnotation.h"

#include "cafContourLines.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfGeometryTools.h"
#include "cvfGeometryUtils.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"

#include <algorithm>

namespace caf
{
template <>
void RimContourMapProjection::ResultAggregation::setUp()
{
    addItem( RimContourMapProjection::RESULTS_OIL_COLUMN, "OIL_COLUMN", "Oil Column" );
    addItem( RimContourMapProjection::RESULTS_GAS_COLUMN, "GAS_COLUMN", "Gas Column" );
    addItem( RimContourMapProjection::RESULTS_HC_COLUMN, "HC_COLUMN", "Hydrocarbon Column" );

    addItem( RimContourMapProjection::RESULTS_MEAN_VALUE, "MEAN_VALUE", "Arithmetic Mean" );
    addItem( RimContourMapProjection::RESULTS_HARM_VALUE, "HARM_VALUE", "Harmonic Mean" );
    addItem( RimContourMapProjection::RESULTS_GEOM_VALUE, "GEOM_VALUE", "Geometric Mean" );
    addItem( RimContourMapProjection::RESULTS_VOLUME_SUM, "VOLUME_SUM", "Volume Weighted Sum" );
    addItem( RimContourMapProjection::RESULTS_SUM, "SUM", "Sum" );

    addItem( RimContourMapProjection::RESULTS_TOP_VALUE, "TOP_VALUE", "Top  Value" );
    addItem( RimContourMapProjection::RESULTS_MIN_VALUE, "MIN_VALUE", "Min Value" );
    addItem( RimContourMapProjection::RESULTS_MAX_VALUE, "MAX_VALUE", "Max Value" );

    setDefault( RimContourMapProjection::RESULTS_MEAN_VALUE );
}
} // namespace caf
CAF_PDM_ABSTRACT_SOURCE_INIT( RimContourMapProjection, "RimContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::RimContourMapProjection()
    : m_pickPoint( cvf::Vec2d::UNDEFINED )
    , m_mapSize( cvf::Vec2ui( 0u, 0u ) )
    , m_currentResultTimestep( -1 )
    , m_minResultAllTimeSteps( std::numeric_limits<double>::infinity() )
    , m_maxResultAllTimeSteps( -std::numeric_limits<double>::infinity() )
{
    CAF_PDM_InitObject( "RimContourMapProjection", ":/2DMapProjection16x16.png" );

    CAF_PDM_InitField( &m_relativeSampleSpacing, "SampleSpacing", 0.9, "Sample Spacing Factor" );
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_resultAggregation, "ResultAggregation", "Result Aggregation" );

    CAF_PDM_InitField( &m_showContourLines, "ContourLines", true, "Show Contour Lines" );
    CAF_PDM_InitField( &m_showContourLabels, "ContourLabels", true, "Show Contour Labels" );
    CAF_PDM_InitField( &m_smoothContourLines, "SmoothContourLines", true, "Smooth Contour Lines" );

    setName( "Map Projection" );
    nameField()->uiCapability()->setUiReadOnly( true );
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
void RimContourMapProjection::generateResultsIfNecessary( int timeStep )
{
    caf::ProgressInfo progress( 100, "Generate Results", true );

    updateGridInformation();
    progress.setProgress( 10 );

    if ( gridMappingNeedsUpdating() || mapCellVisibilityNeedsUpdating() || resultVariableChanged() )
    {
        clearResults();
        clearTimeStepRange();

        if ( gridMappingNeedsUpdating() ) m_projected3dGridIndices = generateGridMapping();
        progress.setProgress( 20 );
        m_mapCellVisibility = getMapCellVisibility();
        progress.setProgress( 30 );
    }
    else
    {
        progress.setProgress( 30 );
    }

    if ( resultsNeedsUpdating( timeStep ) )
    {
        clearGeometry();
        m_aggregatedResults = generateResults( timeStep );
        progress.setProgress( 80 );
        generateVertexResults();
    }
    progress.setProgress( 100 );
    updateAfterResultGeneration( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateGeometryIfNecessary()
{
    caf::ProgressInfo progress( 100, "Generate Geometry", true );

    if ( geometryNeedsUpdating() )
    {
        generateContourPolygons();
        progress.setProgress( 25 );
        generateTrianglesWithVertexValues();
    }
    progress.setProgress( 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimContourMapProjection::generatePickPointPolygon()
{
    std::vector<cvf::Vec3d> points;

    if ( !m_pickPoint.isUndefined() )
    {
        {
#ifndef NDEBUG
            cvf::Vec2d  cellDiagonal( sampleSpacing() * 0.5, sampleSpacing() * 0.5 );
            cvf::Vec2ui pickedCell = ijFromLocalPos( m_pickPoint );
            cvf::Vec2d  cellCenter = cellCenterPosition( pickedCell.x(), pickedCell.y() );
            cvf::Vec2d  cellCorner = cellCenter - cellDiagonal;
            points.push_back( cvf::Vec3d( cellCorner, 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing(), 0.0 ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing(), 0.0 ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing(), sampleSpacing() ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing(), sampleSpacing() ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( 0.0, sampleSpacing() ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( 0.0, sampleSpacing() ), 0.0 ) );
            points.push_back( cvf::Vec3d( cellCorner, 0.0 ) );
#endif
            points.push_back( cvf::Vec3d( m_pickPoint - cvf::Vec2d( 0.5 * sampleSpacing(), 0.0 ), 0.0 ) );
            points.push_back( cvf::Vec3d( m_pickPoint + cvf::Vec2d( 0.5 * sampleSpacing(), 0.0 ), 0.0 ) );
            points.push_back( cvf::Vec3d( m_pickPoint - cvf::Vec2d( 0.0, 0.5 * sampleSpacing() ), 0.0 ) );
            points.push_back( cvf::Vec3d( m_pickPoint + cvf::Vec2d( 0.0, 0.5 * sampleSpacing() ), 0.0 ) );
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
double RimContourMapProjection::sampleSpacingFactor() const
{
    return m_relativeSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::setSampleSpacingFactor( double spacingFactor )
{
    m_relativeSampleSpacing = spacingFactor;
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
    return m_showContourLabels();
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
QString RimContourMapProjection::caseName() const
{
    RimCase* rimCase = baseView()->ownerCase();
    if ( !rimCase )
    {
        return QString();
    }

    return rimCase->caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapProjection::currentTimeStepName() const
{
    RimCase* rimCase = baseView()->ownerCase();
    if ( !rimCase || m_currentResultTimestep == -1 )
    {
        return QString();
    }

    return rimCase->timeStepName( m_currentResultTimestep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::maxValue() const
{
    return maxValue( m_aggregatedResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::minValue() const
{
    return minValue( m_aggregatedResults );
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

    for ( size_t index = 0; index < m_aggregatedResults.size(); ++index )
    {
        if ( m_aggregatedResults[index] != std::numeric_limits<double>::infinity() )
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
    cvf::Vec2ui mapSize = numberOfElementsIJ();
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
double RimContourMapProjection::valueAtVertex( uint i, uint j ) const
{
    size_t index = vertexIndexFromIJ( i, j );
    if ( index < numberOfVertices() )
    {
        return m_aggregatedVertexResults.at( index );
    }
    return std::numeric_limits<double>::infinity();
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
    for ( uint i = 0; i < numberOfCells(); ++i )
    {
        cvf::Vec2ui ij = ijFromCellIndex( i );
        if ( hasResultInCell( ij.x(), ij.y() ) )
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
    return static_cast<size_t>( gridSize.x() ) * static_cast<size_t>( gridSize.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::checkForMapIntersection( const cvf::Vec3d& domainPoint3d, cvf::Vec2d* contourMapPoint, double* valueAtPoint ) const
{
    CVF_TIGHT_ASSERT( contourMapPoint );
    CVF_TIGHT_ASSERT( valueAtPoint );

    cvf::Vec3d mapPos3d = domainPoint3d - m_expandedBoundingBox.min();
    cvf::Vec2d mapPos2d( mapPos3d.x(), mapPos3d.y() );
    cvf::Vec2d gridorigin( m_expandedBoundingBox.min().x(), m_expandedBoundingBox.min().y() );

    double value = interpolateValue( mapPos2d );
    if ( value != std::numeric_limits<double>::infinity() )
    {
        *valueAtPoint    = value;
        *contourMapPoint = mapPos2d + gridorigin;

        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::setPickPoint( cvf::Vec2d globalPickPoint )
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
size_t RimContourMapProjection::gridResultIndex( size_t globalCellIdx ) const
{
    return globalCellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValueInMapCell( uint i, uint j, const std::vector<double>& gridCellValues ) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtIJ( i, j );
    if ( !matchingCells.empty() )
    {
        switch ( m_resultAggregation() )
        {
            case RESULTS_TOP_VALUE:
            {
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        return cellValue;
                    }
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_MEAN_VALUE:
            {
                RiaWeightedMeanCalculator<double> calculator;
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        calculator.addValueAndWeight( cellValue, weight );
                    }
                }
                if ( calculator.validAggregatedWeight() )
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_GEOM_VALUE:
            {
                RiaWeightedGeometricMeanCalculator calculator;
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        if ( cellValue < 1.0e-8 )
                        {
                            return 0.0;
                        }
                        calculator.addValueAndWeight( cellValue, weight );
                    }
                }
                if ( calculator.validAggregatedWeight() )
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_HARM_VALUE:
            {
                RiaWeightedHarmonicMeanCalculator calculator;
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::fabs( cellValue ) < 1.0e-8 )
                    {
                        return 0.0;
                    }
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        calculator.addValueAndWeight( cellValue, weight );
                    }
                }
                if ( calculator.validAggregatedWeight() )
                {
                    return calculator.weightedMean();
                }
                return std::numeric_limits<double>::infinity();
            }
            case RESULTS_MAX_VALUE:
            {
                double maxValue = -std::numeric_limits<double>::infinity();
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        maxValue = std::max( maxValue, cellValue );
                    }
                }
                if ( maxValue == -std::numeric_limits<double>::infinity() )
                {
                    maxValue = std::numeric_limits<double>::infinity();
                }
                return maxValue;
            }
            case RESULTS_MIN_VALUE:
            {
                double minValue = std::numeric_limits<double>::infinity();
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    minValue         = std::min( minValue, cellValue );
                }
                return minValue;
            }
            case RESULTS_VOLUME_SUM:
            case RESULTS_SUM:
            case RESULTS_OIL_COLUMN:
            case RESULTS_GAS_COLUMN:
            case RESULTS_HC_COLUMN:
            {
                double sum = 0.0;
                for ( auto [cellIdx, weight] : matchingCells )
                {
                    double cellValue = gridCellValues[gridResultIndex( cellIdx )];
                    if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
                    {
                        sum += cellValue * weight;
                    }
                }
                return sum;
            }
            default:
                CVF_TIGHT_ASSERT( false );
        }
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::gridMappingNeedsUpdating() const
{
    if ( m_projected3dGridIndices.size() != numberOfCells() )
    {
        return true;
    }

    if ( m_cellGridIdxVisibility.isNull() )
    {
        return true;
    }
    cvf::ref<cvf::UByteArray> currentVisibility = getCellVisibility();

    CVF_ASSERT( currentVisibility->size() == m_cellGridIdxVisibility->size() );
    for ( size_t i = 0; i < currentVisibility->size(); ++i )
    {
        if ( ( *currentVisibility )[i] != ( *m_cellGridIdxVisibility )[i] ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::resultsNeedsUpdating( int timeStep ) const
{
    if ( m_aggregatedResults.size() != numberOfCells() )
    {
        return true;
    }

    if ( m_aggregatedVertexResults.size() != numberOfVertices() )
    {
        return true;
    }

    if ( timeStep != m_currentResultTimestep )
    {
        return true;
    }
    return false;
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
bool RimContourMapProjection::resultRangeIsValid() const
{
    return m_minResultAllTimeSteps != std::numeric_limits<double>::infinity() &&
           m_maxResultAllTimeSteps != -std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::clearGridMapping()
{
    clearResults();
    clearTimeStepRange();
    m_projected3dGridIndices.clear();
    m_mapCellVisibility.clear();
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
void RimContourMapProjection::clearTimeStepRange()
{
    m_minResultAllTimeSteps = std::numeric_limits<double>::infinity();
    m_maxResultAllTimeSteps = -std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::maxValue( const std::vector<double>& aggregatedResults ) const
{
    double maxV = -std::numeric_limits<double>::infinity();

    for ( size_t index = 0; index < aggregatedResults.size(); ++index )
    {
        if ( aggregatedResults[index] != std::numeric_limits<double>::infinity() )
        {
            maxV = std::max( maxV, aggregatedResults[index] );
        }
    }
    return maxV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::minValue( const std::vector<double>& aggregatedResults ) const
{
    double minV = std::numeric_limits<double>::infinity();

    for ( size_t index = 0; index < aggregatedResults.size(); ++index )
    {
        if ( aggregatedResults[index] != std::numeric_limits<double>::infinity() )
        {
            minV = std::min( minV, aggregatedResults[index] );
        }
    }
    return minV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimContourMapProjection::minmaxValuesAllTimeSteps()
{
    if ( !resultRangeIsValid() )
    {
        clearTimeStepRange();

        m_minResultAllTimeSteps = std::min( m_minResultAllTimeSteps, minValue( m_aggregatedResults ) );
        m_maxResultAllTimeSteps = std::max( m_maxResultAllTimeSteps, maxValue( m_aggregatedResults ) );

        for ( int i = 0; i < (int)baseView()->ownerCase()->timeStepStrings().size() - 1; ++i )
        {
            if ( i != m_currentResultTimestep )
            {
                std::vector<double> aggregatedResults = generateResults( i );
                m_minResultAllTimeSteps               = std::min( m_minResultAllTimeSteps, minValue( aggregatedResults ) );
                m_maxResultAllTimeSteps               = std::max( m_maxResultAllTimeSteps, maxValue( aggregatedResults ) );
            }
        }
    }
    return std::make_pair( m_minResultAllTimeSteps, m_maxResultAllTimeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimContourMapProjection::getCellVisibility() const
{
    return baseView()->currentTotalCellVisibility();
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
std::vector<bool> RimContourMapProjection::getMapCellVisibility()
{
    return std::vector<bool>( numberOfCells(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::mapCellVisibilityNeedsUpdating()
{
    std::vector<bool> mapCellVisiblity = getMapCellVisibility();
    return !( mapCellVisiblity == m_mapCellVisibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::pair<size_t, double>>> RimContourMapProjection::generateGridMapping()
{
    m_cellGridIdxVisibility = getCellVisibility();

    int                                                 nCells = numberOfCells();
    std::vector<std::vector<std::pair<size_t, double>>> projected3dGridIndices( nCells );

    std::vector<double> weightingResultValues = retrieveParameterWeights();

    if ( isStraightSummationResult() )
    {
#pragma omp parallel for
        for ( int index = 0; index < nCells; ++index )
        {
            cvf::Vec2ui ij = ijFromCellIndex( index );

            cvf::Vec2d globalPos          = cellCenterPosition( ij.x(), ij.y() ) + origin2d();
            projected3dGridIndices[index] = cellRayIntersectionAndResults( globalPos, weightingResultValues );
        }
    }
    else
    {
#pragma omp parallel for
        for ( int index = 0; index < nCells; ++index )
        {
            cvf::Vec2ui ij = ijFromCellIndex( index );

            cvf::Vec2d globalPos          = cellCenterPosition( ij.x(), ij.y() ) + origin2d();
            projected3dGridIndices[index] = cellOverlapVolumesAndResults( globalPos, weightingResultValues );
        }
    }

    return projected3dGridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateVertexResults()
{
    size_t nCells = numberOfCells();
    if ( nCells != m_aggregatedResults.size() ) return;

    size_t nVertices          = numberOfVertices();
    m_aggregatedVertexResults = std::vector<double>( nVertices, std::numeric_limits<double>::infinity() );
#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nVertices ); ++index )
    {
        cvf::Vec2ui ij                   = ijFromVertexIndex( index );
        m_aggregatedVertexResults[index] = calculateValueAtVertex( ij.x(), ij.y() );
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
    cvf::GeometryUtils::tesselatePatchAsTriangles( patchSize.x(), patchSize.y(), 0u, true, faceList.p() );

    bool                discrete = false;
    std::vector<double> contourLevels;
    if ( legendConfig()->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
    {
        legendConfig()->scalarMapper()->majorTickValues( &contourLevels );
        if ( legendConfig()->mappingMode() == RimRegularLegendConfig::MappingType::LINEAR_DISCRETE ||
             legendConfig()->mappingMode() == RimRegularLegendConfig::MappingType::LOG10_DISCRETE )
        {
            discrete = true;
        }
    }

    const double cellArea      = sampleSpacing() * sampleSpacing();
    const double areaThreshold = 1.0e-5 * 0.5 * cellArea;

    std::vector<std::vector<std::vector<cvf::Vec3d>>> subtractPolygons;
    if ( !m_contourPolygons.empty() )
    {
        subtractPolygons.resize( m_contourPolygons.size() );
        for ( size_t i = 0; i < m_contourPolygons.size() - 1; ++i )
        {
            for ( size_t j = 0; j < m_contourPolygons[i + 1].size(); ++j )
            {
                subtractPolygons[i].push_back( m_contourPolygons[i + 1][j].vertices );
            }
        }
    }

    int numberOfThreads = RiaOpenMPTools::availableThreadCount();

    std::vector<std::vector<std::vector<cvf::Vec4d>>> threadTriangles( numberOfThreads );

#pragma omp parallel
    {
        int myThread = RiaOpenMPTools::currentThreadIndex();
        threadTriangles[myThread].resize( std::max( (size_t)1, m_contourPolygons.size() ) );

#pragma omp for schedule( dynamic )
        for ( int64_t i = 0; i < (int64_t)faceList->size(); i += 3 )
        {
            std::vector<cvf::Vec3d> triangle( 3 );
            std::vector<cvf::Vec4d> triangleWithValues( 3 );
            bool                    anyValidVertex = false;
            for ( size_t n = 0; n < 3; ++n )
            {
                uint vn = ( *faceList )[i + n];
                double value = vn < m_aggregatedVertexResults.size() ? m_aggregatedVertexResults[vn] : std::numeric_limits<double>::infinity();
                triangle[n]           = vertices[vn];
                triangleWithValues[n] = cvf::Vec4d( vertices[vn], value );
                if ( value != std::numeric_limits<double>::infinity() )
                {
                    anyValidVertex = true;
                }
            }

            if ( !anyValidVertex )
            {
                continue;
            }

            if ( m_contourPolygons.empty() )
            {
                threadTriangles[myThread][0].insert( threadTriangles[myThread][0].end(), triangleWithValues.begin(), triangleWithValues.end() );
                continue;
            }

            bool outsideOuterLimit = false;
            for ( size_t c = 0; c < m_contourPolygons.size() && !outsideOuterLimit; ++c )
            {
                std::vector<std::vector<cvf::Vec3d>> intersectPolygons;
                for ( size_t j = 0; j < m_contourPolygons[c].size(); ++j )
                {
                    bool containsAtLeastOne = false;
                    for ( size_t t = 0; t < 3; ++t )
                    {
                        if ( m_contourPolygons[c][j].bbox.contains( triangle[t] ) )
                        {
                            containsAtLeastOne = true;
                        }
                    }
                    if ( containsAtLeastOne )
                    {
                        std::vector<std::vector<cvf::Vec3d>> clippedPolygons =
                            RigCellGeometryTools::intersectionWithPolygon( triangle, m_contourPolygons[c][j].vertices );
                        intersectPolygons.insert( intersectPolygons.end(), clippedPolygons.begin(), clippedPolygons.end() );
                    }
                }

                if ( intersectPolygons.empty() )
                {
                    outsideOuterLimit = true;
                    continue;
                }

                std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

                if ( !subtractPolygons[c].empty() )
                {
                    for ( const std::vector<cvf::Vec3d>& polygon : intersectPolygons )
                    {
                        std::vector<std::vector<cvf::Vec3d>> fullyClippedPolygons =
                            RigCellGeometryTools::subtractPolygons( polygon, subtractPolygons[c] );
                        clippedPolygons.insert( clippedPolygons.end(), fullyClippedPolygons.begin(), fullyClippedPolygons.end() );
                    }
                }
                else
                {
                    clippedPolygons.swap( intersectPolygons );
                }

                {
                    std::vector<cvf::Vec4d> clippedTriangles;
                    for ( std::vector<cvf::Vec3d>& clippedPolygon : clippedPolygons )
                    {
                        std::vector<std::vector<cvf::Vec3d>> polygonTriangles;
                        if ( clippedPolygon.size() == 3u )
                        {
                            polygonTriangles.push_back( clippedPolygon );
                        }
                        else
                        {
                            cvf::Vec3d baryCenter = cvf::Vec3d::ZERO;
                            for ( size_t v = 0; v < clippedPolygon.size(); ++v )
                            {
                                cvf::Vec3d& clippedVertex = clippedPolygon[v];
                                baryCenter += clippedVertex;
                            }
                            baryCenter /= clippedPolygon.size();
                            for ( size_t v = 0; v < clippedPolygon.size(); ++v )
                            {
                                std::vector<cvf::Vec3d> clippedTriangle;
                                if ( v == clippedPolygon.size() - 1 )
                                {
                                    clippedTriangle = { clippedPolygon[v], clippedPolygon[0], baryCenter };
                                }
                                else
                                {
                                    clippedTriangle = { clippedPolygon[v], clippedPolygon[v + 1], baryCenter };
                                }
                                polygonTriangles.push_back( clippedTriangle );
                            }
                        }
                        for ( const std::vector<cvf::Vec3d>& polygonTriangle : polygonTriangles )
                        {
                            // Check triangle area
                            double area =
                                0.5 * ( ( polygonTriangle[1] - polygonTriangle[0] ) ^ ( polygonTriangle[2] - polygonTriangle[0] ) ).length();
                            if ( area < areaThreshold ) continue;
                            for ( const cvf::Vec3d& localVertex : polygonTriangle )
                            {
                                double value = std::numeric_limits<double>::infinity();
                                if ( discrete )
                                {
                                    value = contourLevels[c] + 0.01 * ( contourLevels.back() - contourLevels.front() ) / contourLevels.size();
                                }
                                else
                                {
                                    for ( size_t n = 0; n < 3; ++n )
                                    {
                                        if ( ( triangle[n] - localVertex ).length() < sampleSpacing() * 0.01 &&
                                             triangleWithValues[n].w() != std::numeric_limits<double>::infinity() )
                                        {
                                            value = triangleWithValues[n].w();
                                            break;
                                        }
                                    }
                                    if ( value == std::numeric_limits<double>::infinity() )
                                    {
                                        value = interpolateValue( cvf::Vec2d( localVertex.x(), localVertex.y() ) );
                                        if ( value == std::numeric_limits<double>::infinity() )
                                        {
                                            value = contourLevels[c];
                                        }
                                    }
                                }

                                cvf::Vec4d globalVertex( localVertex, value );
                                clippedTriangles.push_back( globalVertex );
                            }
                        }
                    }

                    {
                        // Add critical section here due to a weird bug when running in a single thread
                        // Running multi threaded does not require this critical section, as we use a thread local data
                        // structure
#pragma omp critical
                        threadTriangles[myThread][c].insert( threadTriangles[myThread][c].end(),
                                                             clippedTriangles.begin(),
                                                             clippedTriangles.end() );
                    }
                }
            }
        }
    }

    std::vector<std::vector<cvf::Vec4d>> trianglesPerLevel( std::max( (size_t)1, m_contourPolygons.size() ) );
    for ( size_t c = 0; c < trianglesPerLevel.size(); ++c )
    {
        std::vector<cvf::Vec4d> allTrianglesThisLevel;
        for ( size_t i = 0; i < threadTriangles.size(); ++i )
        {
            allTrianglesThisLevel.insert( allTrianglesThisLevel.end(), threadTriangles[i][c].begin(), threadTriangles[i][c].end() );
        }

        double triangleAreasThisLevel = sumTriangleAreas( allTrianglesThisLevel );
        if ( c >= m_contourLevelCumulativeAreas.size() || triangleAreasThisLevel > 1.0e-3 * m_contourLevelCumulativeAreas[c] )
        {
            trianglesPerLevel[c] = allTrianglesThisLevel;
        }
    }

    std::vector<cvf::Vec4d> finalTriangles;
    for ( size_t i = 0; i < trianglesPerLevel.size(); ++i )
    {
        finalTriangles.insert( finalTriangles.end(), trianglesPerLevel[i].begin(), trianglesPerLevel[i].end() );
    }

    m_trianglesWithVertexValues = finalTriangles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimContourMapProjection::generateVertices() const
{
    size_t                  nVertices = numberOfVertices();
    std::vector<cvf::Vec3d> vertices( nVertices, cvf::Vec3d::ZERO );

#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nVertices ); ++index )
    {
        cvf::Vec2ui ij     = ijFromVertexIndex( index );
        cvf::Vec2d  mapPos = cellCenterPosition( ij.x(), ij.y() );
        // Shift away from sample point to vertex
        mapPos.x() -= sampleSpacing() * 0.5;
        mapPos.y() -= sampleSpacing() * 0.5;

        cvf::Vec3d vertexPos( mapPos, 0.0 );
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

    std::vector<double> contourLevels;
    if ( resultRangeIsValid() && legendConfig()->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
    {
        legendConfig()->scalarMapper()->majorTickValues( &contourLevels );
        int nContourLevels = static_cast<int>( contourLevels.size() );

        if ( minValue() != std::numeric_limits<double>::infinity() && maxValue() != -std::numeric_limits<double>::infinity() &&
             std::fabs( maxValue() - minValue() ) > 1.0e-8 )
        {
            if ( nContourLevels > 2 )
            {
                const size_t N = contourLevels.size();
                // Adjust contour levels slightly to avoid weird visual artifacts due to numerical error.
                double fudgeFactor    = 1.0e-3;
                double fudgeAmountMin = fudgeFactor * ( contourLevels[1] - contourLevels[0] );
                double fudgeAmountMax = fudgeFactor * ( contourLevels[N - 1u] - contourLevels[N - 2u] );

                contourLevels.front() += fudgeAmountMin;
                contourLevels.back() -= fudgeAmountMax;

                double simplifyEpsilon = m_smoothContourLines() ? 5.0e-2 * sampleSpacing() : 1.0e-3 * sampleSpacing();

                if ( nContourLevels >= 10 )
                {
                    simplifyEpsilon *= 2.0;
                }
                if ( numberOfCells() > 100000 )
                {
                    simplifyEpsilon *= 2.0;
                }
                else if ( numberOfCells() > 1000000 )
                {
                    simplifyEpsilon *= 4.0;
                }

                std::vector<caf::ContourLines::ListOfLineSegments> unorderedLineSegmentsPerLevel =
                    caf::ContourLines::create( m_aggregatedVertexResults, xVertexPositions(), yVertexPositions(), contourLevels );

                contourPolygons = std::vector<ContourPolygons>( unorderedLineSegmentsPerLevel.size() );

#pragma omp parallel for
                for ( int i = 0; i < (int)unorderedLineSegmentsPerLevel.size(); ++i )
                {
                    contourPolygons[i] = createContourPolygonsFromLineSegments( unorderedLineSegmentsPerLevel[i], contourLevels[i] );

                    if ( m_smoothContourLines() )
                    {
                        smoothContourPolygons( &contourPolygons[i], true );
                    }

                    for ( ContourPolygon& polygon : contourPolygons[i] )
                    {
                        RigCellGeometryTools::simplifyPolygon( &polygon.vertices, simplifyEpsilon );
                    }
                }

                // The clipping of contour polygons is intended to detect and fix a smoothed contour polygons
                // crossing into an outer contour line. The current implementation has some side effects causing
                // several contour lines to disappear. Disable this clipping for now
                /*
                if ( m_smoothContourLines() )
                {
                    for ( size_t i = 1; i < contourPolygons.size(); ++i )
                    {
                        clipContourPolygons(&contourPolygons[i], &contourPolygons[i - 1] );
                    }
                }
                */

                m_contourLevelCumulativeAreas.resize( contourPolygons.size(), 0.0 );
                for ( int64_t i = (int64_t)contourPolygons.size() - 1; i >= 0; --i )
                {
                    double levelOuterArea            = sumPolygonArea( contourPolygons[i] );
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
RimContourMapProjection::ContourPolygons
    RimContourMapProjection::createContourPolygonsFromLineSegments( caf::ContourLines::ListOfLineSegments& unorderedLineSegments,
                                                                    double                                 contourValue )
{
    const double areaThreshold = 1.5 * ( sampleSpacing() * sampleSpacing() ) / ( sampleSpacingFactor() * sampleSpacingFactor() );

    ContourPolygons contourPolygons;

    std::vector<std::vector<cvf::Vec3d>> polygons;
    RigCellGeometryTools::createPolygonFromLineSegments( unorderedLineSegments, polygons, 1.0e-8 );
    for ( size_t j = 0; j < polygons.size(); ++j )
    {
        double         signedArea = cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, polygons[j] );
        ContourPolygon contourPolygon;
        contourPolygon.value = contourValue;
        if ( signedArea < 0.0 )
        {
            contourPolygon.vertices.insert( contourPolygon.vertices.end(), polygons[j].rbegin(), polygons[j].rend() );
        }
        else
        {
            contourPolygon.vertices = polygons[j];
        }

        contourPolygon.area = cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, contourPolygon.vertices );
        if ( contourPolygon.area > areaThreshold )
        {
            for ( const cvf::Vec3d& vertex : contourPolygon.vertices )
            {
                contourPolygon.bbox.add( vertex );
            }
            contourPolygons.push_back( contourPolygon );
        }
    }
    return contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::smoothContourPolygons( ContourPolygons* contourPolygons, bool favourExpansion )
{
    CVF_ASSERT( contourPolygons );
    for ( size_t i = 0; i < contourPolygons->size(); ++i )
    {
        ContourPolygon& polygon = contourPolygons->at( i );

        for ( size_t n = 0; n < 20; ++n )
        {
            std::vector<cvf::Vec3d> newVertices;
            newVertices.resize( polygon.vertices.size() );
            double maxChange = 0.0;
            for ( size_t j = 0; j < polygon.vertices.size(); ++j )
            {
                cvf::Vec3d vm1 = polygon.vertices.back();
                cvf::Vec3d v   = polygon.vertices[j];
                cvf::Vec3d vp1 = polygon.vertices.front();
                if ( j > 0u )
                {
                    vm1 = polygon.vertices[j - 1];
                }
                if ( j < polygon.vertices.size() - 1 )
                {
                    vp1 = polygon.vertices[j + 1];
                }
                // Only expand.
                cvf::Vec3d modifiedVertex = 0.5 * ( v + 0.5 * ( vm1 + vp1 ) );
                cvf::Vec3d delta          = modifiedVertex - v;
                cvf::Vec3d tangent3d      = vp1 - vm1;
                cvf::Vec2d tangent2d( tangent3d.x(), tangent3d.y() );
                cvf::Vec3d norm3d( tangent2d.getNormalized().perpendicularVector() );
                if ( delta * norm3d > 0 && favourExpansion )
                {
                    // Normal is always inwards facing so a positive dot product means inward movement
                    // Favour expansion rather than contraction by only contracting by a fraction.
                    // The fraction is empirically found to give a decent result.
                    modifiedVertex = v + 0.2 * delta;
                }
                newVertices[j] = modifiedVertex;
                maxChange      = std::max( maxChange, ( modifiedVertex - v ).length() );
            }
            polygon.vertices.swap( newVertices );
            if ( maxChange < sampleSpacing() * 1.0e-2 ) break;
        }
    }
}

void RimContourMapProjection::clipContourPolygons( ContourPolygons* contourPolygons, const ContourPolygons* clipBy )
{
    CVF_ASSERT( clipBy );
    for ( size_t i = 0; i < contourPolygons->size(); ++i )
    {
        ContourPolygon& polygon = contourPolygons->at( i );
        for ( size_t j = 0; j < clipBy->size(); ++j )
        {
            std::vector<std::vector<cvf::Vec3d>> intersections =
                RigCellGeometryTools::intersectionWithPolygon( polygon.vertices, clipBy->at( j ).vertices );
            if ( !intersections.empty() )
            {
                polygon.vertices = intersections.front();
                polygon.area     = std::abs( cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, polygon.vertices ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumPolygonArea( const ContourPolygons& contourPolygons )
{
    double sumArea = 0.0;
    for ( const ContourPolygon& polygon : contourPolygons )
    {
        sumArea += polygon.area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::sumTriangleAreas( const std::vector<cvf::Vec4d>& triangles )
{
    double sumArea = 0.0;
    for ( size_t i = 0; i < triangles.size(); i += 3 )
    {
        cvf::Vec3d v1( triangles[i].x(), triangles[i].y(), triangles[i].z() );
        cvf::Vec3d v2( triangles[i + 1].x(), triangles[i + 1].y(), triangles[i + 1].z() );
        cvf::Vec3d v3( triangles[i + 2].x(), triangles[i + 2].y(), triangles[i + 2].z() );
        double     area = 0.5 * ( ( v3 - v1 ) ^ ( v2 - v1 ) ).length();
        sumArea += area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapProjection::CellIndexAndResult>
    RimContourMapProjection::cellOverlapVolumesAndResults( const cvf::Vec2d& globalPos2d, const std::vector<double>& weightingResultValues ) const
{
    cvf::Vec3d top2dElementCentroid( globalPos2d, m_expandedBoundingBox.max().z() );
    cvf::Vec3d bottom2dElementCentroid( globalPos2d, m_expandedBoundingBox.min().z() );
    cvf::Vec3d planarDiagonalVector( 0.5 * sampleSpacing(), 0.5 * sampleSpacing(), 0.0 );
    cvf::Vec3d topNECorner    = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement( bottomSWCorner, topNECorner );

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    // Bounding box has been expanded, so 2d element may be outside actual 3d grid
    if ( !bbox2dElement.intersects( m_gridBoundingBox ) )
    {
        return matchingVisibleCellsAndWeight;
    }

    std::vector<size_t> allCellIndices = findIntersectingCells( bbox2dElement );

    std::vector<std::vector<size_t>> kLayerCellIndexVector;
    kLayerCellIndexVector.resize( kLayers() );

    if ( kLayerCellIndexVector.empty() )
    {
        return matchingVisibleCellsAndWeight;
    }

    for ( size_t globalCellIdx : allCellIndices )
    {
        if ( ( *m_cellGridIdxVisibility )[globalCellIdx] )
        {
            kLayerCellIndexVector[kLayer( globalCellIdx )].push_back( globalCellIdx );
        }
    }

    for ( const auto& kLayerIndices : kLayerCellIndexVector )
    {
        for ( size_t globalCellIdx : kLayerIndices )
        {
            double overlapVolume = calculateOverlapVolume( globalCellIdx, bbox2dElement );
            if ( overlapVolume > 0.0 )
            {
                double weight = overlapVolume * getParameterWeightForCell( gridResultIndex( globalCellIdx ), weightingResultValues );
                if ( weight > 0.0 )
                {
                    matchingVisibleCellsAndWeight.push_back( std::make_pair( globalCellIdx, weight ) );
                }
            }
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapProjection::CellIndexAndResult>
    RimContourMapProjection::cellRayIntersectionAndResults( const cvf::Vec2d& globalPos2d, const std::vector<double>& weightingResultValues ) const
{
    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    cvf::Vec3d highestPoint( globalPos2d, m_expandedBoundingBox.max().z() );
    cvf::Vec3d lowestPoint( globalPos2d, m_expandedBoundingBox.min().z() );

    // Bounding box has been expanded, so ray may be outside actual 3d grid
    if ( !m_gridBoundingBox.contains( highestPoint ) )
    {
        return matchingVisibleCellsAndWeight;
    }

    cvf::BoundingBox rayBBox;
    rayBBox.add( highestPoint );
    rayBBox.add( lowestPoint );

    std::vector<size_t> allCellIndices = findIntersectingCells( rayBBox );

    std::map<size_t, std::vector<size_t>> kLayerIndexMap;

    for ( size_t globalCellIdx : allCellIndices )
    {
        if ( ( *m_cellGridIdxVisibility )[globalCellIdx] )
        {
            kLayerIndexMap[kLayer( globalCellIdx )].push_back( globalCellIdx );
        }
    }

    for ( const auto& kLayerIndexPair : kLayerIndexMap )
    {
        double                                 weightSumThisKLayer = 0.0;
        std::vector<std::pair<size_t, double>> cellsAndWeightsThisLayer;
        for ( size_t globalCellIdx : kLayerIndexPair.second )
        {
            double lengthInCell = calculateRayLengthInCell( globalCellIdx, highestPoint, lowestPoint );
            if ( lengthInCell > 0.0 )
            {
                cellsAndWeightsThisLayer.push_back( std::make_pair( globalCellIdx, lengthInCell ) );
                weightSumThisKLayer += lengthInCell;
            }
        }
        for ( auto& cellWeightPair : cellsAndWeightsThisLayer )
        {
            cellWeightPair.second /= weightSumThisKLayer;
            matchingVisibleCellsAndWeight.push_back( cellWeightPair );
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
bool RimContourMapProjection::isStraightSummationResult() const
{
    return isStraightSummationResult( m_resultAggregation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isStraightSummationResult( ResultAggregationEnum aggregationType )
{
    return aggregationType == RESULTS_OIL_COLUMN || aggregationType == RESULTS_GAS_COLUMN || aggregationType == RESULTS_HC_COLUMN ||
           aggregationType == RESULTS_SUM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::interpolateValue( const cvf::Vec2d& gridPos2d ) const
{
    cvf::Vec2ui cellContainingPoint = ijFromLocalPos( gridPos2d );
    cvf::Vec2d  cellCenter          = cellCenterPosition( cellContainingPoint.x(), cellContainingPoint.y() );

    std::array<cvf::Vec3d, 4> x;
    x[0] = cvf::Vec3d( cellCenter + cvf::Vec2d( -sampleSpacing() * 0.5, -sampleSpacing() * 0.5 ), 0.0 );
    x[1] = cvf::Vec3d( cellCenter + cvf::Vec2d( sampleSpacing() * 0.5, -sampleSpacing() * 0.5 ), 0.0 );
    x[2] = cvf::Vec3d( cellCenter + cvf::Vec2d( sampleSpacing() * 0.5, sampleSpacing() * 0.5 ), 0.0 );
    x[3] = cvf::Vec3d( cellCenter + cvf::Vec2d( -sampleSpacing() * 0.5, sampleSpacing() * 0.5 ), 0.0 );

    cvf::Vec4d baryCentricCoords = cvf::GeometryTools::barycentricCoords( x[0], x[1], x[2], x[3], cvf::Vec3d( gridPos2d, 0.0 ) );

    std::array<cvf::Vec2ui, 4> v;
    v[0] = cellContainingPoint;
    v[1] = cvf::Vec2ui( cellContainingPoint.x() + 1u, cellContainingPoint.y() );
    v[2] = cvf::Vec2ui( cellContainingPoint.x() + 1u, cellContainingPoint.y() + 1u );
    v[3] = cvf::Vec2ui( cellContainingPoint.x(), cellContainingPoint.y() + 1u );

    std::array<double, 4> vertexValues;
    double                validBarycentricCoordsSum = 0.0;
    for ( int i = 0; i < 4; ++i )
    {
        double vertexValue = valueAtVertex( v[i].x(), v[i].y() );
        if ( vertexValue == std::numeric_limits<double>::infinity() )
        {
            return std::numeric_limits<double>::infinity();
        }
        else
        {
            vertexValues[i] = vertexValue;
            validBarycentricCoordsSum += baryCentricCoords[i];
        }
    }

    if ( validBarycentricCoordsSum < 1.0e-8 )
    {
        return std::numeric_limits<double>::infinity();
    }

    // Calculate final value
    double value = 0.0;
    for ( int i = 0; i < 4; ++i )
    {
        value += baryCentricCoords[i] / validBarycentricCoordsSum * vertexValues[i];
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::valueInCell( uint i, uint j ) const
{
    size_t index = cellIndexFromIJ( i, j );
    if ( index < numberOfCells() )
    {
        return m_aggregatedResults.at( index );
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::hasResultInCell( uint i, uint j ) const
{
    return !cellsAtIJ( i, j ).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::calculateValueAtVertex( uint vi, uint vj ) const
{
    std::vector<uint> averageIs;
    std::vector<uint> averageJs;

    if ( vi > 0u ) averageIs.push_back( vi - 1 );
    if ( vj > 0u ) averageJs.push_back( vj - 1 );
    if ( vi < m_mapSize.x() ) averageIs.push_back( vi );
    if ( vj < m_mapSize.y() ) averageJs.push_back( vj );

    RiaWeightedMeanCalculator<double> calc;
    for ( uint j : averageJs )
    {
        for ( uint i : averageIs )
        {
            if ( hasResultInCell( i, j ) )
            {
                calc.addValueAndWeight( valueInCell( i, j ), 1.0 );
            }
        }
    }
    if ( calc.validAggregatedWeight() )
    {
        return calc.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RimContourMapProjection::cellsAtIJ( uint i, uint j ) const
{
    size_t cellIndex = cellIndexFromIJ( i, j );
    if ( cellIndex < m_projected3dGridIndices.size() )
    {
        return m_projected3dGridIndices[cellIndex];
    }
    return std::vector<std::pair<size_t, double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::cellIndexFromIJ( uint i, uint j ) const
{
    CVF_ASSERT( i < m_mapSize.x() );
    CVF_ASSERT( j < m_mapSize.y() );

    return i + j * m_mapSize.x();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimContourMapProjection::vertexIndexFromIJ( uint i, uint j ) const
{
    return i + j * ( m_mapSize.x() + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromVertexIndex( size_t gridIndex ) const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();

    uint quotientX  = static_cast<uint>( gridIndex ) / gridSize.x();
    uint remainderX = static_cast<uint>( gridIndex ) % gridSize.x();

    return cvf::Vec2ui( remainderX, quotientX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromCellIndex( size_t cellIndex ) const
{
    CVF_TIGHT_ASSERT( cellIndex < numberOfCells() );

    uint quotientX  = static_cast<uint>( cellIndex ) / m_mapSize.x();
    uint remainderX = static_cast<uint>( cellIndex ) % m_mapSize.x();

    return cvf::Vec2ui( remainderX, quotientX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::ijFromLocalPos( const cvf::Vec2d& localPos2d ) const
{
    uint i = localPos2d.x() / sampleSpacing();
    uint j = localPos2d.y() / sampleSpacing();
    return cvf::Vec2ui( i, j );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::cellCenterPosition( uint i, uint j ) const
{
    cvf::Vec3d gridExtent = m_expandedBoundingBox.extent();
    cvf::Vec2d cellCorner = cvf::Vec2d( ( i * gridExtent.x() ) / ( m_mapSize.x() ), ( j * gridExtent.y() ) / ( m_mapSize.y() ) );

    return cellCorner + cvf::Vec2d( sampleSpacing() * 0.5, sampleSpacing() * 0.5 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::origin2d() const
{
    return cvf::Vec2d( m_expandedBoundingBox.min().x(), m_expandedBoundingBox.min().y() );
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.x() for UTM x)
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::xVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().x();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve( gridSize.x() );
    for ( uint i = 0; i < gridSize.x(); ++i )
    {
        positions.push_back( ( i * gridExtent ) / ( gridSize.x() - 1 ) );
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.y() for UTM y)
//--------------------------------------------------------------------------------------------------
std::vector<double> RimContourMapProjection::yVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().y();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve( gridSize.y() );
    for ( uint j = 0; j < gridSize.y(); ++j )
    {
        positions.push_back( ( j * gridExtent ) / ( gridSize.y() - 1 ) );
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RimContourMapProjection::calculateMapSize() const
{
    cvf::Vec3d gridExtent = m_expandedBoundingBox.extent();

    uint projectionSizeX = static_cast<uint>( std::ceil( gridExtent.x() / sampleSpacing() ) );
    uint projectionSizeY = static_cast<uint>( std::ceil( gridExtent.y() / sampleSpacing() ) );

    return cvf::Vec2ui( projectionSizeX, projectionSizeY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapProjection::gridEdgeOffset() const
{
    return sampleSpacing() * 2.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_resultAggregation )
    {
        ResultAggregation previousAggregation = static_cast<ResultAggregationEnum>( oldValue.toInt() );
        if ( isStraightSummationResult( previousAggregation ) != isStraightSummationResult() )
        {
            clearGridMapping();
        }
        else
        {
            clearResults();
        }
        clearTimeStepRange();
    }
    else if ( changedField == &m_smoothContourLines )
    {
        clearGeometry();
    }
    else if ( changedField == &m_relativeSampleSpacing )
    {
        clearGridMapping();
        clearResults();
        clearTimeStepRange();
    }

    baseView()->updateConnectedEditors();

    RimProject* proj = RimProject::current();
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_relativeSampleSpacing == field )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum                       = 0.2;
            myAttr->m_maximum                       = 2.0;
            myAttr->m_sliderTickCount               = 9;
            myAttr->m_delaySliderUpdateUntilRelease = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* mainGroup = uiOrdering.addNewGroup( "Projection Settings" );
    mainGroup->add( &m_resultAggregation );
    legendConfig()->uiOrdering( "NumLevelsOnly", *mainGroup );
    mainGroup->add( &m_relativeSampleSpacing );
    mainGroup->add( &m_showContourLines );
    mainGroup->add( &m_showContourLabels );
    m_showContourLabels.uiCapability()->setUiReadOnly( !m_showContourLines() );
    mainGroup->add( &m_smoothContourLines );
    m_smoothContourLines.uiCapability()->setUiReadOnly( !m_showContourLines() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::initAfterRead()
{
}
