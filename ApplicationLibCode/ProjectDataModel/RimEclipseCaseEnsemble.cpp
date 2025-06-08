/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Eqinor ASA
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

#include "RimEclipseCaseEnsemble.h"

#include "ContourMap/RimStatisticsContourMap.h"
#include "ContourMap/RimStatisticsContourMapView.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimWellTargetMapping.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEclipseCaseEnsemble, "RimEclipseCaseEnsemble" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble::RimEclipseCaseEnsemble()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Grid Ensemble", ":/GridCaseGroup16x16.png", "", "", "EclipseCaseEnsemble", "Grid Ensemble" );

    CAF_PDM_InitScriptableField( &m_groupId, "Id", -1, "Id" );
    m_groupId.uiCapability()->setUiReadOnly( true );
    m_groupId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_caseCollection, "CaseCollection", "Ensemble Cases" );
    m_caseCollection = new RimCaseCollection;
    m_caseCollection->uiCapability()->setUiName( "Realizations" );
    m_caseCollection->uiCapability()->setUiIconFromResourceString( ":/Cases16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_viewCollection, "ViewCollection", "Views" );
    m_viewCollection = new RimEclipseViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_wellTargetMappings, "WellTargetMappings", "Well Target Mappings" );

    CAF_PDM_InitFieldNoDefault( &m_statisticsContourMaps, "StatisticsContourMaps", "Statistics Contour maps" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble::~RimEclipseCaseEnsemble()
{
    delete m_caseCollection;
    m_caseCollection = nullptr;

    delete m_viewCollection;
    m_viewCollection = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::addCase( RimEclipseCase* reservoir )
{
    CVF_ASSERT( reservoir );

    m_caseCollection()->reservoirs().push_back( reservoir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::removeCase( RimEclipseCase* reservoir )
{
    if ( m_caseCollection()->reservoirs().count( reservoir ) == 0 ) return;

    m_caseCollection()->reservoirs().removeChild( reservoir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCaseEnsemble::contains( RimEclipseCase* reservoir ) const
{
    CVF_ASSERT( reservoir );

    for ( RimEclipseCase* rimReservoir : cases() )
    {
        if ( reservoir->gridFileName() == rimReservoir->gridFileName() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseCaseEnsemble::findByDescription( const QString& caseDescription ) const
{
    if ( !m_caseCollection ) return nullptr;

    return m_caseCollection->findByDescription( caseDescription );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseCaseEnsemble::findByFileName( const QString& gridFileName ) const
{
    for ( RimEclipseCase* rimReservoir : cases() )
    {
        if ( gridFileName == rimReservoir->gridFileName() ) return rimReservoir;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimEclipseCaseEnsemble::cases() const
{
    if ( !m_caseCollection ) return {};

    return m_caseCollection->reservoirs.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimEclipseCase*> RimEclipseCaseEnsemble::casesInViews() const
{
    if ( !m_caseCollection ) return {};
    if ( !m_viewCollection || m_viewCollection->isEmpty() ) return {};

    std::set<RimEclipseCase*> retCases;

    for ( auto view : m_viewCollection->views() )
    {
        if ( view->eclipseCase() != nullptr ) retCases.insert( view->eclipseCase() );
    }

    return retCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellTargetMapping*> RimEclipseCaseEnsemble::wellTargetMappings() const
{
    return m_wellTargetMappings.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::addView( RimEclipseView* view )
{
    m_viewCollection->addView( view );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCaseEnsemble::addViewForCase( RimEclipseCase* eclipseCase )
{
    return m_viewCollection->addView( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewViewForGridEnsembleFeature";

    // Hide this feature for the 2025.04 release, not ready for production use yet.
    // menuBuilder << "RicNewWellTargetMappingFeature";

    menuBuilder << "RicNewStatisticsContourMapFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RimEclipseCaseEnsemble::viewCollection() const
{
    return m_viewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RimEclipseCaseEnsemble::allViews() const
{
    std::vector<RimEclipseView*> retList;
    if ( !viewCollection() ) return retList;

    for ( auto view : viewCollection()->views() )
    {
        retList.push_back( view );
    }

    for ( auto cmap : m_statisticsContourMaps.childrenByType() )
    {
        for ( auto view : cmap->views() )
        {
            retList.push_back( view );
        }
    }

    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::addWellTargetMapping( RimWellTargetMapping* wellTargetMapping )
{
    m_wellTargetMappings.push_back( wellTargetMapping );
    wellTargetMapping->updateResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap )
{
    m_statisticsContourMaps.push_back( statisticsContourMap );
    statisticsContourMap->setName( QString( "Ensemble Contour Map #%1" ).arg( m_statisticsContourMaps.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.skipRemainingFields();
}
