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

#include "ContourMap/RigContourMapCalculator.h"
#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RigContourMapTrianglesGenerator.h"
#include "RigFloodingSettings.h"

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

    addItem( RigContourMapCalculator::MOBILE_OIL_COLUMN, "MOBILE_OIL_COLUMN", "Mobile Oil Column" );
    addItem( RigContourMapCalculator::MOBILE_GAS_COLUMN, "MOBILE_GAS_COLUMN", "Mobile Gas Column" );
    addItem( RigContourMapCalculator::MOBILE_HYDROCARBON_COLUMN, "MOBILE_HC_COLUMN", "Mobile Hydrocarbon Column" );

    addItem( RigContourMapCalculator::MEAN, "MEAN_VALUE", "Arithmetic Mean" );
    addItem( RigContourMapCalculator::HARMONIC_MEAN, "HARM_VALUE", "Harmonic Mean" );
    addItem( RigContourMapCalculator::GEOMETRIC_MEAN, "GEOM_VALUE", "Geometric Mean" );
    addItem( RigContourMapCalculator::SUM, "SUM", "Sum" );

    addItem( RigContourMapCalculator::TOP_VALUE, "TOP_VALUE", "Top Value" );
    addItem( RigContourMapCalculator::MIN_VALUE, "MIN_VALUE", "Min Value" );
    addItem( RigContourMapCalculator::MAX_VALUE, "MAX_VALUE", "Max Value" );

    setDefault( RigContourMapCalculator::MEAN );
}

