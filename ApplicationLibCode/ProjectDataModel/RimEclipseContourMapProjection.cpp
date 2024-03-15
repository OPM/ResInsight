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

#include "RimEclipseContourMapProjection.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
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

CAF_PDM_SOURCE_INIT( RimEclipseContourMapProjection, "RimEclipseContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::RimEclipseContourMapProjection()
    : RimContourMapProjection()
    , m_kLayers( 0u )
    , m_useActiveCellInfo( true )
{
    CAF_PDM_InitObject( "RimEclipseContourMapProjection", ":/2DMapProjection16x16.png" );

    CAF_PDM_InitField( &m_weightByParameter, "WeightByParameter", false, "Weight by Result Parameter" );
    CAF_PDM_InitFieldNoDefault( &m_weightingResult, "WeightingResult", "" );
    m_weightingResult.uiCapability()->setUiTreeChildrenHidden( true );
    m_weightingResult = new RimEclipseResultDefinition;
    m_weightingResult->findField( "MResultType" )->uiCapability()->setUiName( "Result Type" );

    setName( "Map Projection" );
    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::~RimEclipseContourMapProjection()
{
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::resultDescriptionText() const
{
    QString resultText = resultAggregationText();
    if ( !isColumnResult() )
    {
        resultText += QString( ", %1" ).arg( view()->cellResult()->resultVariable() );
    }

    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::weightingParameter() const
{
    QString parameter = "None";
    if ( m_weightByParameter() && !m_weightingResult->isTernarySaturationSelected() )
    {
        parameter = m_weightingResult->resultVariableUiShortName();
    }
    return parameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEclipseContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateLegend()
{
    RimEclipseCellColors* cellColors = view()->cellResult();

    double minVal = minValue( m_aggregatedResults );
    double maxVal = maxValue( m_aggregatedResults );

    auto [minValAllTimeSteps, maxValAllTimeSteps] = minmaxValuesAllTimeSteps();

    legendConfig()->setAutomaticRanges( minValAllTimeSteps, maxValAllTimeSteps, minVal, maxVal );

    if ( m_resultAggregation() == RESULTS_OIL_COLUMN || m_resultAggregation() == RESULTS_GAS_COLUMN ||
         m_resultAggregation() == RESULTS_HC_COLUMN )
    {
        legendConfig()->setTitle( QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() ) );
    }
    else
    {
        QString projectionLegendText = QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() );
        if ( weightingParameter() != "None" )
        {
            projectionLegendText += QString( "(W: %1)" ).arg( weightingParameter() );
        }
        projectionLegendText += QString( "\nResult: %1" ).arg( cellColors->resultVariableUiShortName() );

        legendConfig()->setTitle( projectionLegendText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::sampleSpacing() const
{
    if ( m_mainGrid.notNull() )
    {
        return m_relativeSampleSpacing * m_mainGrid->characteristicIJCellSize();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::clearGridMappingAndRedraw()
{
    clearGridMapping();
    updateConnectedEditors();
    generateResultsIfNecessary( view()->currentTimeStep() );
    updateLegend();

    RimEclipseView* parentView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();
    parentView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEclipseContourMapProjection::generateResults( int timeStep )
{
    m_weightingResult->loadResult();

    size_t nCells = numberOfCells();

    std::vector<double> aggregatedResults = std::vector<double>( nCells, std::numeric_limits<double>::infinity() );

    RimEclipseCellColors* cellColors     = view()->cellResult();
    auto                  gridCellResult = view()->currentGridCellResults();
    {
        {
            auto resultAdr = cellColors->eclipseResultAddress();
            if ( resultAdr.isValid() && gridCellResult->hasResultEntry( resultAdr ) )
                m_useActiveCellInfo = gridCellResult->isUsingGlobalActiveIndex( resultAdr );
        }

        if ( !cellColors->isTernarySaturationSelected() )
        {
            std::vector<double> gridResultValues;
            if ( isColumnResult() )
            {
                m_currentResultName = "";
                gridCellResult->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" ) );
                gridCellResult->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );
                gridCellResult->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );
                if ( m_resultAggregation == RESULTS_OIL_COLUMN || m_resultAggregation == RESULTS_HC_COLUMN )
                {
                    gridCellResult->ensureKnownResultLoaded(
                        RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) );
                }
                if ( m_resultAggregation == RESULTS_GAS_COLUMN || m_resultAggregation == RESULTS_HC_COLUMN )
                {
                    gridCellResult->ensureKnownResultLoaded(
                        RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );
                }
                gridResultValues = calculateColumnResult( m_resultAggregation() );
            }
            else
            {
                if ( cellColors->hasStaticResult() && timeStep > 0 ) timeStep = 0;

                m_currentResultName = cellColors->resultVariable();
                RigEclipseResultAddress resAddr( cellColors->resultType(),
                                                 cellColors->resultVariable(),
                                                 cellColors->timeLapseBaseTimeStep(),
                                                 cellColors->caseDiffIndex() );

                // When loading a project file, grid calculator results are not computed the first time this function is
                // called. Must check if result is loaded. See RimReloadCaseTools::updateAll3dViews()
                if ( resAddr.isValid() && gridCellResult->hasResultEntry( resAddr ) && gridCellResult->isResultLoaded( resAddr ) )
                {
                    gridResultValues = gridCellResult->cellScalarResults( resAddr, timeStep );
                }
            }

            if ( !gridResultValues.empty() )
            {
#pragma omp parallel for
                for ( int index = 0; index < static_cast<int>( nCells ); ++index )
                {
                    cvf::Vec2ui ij           = ijFromCellIndex( index );
                    aggregatedResults[index] = calculateValueInMapCell( ij.x(), ij.y(), gridResultValues );
                }
            }
        }
    }
    return aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapProjection::resultVariableChanged() const
{
    if ( !m_currentResultName.isEmpty() )
    {
        RimEclipseCellColors* cellColors = view()->cellResult();
        if ( cellColors->resultVariable() != m_currentResultName )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::clearResultVariable()
{
    m_currentResultName = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEclipseContourMapProjection::calculateColumnResult( ResultAggregation resultAggregation ) const
{
    const RigCaseCellResultsData* resultData = eclipseCase()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    bool hasPoroResult = resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" ) );
    bool hasNtgResult  = resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );
    bool hasDzResult   = resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );

    if ( !( hasPoroResult && hasNtgResult && hasDzResult ) )
    {
        return std::vector<double>();
    }

    const std::vector<double>& poroResults =
        resultData->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" ), 0 );
    const std::vector<double>& ntgResults =
        resultData->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ), 0 );
    const std::vector<double>& dzResults =
        resultData->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ), 0 );

    CVF_ASSERT( poroResults.size() == ntgResults.size() && ntgResults.size() == dzResults.size() );

    int timeStep = view()->currentTimeStep();

    std::vector<double> resultValues( poroResults.size(), 0.0 );

    if ( resultAggregation == RESULTS_OIL_COLUMN || resultAggregation == RESULTS_HC_COLUMN )
    {
        const std::vector<double>& soilResults =
            resultData->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ),
                                           timeStep );
        for ( size_t cellResultIdx = 0; cellResultIdx < resultValues.size(); ++cellResultIdx )
        {
            resultValues[cellResultIdx] = soilResults[cellResultIdx];
        }
    }

    if ( resultAggregation == RESULTS_GAS_COLUMN || resultAggregation == RESULTS_HC_COLUMN )
    {
        bool hasGasResult =
            resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );
        if ( hasGasResult )
        {
            const std::vector<double>& sgasResults =
                resultData->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ),
                                               timeStep );
            for ( size_t cellResultIdx = 0; cellResultIdx < resultValues.size(); ++cellResultIdx )
            {
                resultValues[cellResultIdx] += sgasResults[cellResultIdx];
            }
        }
    }

    for ( size_t cellResultIdx = 0; cellResultIdx < resultValues.size(); ++cellResultIdx )
    {
        resultValues[cellResultIdx] *= poroResults[cellResultIdx] * ntgResults[cellResultIdx] * dzResults[cellResultIdx];
    }
    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateGridInformation()
{
    auto eclipseCase = this->eclipseCase();
    m_mainGrid       = eclipseCase->eclipseCaseData()->mainGrid();
    m_activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    m_kLayers        = m_mainGrid->cellCountK();

    m_gridBoundingBox           = eclipseCase->activeCellsBoundingBox();
    cvf::Vec3d minExpandedPoint = m_gridBoundingBox.min() - cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    cvf::Vec3d maxExpandedPoint = m_gridBoundingBox.max() + cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    m_expandedBoundingBox       = cvf::BoundingBox( minExpandedPoint, maxExpandedPoint );

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
std::vector<double> RimEclipseContourMapProjection::retrieveParameterWeights()
{
    std::vector<double> weights;
    if ( m_weightByParameter() )
    {
        RigEclipseResultAddress gridScalarResultIdx = m_weightingResult->eclipseResultAddress();
        if ( gridScalarResultIdx.isValid() )
        {
            m_weightingResult->loadResult();
            int timeStep = 0;
            if ( m_weightingResult->hasDynamicResult() )
            {
                timeStep = view()->currentTimeStep();
            }
            weights = m_weightingResult->currentGridCellResults()->cellScalarResults( gridScalarResultIdx, timeStep );
        }
    }
    return weights;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseContourMapProjection::eclipseCase() const
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( !view ) return nullptr;

    return dynamic_cast<RimEclipseCase*>( view->ownerCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimEclipseContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimEclipseContourMapProjection::findIntersectingCells( const cvf::BoundingBox& bbox ) const
{
    return m_mainGrid->findIntersectingCells( bbox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEclipseContourMapProjection::kLayer( size_t globalCellIdx ) const
{
    const RigCell& cell            = m_mainGrid->globalCellArray()[globalCellIdx];
    size_t         mainGridCellIdx = cell.mainGridCellIndex();
    size_t         i, j, k;
    m_mainGrid->ijkFromCellIndex( mainGridCellIdx, &i, &j, &k );
    return k;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEclipseContourMapProjection::kLayers() const
{
    return m_kLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const RigCell& cell = m_mainGrid->globalCellArray()[globalCellIdx];

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices( localCellIdx, hexCorners.data() );

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
double RimEclipseContourMapProjection::calculateRayLengthInCell( size_t            globalCellIdx,
                                                                 const cvf::Vec3d& highestPoint,
                                                                 const cvf::Vec3d& lowestPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    RigCell cell = m_mainGrid->globalCellArray()[globalCellIdx];

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices( localCellIdx, hexCorners.data() );
    std::vector<HexIntersectionInfo> intersections;

    if ( RigHexIntersectionTools::lineHexCellIntersection( highestPoint, lowestPoint, hexCorners.data(), 0, &intersections ) )
    {
        double lengthInCell = ( intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint ).length();
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::getParameterWeightForCell( size_t cellResultIdx, const std::vector<double>& cellWeights ) const
{
    if ( cellWeights.empty() ) return 1.0;

    double result = std::max( cellWeights[cellResultIdx], 0.0 );
    if ( result < 1.0e-6 )
    {
        result = 0.0;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEclipseContourMapProjection::gridResultIndex( size_t globalCellIdx ) const
{
    if ( m_useActiveCellInfo ) return m_activeCellInfo->cellResultIndex( globalCellIdx );

    return globalCellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RimEclipseContourMapProjection::view() const
{
    return firstAncestorOrThisOfTypeAsserted<RimEclipseContourMapView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateAfterResultGeneration( int timeStep )
{
    m_currentResultTimestep = timeStep;

    RimEclipseCellColors* cellColors = view()->cellResult();
    m_currentResultName              = cellColors->resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimContourMapProjection::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_weightByParameter || changedField == &m_weightingResult )
    {
        clearGridMapping();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimContourMapProjection::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* weightingGroup = uiOrdering.addNewGroup( "Mean Weighting Options" );
    weightingGroup->add( &m_weightByParameter );
    weightingGroup->setCollapsedByDefault();

    m_weightByParameter.uiCapability()->setUiReadOnly( !isMeanResult() );
    if ( !isMeanResult() )
    {
        m_weightByParameter = false;
    }

    if ( m_weightByParameter() )
    {
        m_weightingResult->uiOrdering( uiConfigName, *weightingGroup );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::initAfterRead()
{
    RimContourMapProjection::initAfterRead();
    if ( eclipseCase() )
    {
        m_weightingResult->setEclipseCase( eclipseCase() );
    }
}
