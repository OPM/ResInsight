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

#include "RimIntersectionHandle.h"

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
#include "RivHexGridIntersectionTools.h"

CAF_PDM_SOURCE_INIT( RimIntersectionHandle, "RimIntersectionHandle" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionHandle::RimIntersectionHandle()
{
    CAF_PDM_InitField( &m_name, "UserDescription", QString( "Intersection Name" ), "Name", "", "", "" );
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_showInactiveCells, "ShowInactiveCells", false, "Show Inactive Cells", "", "", "" );
    CAF_PDM_InitField( &m_useSeparateDataSource, "UseSeparateIntersectionDataSource", true, "Enable", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_separateDataSource, "SeparateIntersectionDataSource", "Source", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionHandle::~RimIntersectionHandle() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimIntersectionHandle::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionHandle::setName( const QString& newName )
{
    m_name = newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionHandle::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionHandle::setActive( bool isActive )
{
    m_isActive = isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionHandle::isInactiveCellsVisible() const
{
    return m_showInactiveCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultDefinition* RimIntersectionHandle::activeSeparateResultDefinition()
{
    updateDefaultSeparateDataSource();

    if ( !m_useSeparateDataSource ) return nullptr;

    if ( !m_separateDataSource ) return nullptr;

    if ( !m_separateDataSource->isActive() ) return nullptr;

    RimGridView* view;
    this->firstAncestorOrThisOfTypeAsserted( view );

    if ( !view->separateIntersectionResultsCollection()->isActive() ) return nullptr;

    return m_separateDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimIntersectionHandle::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_separateDataSource )
    {
        RimGridView* view;
        this->firstAncestorOrThisOfTypeAsserted( view );

        std::vector<RimIntersectionResultDefinition*> iResDefs =
            view->separateIntersectionResultsCollection()->intersectionResultsDefinitions();

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
caf::PdmFieldHandle* RimIntersectionHandle::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionHandle::objectToggleField()
{
    return &m_isActive;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionHandle::initAfterRead()
{
    updateDefaultSeparateDataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionHandle::defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    QString inactiveText;
    if ( !this->activeSeparateResultDefinition() )
    {
        inactiveText = " (Inactive)";
    }

    caf::PdmUiGroup* separateResultsGroup = uiOrdering.addNewGroup( "Separate Result Reference" + inactiveText );
    separateResultsGroup->setCollapsedByDefault( true );
    separateResultsGroup->add( &m_useSeparateDataSource );
    separateResultsGroup->add( &m_separateDataSource );
    m_separateDataSource.uiCapability()->setUiReadOnly( !m_useSeparateDataSource() );

    m_separateDataSource.uiCapability()->setUiName( "Source" + inactiveText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionHandle::updateDefaultSeparateDataSource()
{
    if ( m_separateDataSource() == nullptr )
    {
        RimGridView* view;
        this->firstAncestorOrThisOfType( view );

        if ( view )
        {
            std::vector<RimIntersectionResultDefinition*> iResDefs =
                view->separateIntersectionResultsCollection()->intersectionResultsDefinitions();

            if ( iResDefs.size() )
            {
                m_separateDataSource = iResDefs[0];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RimIntersectionHandle::createHexGridInterface()
{
    RimIntersectionResultDefinition* resDef = activeSeparateResultDefinition();
    if ( resDef && resDef->activeCase() )
    {
        // Eclipse case

        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( resDef->activeCase() );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            return new RivEclipseIntersectionGrid( eclipseCase->eclipseCaseData()->mainGrid(),
                                                   eclipseCase->eclipseCaseData()->activeCellInfo(
                                                       resDef->eclipseResultDefinition()->porosityModel() ),
                                                   this->isInactiveCellsVisible() );
        }

        // Geomech case

        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>( resDef->activeCase() );

        if ( geomCase && geomCase->geoMechData() && geomCase->geoMechData()->femParts() )
        {
            RigFemPart* femPart = geomCase->geoMechData()->femParts()->part( 0 );
            return new RivFemIntersectionGrid( femPart );
        }
    }

    RimEclipseView* eclipseView;
    this->firstAncestorOrThisOfType( eclipseView );
    if ( eclipseView )
    {
        RigMainGrid* grid = eclipseView->mainGrid();
        return new RivEclipseIntersectionGrid( grid, eclipseView->currentActiveCellInfo(), this->isInactiveCellsVisible() );
    }

    RimGeoMechView* geoView;
    this->firstAncestorOrThisOfType( geoView );
    if ( geoView && geoView->femParts() && geoView->femParts()->partCount() )
    {
        RigFemPart* femPart = geoView->femParts()->part( 0 );

        return new RivFemIntersectionGrid( femPart );
    }

    return nullptr;
}