template <>
void RimContourMapProjection::FloodingType::setUp()
{
    addItem( RigFloodingSettings::FloodingType::WATER_FLOODING, "WATER_FLOODING", "Water Flooding" );
    addItem( RigFloodingSettings::FloodingType::GAS_FLOODING, "GAS_FLOODING", "Gas Flooding" );
    addItem( RigFloodingSettings::FloodingType::USER_DEFINED, "USER_DEFINED", "User Defined Value" );
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
    CAF_PDM_InitObject( "Map Projection", ":/2DMapProjection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_resolution, "Resolution", "Sampling Resolution" );
    m_resolution.setValue( RimContourMapResolutionTools::SamplingResolution::NORMAL );

    CAF_PDM_InitFieldNoDefault( &m_resultAggregation, "ResultAggregation", "Result Aggregation" );

    CAF_PDM_InitFieldNoDefault( &m_oilFloodingType, "OilFloodingType", "Residual Oil Given By" );
    m_oilFloodingType.setValue( RigFloodingSettings::FloodingType::WATER_FLOODING );
    CAF_PDM_InitField( &m_userDefinedFloodingOil, "UserDefinedFloodingOil", 0.0, "" );
    m_userDefinedFloodingOil.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasFloodingType, "GasFloodingType", RigFloodingSettings::FloodingType::GAS_FLOODING, "Residual Oil-in-Gas Given By" );
    caf::AppEnum<RigFloodingSettings::FloodingType>::setEnumSubset( &m_gasFloodingType,
                                                                    { RigFloodingSettings::FloodingType::GAS_FLOODING,
                                                                      RigFloodingSettings::FloodingType::USER_DEFINED } );

    CAF_PDM_InitField( &m_userDefinedFloodingGas, "UserDefinedFloodingGas", 0.0, "" );
    m_userDefinedFloodingGas.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showContourLines, "ContourLines", true, "Show Contour Lines" );
    CAF_PDM_InitField( &m_showContourLabels, "ContourLabels", true, "Show Contour Labels" );
    CAF_PDM_InitField( &m_smoothContourLines, "SmoothContourLines", true, "Smooth Contour Lines" );

    auto defaultValue = caf::AppEnum<RimIntersectionFilterEnum>( RimIntersectionFilterEnum::INTERSECT_FILTER_NONE );
    CAF_PDM_InitField( &m_valueFilterType, "ValueFilterType", defaultValue, "Value Filter" );
    CAF_PDM_InitField( &m_upperThreshold, "UpperThreshold", 0.0, "Upper Threshold" );
    m_upperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_lowerThreshold, "LowerThreshold", 0.0, "Lower Threshold" );
    m_lowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

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
        clearMinMaxValueRange();

        if ( gridMappingNeedsUpdating() )
        {
            m_contourMapProjection->setCellVisibility( getCellVisibility() );
            m_contourMapProjection->generateGridMapping( m_resultAggregation(), retrieveParameterWeights(), m_selectedKLayers, {} );
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

    if ( geometryNeedsUpdating() && m_contourMapProjection != nullptr )
    {
        m_contourMapProjection->setValueFilter( valueFilterMinMax() );

        std::vector<double> contourLevels;

        bool discrete = false;
        if ( ( legendConfig() != nullptr ) && ( legendConfig()->scalarMapper() != nullptr ) &&
             ( legendConfig()->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER ) )
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
    return RimContourMapResolutionTools::resolutionFromEnumValue( m_resolution() );
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
std::optional<std::pair<double, double>> RimContourMapProjection::valueFilterMinMax() const
{
    if ( m_valueFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_NONE ) return std::nullopt;

    double minimum = m_minResultAllTimeSteps;
    double maximum = m_maxResultAllTimeSteps;

    if ( m_valueFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW ||
         m_valueFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN )
    {
        maximum = m_upperThreshold;
    }

    if ( m_valueFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE ||
         m_valueFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN )
    {
        minimum = m_lowerThreshold;
    }

    return std::pair<double, double>( minimum, maximum );
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
    if ( currentVisibility->size() != cellGridIdxVisibility->size() ) return true;

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
             m_contourMapProjection->aggregatedVertexResultsFiltered().size() != m_contourMapProjection->numberOfVertices() ||
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
    clearMinMaxValueRange();

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

    clearMinMaxValueRange();
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
std::pair<double, double> RimContourMapProjection::minmaxValuesAllTimeSteps()
{
    if ( !resultRangeIsValid() )
    {
        const auto [minVal, maxVal] = computeMinMaxValuesAllTimeSteps();
        m_minResultAllTimeSteps     = minVal;
        m_maxResultAllTimeSteps     = maxVal;
    }

    return std::pair<double, double>( m_minResultAllTimeSteps, m_maxResultAllTimeSteps );
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
    if ( ( changedField == &m_resultAggregation ) || ( changedField == &m_oilFloodingType ) || ( changedField == &m_gasFloodingType ) ||
         ( changedField == &m_userDefinedFloodingOil ) || ( changedField == &m_userDefinedFloodingGas ) )
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
        clearMinMaxValueRange();
    }
    else if ( changedField == &m_smoothContourLines )
    {
        clearGeometry();
    }
    else if ( changedField == &m_resolution )
    {
        clearGridMapping();
        clearResults();
        clearMinMaxValueRange();
    }

    baseView()->updateConnectedEditors();
    baseView()->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_lowerThreshold == field || &m_upperThreshold == field )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum         = m_minResultAllTimeSteps;
            myAttr->m_maximum         = m_maxResultAllTimeSteps;
            myAttr->m_sliderTickCount = 20;
        }
    }
    else if ( ( &m_userDefinedFloodingOil == field ) || ( &m_userDefinedFloodingGas == field ) )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum         = 0.0;
            myAttr->m_maximum         = 1.0;
            myAttr->m_sliderTickCount = 20;
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

    if ( RigContourMapCalculator::isMobileColumnResult( m_resultAggregation() ) )
    {
        if ( m_resultAggregation() != RigContourMapCalculator::MOBILE_GAS_COLUMN )
        {
            mainGroup->add( &m_oilFloodingType );
            if ( m_oilFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                mainGroup->add( &m_userDefinedFloodingOil );
            }
        }
        if ( m_resultAggregation() != RigContourMapCalculator::MOBILE_OIL_COLUMN )
        {
            mainGroup->add( &m_gasFloodingType );
            if ( m_gasFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                mainGroup->add( &m_userDefinedFloodingGas );
            }
        }
    }

    legendConfig()->uiOrdering( "NumLevelsOnly", *mainGroup );
    mainGroup->add( &m_resolution );
    mainGroup->add( &m_showContourLines );
    mainGroup->add( &m_showContourLabels );
    m_showContourLabels.uiCapability()->setUiReadOnly( !m_showContourLines() );
    mainGroup->add( &m_smoothContourLines );
    m_smoothContourLines.uiCapability()->setUiReadOnly( !m_showContourLines() );

    appendValueFilterGroup( uiOrdering );

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
void RimContourMapProjection::appendValueFilterGroup( caf::PdmUiOrdering& uiOrdering )
{
    auto valueFilterGroup = uiOrdering.addNewGroup( "Value Filter" );
    valueFilterGroup->add( &m_valueFilterType );

    switch ( m_valueFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
            m_upperThreshold.uiCapability()->setUiName( "Threshold" );
            valueFilterGroup->add( &m_upperThreshold );
            break;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            m_lowerThreshold.uiCapability()->setUiName( "Lower Threshold" );
            valueFilterGroup->add( &m_lowerThreshold );
            m_upperThreshold.uiCapability()->setUiName( "Upper Threshold" );
            valueFilterGroup->add( &m_upperThreshold );
            break;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
            m_lowerThreshold.uiCapability()->setUiName( "Threshold" );
            valueFilterGroup->add( &m_lowerThreshold );
            break;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            break;
    }
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
void RimContourMapProjection::clearMinMaxValueRange()
{
    m_minResultAllTimeSteps = std::numeric_limits<double>::infinity();
    m_maxResultAllTimeSteps = -std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapProjection::useKLayers( std::set<int> kLayers )
{
    m_selectedKLayers = kLayers;
}
