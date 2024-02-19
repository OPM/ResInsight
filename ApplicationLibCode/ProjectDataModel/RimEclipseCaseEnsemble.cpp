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

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"

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
    m_caseCollection->uiCapability()->setUiName( "Cases" );
    m_caseCollection->uiCapability()->setUiIconFromResourceString( ":/Cases16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_selectedCase, "SelectedCase", "Selected Case" );

    CAF_PDM_InitFieldNoDefault( &m_viewCollection, "ViewCollection", "Views" );
    m_viewCollection = new RimEclipseViewCollection;

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
std::vector<RimEclipseCase*> RimEclipseCaseEnsemble::cases() const
{
    return m_caseCollection->reservoirs.childrenByType();
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
QList<caf::PdmOptionItemInfo> RimEclipseCaseEnsemble::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedCase )
    {
        for ( auto eclCase : cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( eclCase->caseUserDescription(), eclCase, false, eclCase->uiIconProvider() ) );
        }

        return options;
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_selectedCase )
    {
        for ( auto view : m_viewCollection->views() )
        {
            view->setEclipseCase( m_selectedCase() );
            view->loadDataAndUpdate();
            view->updateGridBoxData();
            view->updateAnnotationItems();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RimEclipseCaseEnsemble::viewCollection() const
{
    return m_viewCollection;
}
