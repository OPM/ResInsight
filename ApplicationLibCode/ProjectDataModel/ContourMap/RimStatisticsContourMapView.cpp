/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimStatisticsContourMapView.h"

#include "RiuViewer.h"

#include "RivContourMapProjectionPartMgr.h"

#include "Polygons/RimPolygonInViewCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimFaultInViewCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimStatisticsContourMap.h"
#include "RimStatisticsContourMapProjection.h"
#include "RimViewLinker.h"
#include "RimViewNameConfig.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT( RimStatisticsContourMapView, "RimStatisticsContourMapView" );

RimStatisticsContourMapView::RimStatisticsContourMapView()
    : RimEclipseContourMapView()
{
    CAF_PDM_InitObject( "Contour Map View", ":/2DMap16x16.png", "", "", "StatisticsContourMap", "A contour map for Eclipse cases" );

    CAF_PDM_InitFieldNoDefault( &m_statisticsContourMap, "StatisticsContourMap", "Statistics Contour Map" );

    m_contourMapProjection = new RimStatisticsContourMapProjection();

    setFaultVisParameters();
    setDefaultCustomName();

    m_contourMapProjectionPartMgr = new RivContourMapProjectionPartMgr( contourMapProjection() );

    cellResult()->setTernaryEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapView::setStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap )
{
    m_statisticsContourMap = statisticsContourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapView::createAutoName() const
{
    QStringList autoName;

    if ( !nameConfig()->customName().isEmpty() )
    {
        autoName.push_back( nameConfig()->customName() );
    }

    QStringList generatedAutoTags;

    if ( nameConfig()->addCaseName() && m_statisticsContourMap->ensemble() )
    {
        generatedAutoTags.push_back( m_statisticsContourMap->ensemble()->name() );
    }

    if ( nameConfig()->addAggregationType() )
    {
        generatedAutoTags.push_back( m_statisticsContourMap->resultAggregationText() );
    }

    if ( nameConfig()->addProperty() )
    {
        if ( auto proj = dynamic_cast<RimStatisticsContourMapProjection*>( m_contourMapProjection().p() ) )
        {
            if ( !m_statisticsContourMap->isColumnResult() )
            {
                generatedAutoTags.push_back( m_statisticsContourMap->resultVariable() );
            }
            generatedAutoTags.push_back( proj->statisticsType() );
        }
    }

    if ( nameConfig()->addSampleSpacing() )
    {
        generatedAutoTags.push_back( QString( "%1" ).arg( m_statisticsContourMap->sampleSpacingFactor(), 3, 'f', 2 ) );
    }

    if ( !generatedAutoTags.empty() )
    {
        autoName.push_back( generatedAutoTags.join( ", " ) );
    }
    return autoName.join( ": " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapView::setDefaultCustomName()
{
    nameConfig()->setCustomName( "" );
    nameConfig()->hideCaseNameField( false );
    nameConfig()->hideAggregationTypeField( false );
    nameConfig()->hidePropertyField( false );
    nameConfig()->hideSampleSpacingField( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Contour Map Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );

    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup( "Viewer" );
    viewGroup->add( userDescriptionField() );
    viewGroup->add( backgroundColorField() );
    viewGroup->add( &m_showAxisLines );
    viewGroup->add( &m_showScaleLegend );
    viewGroup->add( &m_showFaultLines );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_contourMapProjection );
    uiTreeOrdering.add( cellResult()->legendConfig() );
    uiTreeOrdering.add( wellCollection() );
    uiTreeOrdering.add( faultCollection() );
    uiTreeOrdering.add( annotationCollection() );
    uiTreeOrdering.add( polygonInViewCollection() );

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMap* RimStatisticsContourMapView::statisticsContourMap() const
{
    return m_statisticsContourMap;
}

//--------------------------------------------------------------------------------------------------
/// Clamp the current timestep to actual possibilities
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMapView::onClampCurrentTimestep()
{
    if ( statisticsContourMap() )
    {
        auto maxSteps = (int)statisticsContourMap()->selectedTimeSteps().size();
        if ( m_currentTimeStep() >= maxSteps )
        {
            m_currentTimeStep = maxSteps - 1;
        }
    }

    if ( m_currentTimeStep < 0 ) m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStatisticsContourMapView::onTimeStepCountRequested()
{
    if ( statisticsContourMap() )
    {
        return statisticsContourMap()->selectedTimeSteps().size();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMapView::timeStepName( int frameIdx ) const
{
    if ( !statisticsContourMap() ) return "";

    auto steps = statisticsContourMap()->selectedTimeSteps();
    if ( frameIdx >= (int)steps.size() ) return "";
    auto realTimeStep = steps[frameIdx];
    return statisticsContourMap()->timeStepName( realTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimStatisticsContourMapView::timeStepStrings() const
{
    QStringList retList;

    if ( !statisticsContourMap() ) return retList;

    for ( auto ts : statisticsContourMap()->selectedTimeSteps() )
    {
        retList.append( statisticsContourMap()->timeStepName( ts ) );
    }

    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimStatisticsContourMapView::activeTimeStepIndices( bool propertyFiltersActive )
{
    std::vector<size_t> timeStepIndices;

    // First entry in this vector is used to define the geometry only result mode with no results.
    timeStepIndices.push_back( 0 );

    // add any timesteps with dynamic data
    if ( !statisticsContourMap() ) return timeStepIndices;

    for ( size_t i = 0; i < statisticsContourMap()->selectedTimeSteps().size(); i++ )
    {
        timeStepIndices.push_back( i );
    }

    return timeStepIndices;
}
