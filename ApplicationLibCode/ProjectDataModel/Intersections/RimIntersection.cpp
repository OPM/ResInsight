/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimIntersection.h"

#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"

#include "RivEclipseIntersectionGrid.h"
#include "RivFemIntersectionGrid.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimIntersection, "RimIntersectionHandle" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersection::RimIntersection()
{
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_showInactiveCells, "ShowInactiveCells", false, "Show Inactive Cells", "", "", "" );
    CAF_PDM_InitField( &m_useSeparateDataSource, "UseSeparateIntersectionDataSource", true, "Enable", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_separateDataSource, "SeparateIntersectionDataSource", "Source", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersection::~RimIntersection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::setActive( bool isActive )
{
    m_isActive = isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersection::isInactiveCellsVisible() const
{
    return m_showInactiveCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultDefinition* RimIntersection::activeSeparateResultDefinition()
{
    updateDefaultSeparateDataSource();

    if ( !m_useSeparateDataSource ) return nullptr;

    if ( !m_separateDataSource ) return nullptr;

    if ( !m_separateDataSource->isActive() ) return nullptr;

    if ( !findSeparateResultsCollection() ) return nullptr;

    if ( !findSeparateResultsCollection()->isActive() ) return nullptr;

    return m_separateDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimIntersection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_separateDataSource )
    {
        std::vector<RimIntersectionResultDefinition*> iResDefs =
            findSeparateResultsCollection()->intersectionResultsDefinitions();

        for ( auto iresdef : iResDefs )
        {
            options.push_back( caf::PdmOptionItemInfo( iresdef->autoName(), iresdef ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimIntersection::findSeparateResultsCollection()
{
    RimGridView* view;
    this->firstAncestorOrThisOfTypeAsserted( view );
    return view->separateIntersectionResultsCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    QString inactiveText;
    if ( !this->activeSeparateResultDefinition() )
    {
        inactiveText = " (Inactive)";
    }

    caf::PdmUiGroup* separateResultsGroup =
        uiOrdering.addNewGroupWithKeyword( "Result Reference" + inactiveText, "SeparateResultReference" );
    separateResultsGroup->setCollapsedByDefault( true );
    separateResultsGroup->add( &m_useSeparateDataSource );
    separateResultsGroup->add( &m_separateDataSource );
    m_separateDataSource.uiCapability()->setUiReadOnly( !m_useSeparateDataSource() );

    m_separateDataSource.uiCapability()->setUiName( "Source" + inactiveText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::updateDefaultSeparateDataSource()
{
    if ( m_separateDataSource() == nullptr )
    {
        RimIntersectionResultsDefinitionCollection* defcoll = findSeparateResultsCollection();

        if ( defcoll )
        {
            std::vector<RimIntersectionResultDefinition*> iResDefs = defcoll->intersectionResultsDefinitions();

            if ( !iResDefs.empty() )
            {
                m_separateDataSource = iResDefs[0];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RimIntersection::createHexGridInterface()
{
    RimGeoMechView* geoView;
    this->firstAncestorOrThisOfType( geoView );

    RimIntersectionResultDefinition* resDef = activeSeparateResultDefinition();
    if ( resDef && resDef->activeCase() )
    {
        // Eclipse case

        auto* eclipseCase = dynamic_cast<RimEclipseCase*>( resDef->activeCase() );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            return new RivEclipseIntersectionGrid( eclipseCase->eclipseCaseData()->mainGrid(),
                                                   eclipseCase->eclipseCaseData()->activeCellInfo(
                                                       resDef->eclipseResultDefinition()->porosityModel() ),
                                                   this->isInactiveCellsVisible() );
        }

        // Geomech case

        auto* geomCase = dynamic_cast<RimGeoMechCase*>( resDef->activeCase() );

        if ( geomCase && geomCase->geoMechData() && geomCase->geoMechData()->femParts() && geoView )
        {
            return new RivFemIntersectionGrid( geomCase->geoMechData()->femParts(), geoView->partsCollection() );
        }
    }

    RimEclipseView* eclipseView;
    this->firstAncestorOrThisOfType( eclipseView );
    if ( eclipseView && eclipseView->mainGrid() )
    {
        RigMainGrid* grid = eclipseView->mainGrid();

        return new RivEclipseIntersectionGrid( grid, eclipseView->currentActiveCellInfo(), this->isInactiveCellsVisible() );
    }

    if ( geoView && geoView->femParts() )
    {
        return new RivFemIntersectionGrid( geoView->femParts(), geoView->partsCollection() );
    }

    return nullptr;
}
