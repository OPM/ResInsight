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

#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"
#include "RigContourMapProjection.h"
#include "RigContourMapTrianglesGenerator.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTextAnnotation.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfArray.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfVector2.h"

namespace caf
{
template <>
void RimContourMapProjection::ResultAggregation::setUp()
{
    addItem( RigContourMapCalculator::OIL_COLUMN, "OIL_COLUMN", "Oil Column" );
    addItem( RigContourMapCalculator::GAS_COLUMN, "GAS_COLUMN", "Gas Column" );
    addItem( RigContourMapCalculator::HYDROCARBON_COLUMN, "HC_COLUMN", "Hydrocarbon Column" );

    addItem( RigContourMapCalculator::MEAN, "MEAN_VALUE", "Arithmetic Mean" );
    addItem( RigContourMapCalculator::HARMONIC_MEAN, "HARM_VALUE", "Harmonic Mean" );
    addItem( RigContourMapCalculator::GEOMETRIC_MEAN, "GEOM_VALUE", "Geometric Mean" );
    addItem( RigContourMapCalculator::VOLUME_SUM, "VOLUME_SUM", "Volume Weighted Sum" );
    addItem( RigContourMapCalculator::SUM, "SUM", "Sum" );

    addItem( RigContourMapCalculator::TOP_VALUE, "TOP_VALUE", "Top  Value" );
    addItem( RigContourMapCalculator::MIN_VALUE, "MIN_VALUE", "Min Value" );
    addItem( RigContourMapCalculator::MAX_VALUE, "MAX_VALUE", "Max Value" );

    setDefault( RigContourMapCalculator::MEAN );
}
} // namespace caf

