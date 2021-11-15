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
#include "RimGeoMechContourMapProjection.h"

#include "RiaImageTools.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCellGeometryTools.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigHexIntersectionTools.h"

#include "RimCellFilterCollection.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechPropertyFilterCollection.h"

#include "RivFemElmVisibilityCalculator.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfArray.h"
#include "cvfCellRange.h"
#include "cvfGeometryTools.h"
#include "cvfGeometryUtils.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfVector3.h"

#include <QDebug>
#include <algorithm>
#include <array>

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapProjection, "RimGeoMechContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::RimGeoMechContourMapProjection()
    : m_kLayers( 0u )
{
    CAF_PDM_InitObject( "RimContourMapProjection", ":/2DMapProjection16x16.png", "", "" );
    CAF_PDM_InitField( &m_limitToPorePressureRegions, "LimitToPorRegion", true, "Limit to Pore Pressure regions" );
    CAF_PDM_InitField( &m_applyPPRegionLimitVertically, "VerticalLimit", false, "Apply Limit Vertically" );
    CAF_PDM_InitField( &m_paddingAroundPorePressureRegion,
                       "PaddingAroundPorRegion",
                       0.0,
                       "Horizontal Padding around PP regions",
                       "",
                       "",
                       "" );
    m_paddingAroundPorePressureRegion.uiCapability()->setUiEditorTypeName(
        caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    setName( "Map Projection" );
    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::~RimGeoMechContourMapProjection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechContourMapProjection::resultDescriptionText() const
{
    QString resultText =
        QString( "%1, %2" ).arg( resultAggregationText() ).arg( view()->cellResult()->resultFieldUiName() );
    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimGeoMechContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateLegend()
{
    RimGeoMechCellColors* cellColors = view()->cellResult();

    double minVal = minValue( m_aggregatedResults );
    double maxVal = maxValue( m_aggregatedResults );

    std::pair<double, double> minmaxValAllTimeSteps = minmaxValuesAllTimeSteps();

    legendConfig()->setAutomaticRanges( minmaxValAllTimeSteps.first, minmaxValAllTimeSteps.second, minVal, maxVal );

    QString projectionLegendText = QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() );
    if ( cellColors->resultAddress().isValid() )
    {
        projectionLegendText += QString( "\nResult: %1" ).arg( cellColors->resultFieldUiName() );
        if ( !cellColors->resultComponentUiName().isEmpty() )
        {
            projectionLegendText += QString( ", %1" ).arg( cellColors->resultComponentUiName() );
        }
    }
    else
    {
        projectionLegendText += QString( "\nNo Result Selected" );
    }

    legendConfig()->setTitle( projectionLegendText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::sampleSpacing() const
{
    RimGeoMechCase* geoMechCase = this->geoMechCase();
    if ( geoMechCase )
    {
        return m_relativeSampleSpacing * geoMechCase->characteristicCellSize();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimGeoMechContourMapProjection::getCellVisibility() const
{
    cvf::ref<cvf::UByteArray> cellGridIdxVisibility = new cvf::UByteArray( m_femPart->elementCount() );
    RivFemElmVisibilityCalculator::computeAllVisible( cellGridIdxVisibility.p(), m_femPart.p() );

    if ( view()->cellFilterCollection()->isActive() )
    {
        cvf::CellRangeFilter cellRangeFilter;
        view()->cellFilterCollection()->compoundCellRangeFilter( &cellRangeFilter, 0 );
        RivFemElmVisibilityCalculator::computeRangeVisibility( cellGridIdxVisibility.p(), m_femPart.p(), cellRangeFilter );
    }
    if ( view()->propertyFilterCollection()->isActive() )
    {
        RivFemElmVisibilityCalculator::computePropertyVisibility( cellGridIdxVisibility.p(),
                                                                  m_femPart.p(),
                                                                  view()->currentTimeStep(),
                                                                  cellGridIdxVisibility.p(),
                                                                  view()->geoMechPropertyFilterCollection() );
    }

    return cellGridIdxVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechContourMapProjection::calculateExpandedPorBarBBox( int timeStep ) const
{
    RigFemResultAddress          porBarAddr( RigFemResultPosEnum::RIG_ELEMENT_NODAL,
                                    "POR-Bar",
                                    view()->cellResult()->resultComponentName().toStdString() );
    RigGeoMechCaseData*          caseData         = geoMechCase()->geoMechData();
    RigFemPartResultsCollection* resultCollection = caseData->femPartResults();

    const std::vector<float>& resultValues = resultCollection->resultValues( porBarAddr, 0, timeStep );
    cvf::BoundingBox          boundingBox;

    if ( resultValues.empty() )
    {
        return boundingBox;
    }

    for ( int i = 0; i < m_femPart->elementCount(); ++i )
    {
        size_t resValueIdx = m_femPart->elementNodeResultIdx( (int)i, 0 );
        CVF_ASSERT( resValueIdx < resultValues.size() );
        double scalarValue   = resultValues[resValueIdx];
        bool   validPorValue = scalarValue != std::numeric_limits<double>::infinity();

        if ( validPorValue )
        {
            std::array<cvf::Vec3d, 8> hexCorners;
            m_femPartGrid->cellCornerVertices( i, hexCorners.data() );
            for ( size_t c = 0; c < 8; ++c )
            {
                boundingBox.add( hexCorners[c] );
            }
        }
    }
    cvf::Vec3d boxMin    = boundingBox.min();
    cvf::Vec3d boxMax    = boundingBox.max();
    cvf::Vec3d boxExtent = boundingBox.extent();
    boxMin.x() -= boxExtent.x() * 0.5 * m_paddingAroundPorePressureRegion();
    boxMin.y() -= boxExtent.y() * 0.5 * m_paddingAroundPorePressureRegion();
    boxMax.x() += boxExtent.x() * 0.5 * m_paddingAroundPorePressureRegion();
    boxMax.y() += boxExtent.y() * 0.5 * m_paddingAroundPorePressureRegion();
    return cvf::BoundingBox( boxMin, boxMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateGridInformation()
{
    RimGeoMechCase* geoMechCase = this->geoMechCase();
    m_femPart                   = geoMechCase->geoMechData()->femParts()->part( 0 );
    m_femPartGrid               = m_femPart->getOrCreateStructGrid();
    m_kLayers                   = m_femPartGrid->cellCountK();
    m_femPart->ensureIntersectionSearchTreeIsBuilt();

    m_gridBoundingBox = geoMechCase->activeCellsBoundingBox();

    if ( m_limitToPorePressureRegions )
    {
        m_expandedBoundingBox = calculateExpandedPorBarBBox( view()->currentTimeStep() );
        if ( !m_expandedBoundingBox.isValid() )
        {
            m_limitToPorePressureRegions = false;
        }
    }

    if ( !m_limitToPorePressureRegions )
    {
        m_expandedBoundingBox = m_gridBoundingBox;
    }

    cvf::Vec3d minExpandedPoint = m_expandedBoundingBox.min() - cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    cvf::Vec3d maxExpandedPoint = m_expandedBoundingBox.max() + cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    if ( m_limitToPorePressureRegions && !m_applyPPRegionLimitVertically )
    {
        minExpandedPoint.z() = m_gridBoundingBox.min().z();
        maxExpandedPoint.z() = m_gridBoundingBox.max().z();
    }
    m_expandedBoundingBox = cvf::BoundingBox( minExpandedPoint, maxExpandedPoint );

    m_mapSize = calculateMapSize();

    // Re-jig max point to be an exact multiple of cell size
    cvf::Vec3d minPoint   = m_expandedBoundingBox.min();
    cvf::Vec3d maxPoint   = m_expandedBoundingBox.max();
    maxPoint.x()          = minPoint.x() + m_mapSize.x() * sampleSpacing();
    maxPoint.y()          = minPoint.y() + m_mapSize.y() * sampleSpacing();
    m_expandedBoundingBox = cvf::BoundingBox( minPoint, maxPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RimGeoMechContourMapProjection::getMapCellVisibility()
{
    cvf::Vec2ui                            nCellsIJ = numberOfElementsIJ();
    std::vector<std::vector<unsigned int>> distanceImage( nCellsIJ.x(), std::vector<unsigned int>( nCellsIJ.y(), 0u ) );

    std::vector<bool>   mapCellVisibility;
    RigFemResultAddress resAddr = view()->cellResult()->resultAddress();

    if ( m_limitToPorePressureRegions )
    {
        resAddr = RigFemResultAddress( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "POR-Bar", "" );
    }

    std::vector<double> cellResults = generateResultsFromAddress( resAddr, mapCellVisibility, view()->currentTimeStep() );

    mapCellVisibility.resize( numberOfCells(), true );
    CVF_ASSERT( mapCellVisibility.size() == cellResults.size() );

    {
        cvf::BoundingBox validResBoundingBox;
        for ( size_t cellIndex = 0; cellIndex < cellResults.size(); ++cellIndex )
        {
            cvf::Vec2ui ij = ijFromCellIndex( cellIndex );
            if ( cellResults[cellIndex] != std::numeric_limits<double>::infinity() )
            {
                distanceImage[ij.x()][ij.y()] = 1u;
                validResBoundingBox.add( cvf::Vec3d( cellCenterPosition( ij.x(), ij.y() ), 0.0 ) );
            }
            else
            {
                mapCellVisibility[cellIndex] = false;
            }
        }

        if ( m_limitToPorePressureRegions && m_paddingAroundPorePressureRegion > 0.0 )
        {
            RiaImageTools::distanceTransform2d( distanceImage );

            cvf::Vec3d porExtent   = validResBoundingBox.extent();
            double     radius      = std::max( porExtent.x(), porExtent.y() ) * 0.25;
            double     expansion   = m_paddingAroundPorePressureRegion * radius;
            size_t     cellPadding = std::ceil( expansion / sampleSpacing() );
            for ( size_t cellIndex = 0; cellIndex < cellResults.size(); ++cellIndex )
            {
                if ( !mapCellVisibility[cellIndex] )
                {
                    cvf::Vec2ui ij = ijFromCellIndex( cellIndex );
                    if ( distanceImage[ij.x()][ij.y()] < cellPadding * cellPadding )
                    {
                        mapCellVisibility[cellIndex] = true;
                    }
                }
            }
        }
    }
    return mapCellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::retrieveParameterWeights()
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::generateResults( int timeStep )
{
    RimGeoMechCellColors* cellColors    = view()->cellResult();
    RigFemResultAddress   resultAddress = cellColors->resultAddress();

    std::vector<double> aggregatedResults = generateResultsFromAddress( resultAddress, m_mapCellVisibility, timeStep );

    return aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::generateResultsFromAddress( RigFemResultAddress      resultAddress,
                                                                                const std::vector<bool>& mapCellVisibility,
                                                                                int                      timeStep )
{
    RigGeoMechCaseData*          caseData         = geoMechCase()->geoMechData();
    RigFemPartResultsCollection* resultCollection = caseData->femPartResults();
    size_t                       nCells           = numberOfCells();
    std::vector<double> aggregatedResults = std::vector<double>( nCells, std::numeric_limits<double>::infinity() );

    bool wasInvalid = false;
    if ( !resultAddress.isValid() )
    {
        wasInvalid    = true;
        resultAddress = RigFemResultAddress( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "POR-Bar", "" );
    }

    if ( resultAddress.fieldName == "PP" )
    {
        resultAddress.fieldName = "POR-Bar"; // More likely to be in memory than POR
    }
    if ( resultAddress.fieldName == "POR-Bar" )
    {
        resultAddress.resultPosType = RIG_ELEMENT_NODAL;
    }
    else if ( resultAddress.resultPosType == RIG_FORMATION_NAMES )
    {
        resultAddress.resultPosType = RIG_ELEMENT_NODAL; // formation indices are stored per element node result.
    }

    std::vector<float> resultValuesF = resultCollection->resultValues( resultAddress, 0, timeStep );
    if ( resultValuesF.empty() ) return aggregatedResults;

    std::vector<double> resultValues = gridCellValues( resultAddress, resultValuesF );

    if ( wasInvalid )
    {
        // For invalid result addresses we just use the POR-Bar result to get the reservoir region
        // And display a dummy 0-result in the region.
        for ( double& value : resultValues )
        {
            if ( value != std::numeric_limits<double>::infinity() )
            {
                value = 0.0;
            }
        }
    }

#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nCells ); ++index )
    {
        if ( mapCellVisibility.empty() || mapCellVisibility[index] )
        {
            cvf::Vec2ui ij           = ijFromCellIndex( index );
            aggregatedResults[index] = calculateValueInMapCell( ij.x(), ij.y(), resultValues );
        }
    }

    return aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapProjection::resultVariableChanged() const
{
    RimGeoMechCellColors* cellColors = view()->cellResult();
    RigFemResultAddress   resAddr    = cellColors->resultAddress();

    return !m_currentResultAddr.isValid() || !( m_currentResultAddr == resAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::clearResultVariable()
{
    m_currentResultAddr = RigFemResultAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimGeoMechContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimGeoMechContourMapProjection::findIntersectingCells( const cvf::BoundingBox& bbox ) const
{
    std::vector<size_t> allCellIndices;
    m_femPart->findIntersectingCellsWithExistingSearchTree( bbox, &allCellIndices );
    return allCellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGeoMechContourMapProjection::kLayer( size_t globalCellIdx ) const
{
    size_t i, j, k;
    m_femPartGrid->ijkFromCellIndex( globalCellIdx, &i, &j, &k );
    return k;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGeoMechContourMapProjection::kLayers() const
{
    return m_kLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;
    m_femPartGrid->cellCornerVertices( globalCellIdx, hexCorners.data() );

    cvf::BoundingBox          overlapBBox;
    std::array<cvf::Vec3d, 8> overlapCorners;
    if ( RigCellGeometryTools::estimateHexOverlapWithBoundingBox( hexCorners, bbox, &overlapCorners, &overlapBBox ) )
    {
        double overlapVolume = RigCellGeometryTools::calculateCellVolume( overlapCorners );
        return overlapVolume;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::calculateRayLengthInCell( size_t            globalCellIdx,
                                                                 const cvf::Vec3d& highestPoint,
                                                                 const cvf::Vec3d& lowestPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities( globalCellIdx );

    hexCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
    hexCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
    hexCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
    hexCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
    hexCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
    hexCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
    hexCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
    hexCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

    std::vector<HexIntersectionInfo> intersections;

    if ( RigHexIntersectionTools::lineHexCellIntersection( highestPoint, lowestPoint, hexCorners.data(), 0, &intersections ) )
    {
        double lengthInCell =
            ( intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint ).length();
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::getParameterWeightForCell( size_t                     globalCellIdx,
                                                                  const std::vector<double>& parameterWeights ) const
{
    if ( parameterWeights.empty() ) return 1.0;

    return parameterWeights[globalCellIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::gridCellValues( RigFemResultAddress resAddr,
                                                                    std::vector<float>& resultValues ) const
{
    std::vector<double> gridCellValues( m_femPart->elementCount(), std::numeric_limits<double>::infinity() );
    for ( size_t globalCellIdx = 0; globalCellIdx < static_cast<size_t>( m_femPart->elementCount() ); ++globalCellIdx )
    {
        RigElementType elmType = m_femPart->elementType( globalCellIdx );
        if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

        if ( resAddr.resultPosType == RIG_ELEMENT )
        {
            gridCellValues[globalCellIdx] = static_cast<double>( resultValues[globalCellIdx] );
        }
        else if ( resAddr.resultPosType == RIG_ELEMENT_NODAL )
        {
            RiaWeightedMeanCalculator<float> cellAverage;
            for ( int i = 0; i < 8; ++i )
            {
                size_t gridResultValueIdx = m_femPart->resultValueIdxFromResultPosType( resAddr.resultPosType,
                                                                                        static_cast<int>( globalCellIdx ),
                                                                                        i );
                cellAverage.addValueAndWeight( resultValues[gridResultValueIdx], 1.0 );
            }

            gridCellValues[globalCellIdx] = static_cast<double>( cellAverage.weightedMean() );
        }
        else
        {
            RiaWeightedMeanCalculator<float> cellAverage;
            const int*                       elmNodeIndices = m_femPart->connectivities( globalCellIdx );
            for ( int i = 0; i < 8; ++i )
            {
                cellAverage.addValueAndWeight( resultValues[elmNodeIndices[i]], 1.0 );
            }
            gridCellValues[globalCellIdx] = static_cast<double>( cellAverage.weightedMean() );
        }
    }
    return gridCellValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechContourMapProjection::geoMechCase() const
{
    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType( geoMechCase );
    return geoMechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RimGeoMechContourMapProjection::view() const
{
    RimGeoMechContourMapView* view = nullptr;
    firstAncestorOrThisOfTypeAsserted( view );
    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateAfterResultGeneration( int timeStep )
{
    m_currentResultTimestep = timeStep;

    RimGeoMechCellColors* cellColors = view()->cellResult();
    RigFemResultAddress   resAddr    = cellColors->resultAddress();
    m_currentResultAddr              = resAddr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                       const QVariant&            oldValue,
                                                       const QVariant&            newValue )
{
    RimContourMapProjection::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_limitToPorePressureRegions || changedField == &m_applyPPRegionLimitVertically ||
         changedField == &m_paddingAroundPorePressureRegion )
    {
        clearGridMapping();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGeoMechContourMapProjection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                           bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultAggregation )
    {
        std::vector<ResultAggregationEnum> validOptions = { RESULTS_TOP_VALUE,
                                                            RESULTS_MEAN_VALUE,
                                                            RESULTS_GEOM_VALUE,
                                                            RESULTS_HARM_VALUE,
                                                            RESULTS_MIN_VALUE,
                                                            RESULTS_MAX_VALUE,
                                                            RESULTS_SUM };

        for ( ResultAggregationEnum option : validOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( ResultAggregation::uiText( option ), option ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimContourMapProjection::defineUiOrdering( uiConfigName, uiOrdering );
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Map Boundaries" );
    group->add( &m_limitToPorePressureRegions );
    group->add( &m_applyPPRegionLimitVertically );
    group->add( &m_paddingAroundPorePressureRegion );
    m_applyPPRegionLimitVertically.uiCapability()->setUiReadOnly( !m_limitToPorePressureRegions() );
    m_paddingAroundPorePressureRegion.uiCapability()->setUiReadOnly( !m_limitToPorePressureRegions() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                            QString                    uiConfigName,
                                                            caf::PdmUiEditorAttribute* attribute )
{
    RimContourMapProjection::defineEditorAttribute( field, uiConfigName, attribute );
    if ( field == &m_paddingAroundPorePressureRegion )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum                       = 0.0;
            myAttr->m_maximum                       = 2.0;
            myAttr->m_sliderTickCount               = 4;
            myAttr->m_delaySliderUpdateUntilRelease = true;
        }
    }
}
