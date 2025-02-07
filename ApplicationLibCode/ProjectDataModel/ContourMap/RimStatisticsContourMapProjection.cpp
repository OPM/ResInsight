/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimStatisticsContourMapProjection.h"

#include "ContourMap/RigContourMapCalculator.h"
#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RigStatisticsContourMapProjection.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimStatisticsContourMap.h"
#include "RimStatisticsContourMapView.h"

#include <memory>

CAF_PDM_SOURCE_INIT( RimStatisticsContourMapProjection, "RimStatisticsContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMapProjection::RimStatisticsContourMapProjection()
    : RimContourMapProjection()
{
    CAF_PDM_InitObject( "RimStatisticsContourMapProjection", ":/2DMapProjection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_statisticsType, "StatisticsType", "Statistics Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMapProjection::~RimStatisticsContourMapProjection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapProjection::resultDescriptionText() const
{
    QString resultText;
    if ( auto scm = statisticsContourMap() )
    {
        resultText = scm->resultAggregationText();

        if ( !scm->isColumnResult() )
        {
            resultText += QString( ", %1" ).arg( scm->resultVariable() );
        }
        resultText += ", " + statisticsType();
    }

    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapProjection::statisticsType() const
{
    return m_statisticsType().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapProjection::resultVariableName() const
{
    if ( auto scm = statisticsContourMap() )
    {
        if ( !scm->isColumnResult() )
            return scm->resultVariable() + ", " + statisticsType();
        else
            return scm->resultAggregationText() + ", " + statisticsType();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimStatisticsContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::updateLegend()
{
    auto [minValAllTimeSteps, maxValAllTimeSteps] = minmaxValuesAllTimeSteps();

    double minVal = m_contourMapProjection ? m_contourMapProjection->minValue() : std::numeric_limits<double>::infinity();
    double maxVal = m_contourMapProjection ? m_contourMapProjection->maxValue() : -std::numeric_limits<double>::infinity();

    legendConfig()->setAutomaticRanges( minValAllTimeSteps, maxValAllTimeSteps, minVal, maxVal );

    if ( statisticsContourMap()->isColumnResult() )
    {
        legendConfig()->setTitle( QString( "Map Projection\n%1" ).arg( resultVariableName() ) );
    }
    else
    {
        QString projectionLegendText = QString( "Map Projection\n%1" ).arg( resultAggregationText() );
        projectionLegendText += QString( "\nResult: %1" ).arg( resultVariableName() );

        legendConfig()->setTitle( projectionLegendText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStatisticsContourMapProjection::sampleSpacing() const
{
    if ( RimStatisticsContourMap* contourMap = statisticsContourMap() )
    {
        if ( RigContourMapGrid* contourMapGrid = contourMap->contourMapGrid() )
        {
            return contourMapGrid->sampleSpacing();
        }
    }

    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::clearGridMappingAndRedraw()
{
    clearGridMapping();
    generateResultsIfNecessary( view()->currentTimeStep() );
    updateLegend();
    updateConnectedEditors();

    RimEclipseView* parentView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();
    parentView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStatisticsContourMapProjection::generateResults( int timeStep ) const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::generateAndSaveResults( int timeStep )
{
    if ( auto statistics = statisticsContourMap() )
    {
        dynamic_cast<RigStatisticsContourMapProjection*>( m_contourMapProjection.get() )
            ->generateAndSaveResults( statistics->result( timeStep, m_statisticsType() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStatisticsContourMapProjection::resultVariableChanged() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::clearResultVariable()
{
    m_currentResultName = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::updateGridInformation()
{
    RimStatisticsContourMap* contourMap = statisticsContourMap();
    contourMap->ensureResultsComputed();

    m_contourMapGrid       = std::make_unique<RigContourMapGrid>( *contourMap->contourMapGrid() );
    m_contourMapProjection = std::make_unique<RigStatisticsContourMapProjection>( *m_contourMapGrid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStatisticsContourMapProjection::retrieveParameterWeights()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStatisticsContourMapProjection::eclipseCase() const
{
    auto v = view();
    if ( !v ) return nullptr;

    return dynamic_cast<RimEclipseCase*>( v->ownerCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMap* RimStatisticsContourMapProjection::statisticsContourMap() const
{
    auto v = view();
    if ( !v ) return nullptr;

    return v->statisticsContourMap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimStatisticsContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMapView* RimStatisticsContourMapProjection::view() const
{
    return firstAncestorOrThisOfTypeAsserted<RimStatisticsContourMapView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::updateAfterResultGeneration( int timeStep )
{
    m_currentResultTimestep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimStatisticsContourMapProjection::computeMinMaxValuesAllTimeSteps()
{
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();

    if ( auto map = statisticsContourMap() )
    {
        for ( size_t ts = 0; ts < map->selectedTimeSteps().size(); ts++ )
        {
            std::vector<double> aggregatedResults = statisticsContourMap()->result( ts, m_statisticsType() );
            minimum                               = std::min( minimum, RigContourMapProjection::minValue( aggregatedResults ) );
            maximum                               = std::max( maximum, RigContourMapProjection::maxValue( aggregatedResults ) );
        }
    }

    return std::make_pair( minimum, maximum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_statisticsType );

    appendValueFilterGroup( uiOrdering );

    caf::PdmUiGroup* mainGroup = uiOrdering.addNewGroup( "Projection Settings" );
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
bool RimStatisticsContourMapProjection::isColumnResult() const
{
    return ( statisticsContourMap() && statisticsContourMap()->isColumnResult() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapProjection::resultAggregationText() const
{
    if ( statisticsContourMap() ) return statisticsContourMap()->resultAggregationText();

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    if ( changedField == &m_statisticsType )
    {
        clearGridMappingAndRedraw();
    }
    else
    {
        RimContourMapProjection::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStatisticsContourMapProjection::gridMappingNeedsUpdating() const
{
    if ( !m_contourMapProjection ) return true;

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