CAF_PDM_ABSTRACT_SOURCE_INIT( RimContourMapProjection, "RimContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection::RimContourMapProjection()
    : m_pickPoint( cvf::Vec2d::UNDEFINED )
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

    if ( !m_contourMapGrid || !m_contourMapProjection ) updateGridInformation();

    progress.setProgress( 10 );

    if ( gridMappingNeedsUpdating() || mapCellVisibilityNeedsUpdating( timeStep ) || resultVariableChanged() )
    {
        clearResults();
        clearTimeStepRange();

        if ( gridMappingNeedsUpdating() )
        {
            m_contourMapProjection->setCellVisibility( getCellVisibility() );
            m_contourMapProjection->generateGridMapping( m_resultAggregation(), retrieveParameterWeights() );
        }
        progress.setProgress( 20 );
        m_mapCellVisibility = m_contourMapProjection->getMapCellVisibility( timeStep, m_resultAggregation() );
        progress.setProgress( 30 );
    }
    else
    {
        progress.setProgress( 30 );
    }

    if ( resultsNeedsUpdating( timeStep ) )
    {
        clearGeometry();
        generateAndSaveResults( timeStep );

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
        std::vector<double> contourLevels;

        bool discrete = false;
        if ( legendConfig()->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
        {
            legendConfig()->scalarMapper()->majorTickValues( &contourLevels );

            if ( resultRangeIsValid() )
            {
                std::tie( m_contourPolygons, m_contourLevelCumulativeAreas ) =
                    RigContourMapTrianglesGenerator::generateContourPolygons( *m_contourMapGrid,
                                                                              *m_contourMapProjection,
                                                                              contourLevels,
                                                                              sampleSpacing(),
                                                                              sampleSpacingFactor(),
                                                                              m_smoothContourLines() );
            }
            progress.setProgress( 25 );

            if ( legendConfig()->mappingMode() == RimRegularLegendConfig::MappingType::LINEAR_DISCRETE ||
                 legendConfig()->mappingMode() == RimRegularLegendConfig::MappingType::LOG10_DISCRETE )
            {
                discrete = true;
            }
        }

        m_trianglesWithVertexValues = RigContourMapTrianglesGenerator::generateTrianglesWithVertexValues( *m_contourMapGrid,
                                                                                                          *m_contourMapProjection,
                                                                                                          m_contourPolygons,
                                                                                                          contourLevels,
                                                                                                          m_contourLevelCumulativeAreas,
                                                                                                          discrete,
                                                                                                          sampleSpacing() );
    }
    progress.setProgress( 100 );
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
    if ( !rimCase ) return QString();

    return rimCase->caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapProjection::currentTimeStepName() const
{
    RimCase* rimCase = baseView()->ownerCase();
    if ( !rimCase || m_currentResultTimestep == -1 ) return QString();

    return rimCase->timeStepName( m_currentResultTimestep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigContourMapProjection* RimContourMapProjection::mapProjection() const
{
    return m_contourMapProjection.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigContourMapGrid* RimContourMapProjection::mapGrid() const
{
    return m_contourMapGrid.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isColumnResult() const
{
    return RigContourMapCalculator::isColumnResult( m_resultAggregation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::setPickPoint( cvf::Vec2d globalPickPoint )
{
    m_pickPoint = globalPickPoint - m_contourMapGrid->origin2d();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RimContourMapProjection::pickPoint() const
{
    return m_pickPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimContourMapProjection::origin3d() const
{
    return m_contourMapGrid->origin3d();
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
    const std::vector<std::pair<size_t, double>>& matchingCells = m_contourMapProjection->cellsAtIJ( i, j );
    return RigContourMapCalculator::calculateValueInMapCell( *m_contourMapProjection, matchingCells, gridCellValues, m_resultAggregation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::gridMappingNeedsUpdating() const
{
    if ( !m_contourMapProjection ) return true;

    if ( m_contourMapProjection->projected3dGridIndices().size() != m_contourMapProjection->numberOfCells() ) return true;

    auto cellGridIdxVisibility = m_contourMapProjection->getCellVisibility();
    if ( cellGridIdxVisibility.isNull() ) return true;

    cvf::ref<cvf::UByteArray> currentVisibility = getCellVisibility();

    CVF_ASSERT( currentVisibility->size() == cellGridIdxVisibility->size() );
    for ( size_t i = 0; i < currentVisibility->size(); ++i )
    {
        if ( ( *currentVisibility )[i] != ( *cellGridIdxVisibility )[i] ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::resultsNeedsUpdating( int timeStep ) const
{
    if ( !m_contourMapProjection ) return true;

    return ( m_contourMapProjection->aggregatedResults().size() != m_contourMapProjection->numberOfCells() ||
             m_contourMapProjection->aggregatedVertexResults().size() != m_contourMapProjection->numberOfVertices() ||
             timeStep != m_currentResultTimestep );
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
    clearTimeStepRange();

    m_contourMapProjection.reset();
    m_contourMapGrid.reset();

    m_mapCellVisibility.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::clearResults()
{
    clearGeometry();

    if ( m_contourMapProjection ) m_contourMapProjection->clearResults();
    m_currentResultTimestep = -1;

    clearResultVariable();

    clearTimeStepRange();
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
bool RimContourMapProjection::mapCellVisibilityNeedsUpdating( int timestep )
{
    if ( !m_contourMapProjection ) return true;

    std::vector<bool> mapCellVisiblity = m_contourMapProjection->getMapCellVisibility( timestep, m_resultAggregation() );
    return !( mapCellVisiblity == m_mapCellVisibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::generateVertexResults()
{
    if ( m_contourMapProjection ) m_contourMapProjection->generateVertexResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isMeanResult() const
{
    return RigContourMapCalculator::isMeanResult( m_resultAggregation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapProjection::isStraightSummationResult() const
{
    return RigContourMapCalculator::isStraightSummationResult( m_resultAggregation() );
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
        ResultAggregation previousAggregation = static_cast<RigContourMapCalculator::ResultAggregationType>( oldValue.toInt() );
        if ( RigContourMapCalculator::isStraightSummationResult( previousAggregation ) != isStraightSummationResult() )
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
void RimContourMapProjection::clearTimeStepRange()
{
    m_minResultAllTimeSteps = std::numeric_limits<double>::infinity();
    m_maxResultAllTimeSteps = -std::numeric_limits<double>::infinity();
}
