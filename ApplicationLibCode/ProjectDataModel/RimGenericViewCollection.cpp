/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimGenericViewCollection.h"

#include "RiaPreferences.h"

#include "QuickAccess/RimQuickAccessCollection.h"
#include "RimCase.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeneric3dView.h"
#include "RimStimPlanColors.h"

#include "cafPdmDocument.h"
#include "cafPdmUiTreeOrdering.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimGenericViewCollection, "GenericViewCollection", "GenericViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericViewCollection::RimGenericViewCollection()
{
    CAF_PDM_InitObject( "Views", ":/3DView16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseViews, "Views", "Views" );
    CAF_PDM_InitFieldNoDefault( &m_genericViews, "GenericViews", "Views" );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericViewCollection::~RimGenericViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericViewCollection::isEmpty() const
{
    return !m_eclipseViews.hasChildren() && !m_genericViews.hasChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RimGenericViewCollection::views() const
{
    return m_eclipseViews.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimGenericViewCollection::addView( RimEclipseCase* eclipseCase )
{
    RimEclipseView* view = new RimEclipseView();

    view->setEclipseCase( eclipseCase );

    auto prefs = RiaPreferences::current();
    view->faultCollection()->setActive( prefs->enableFaultsByDefault() );

    view->cellEdgeResult()->setResultVariable( "MULT" );
    view->cellEdgeResult()->setActive( false );
    view->fractureColors()->setDefaultResultName();

    caf::PdmDocument::updateUiIconStateRecursively( view );

    m_eclipseViews.push_back( view );

    RimQuickAccessCollection::instance()->addQuickAccessFields( view );

    view->loadDataAndUpdate();

    // Set default values
    if ( view->currentGridCellResults() )
    {
        auto defaultResult = view->currentGridCellResults()->defaultResult();
        view->cellResult()->setFromEclipseResultAddress( defaultResult );
    }

    updateConnectedEditors();

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericViewCollection::addView( RimEclipseView* view )
{
    m_eclipseViews.push_back( view );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericViewCollection::removeView( RimEclipseView* view )
{
    m_eclipseViews.removeChild( view );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeneric3dView* RimGenericViewCollection::addGenericView()
{
    auto* view = new RimGeneric3dView();

    // Add before loadDataAndUpdate() so RimProject::assignViewIdToView() sees the view in the project tree
    m_genericViews.push_back( view );
    caf::PdmDocument::updateUiIconStateRecursively( view );

    view->loadDataAndUpdate();

    updateConnectedEditors();

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeneric3dView*> RimGenericViewCollection::genericViews() const
{
    return m_genericViews.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    // Display Eclipse views and generic 3D views together as a single flat list.
    for ( auto view : m_eclipseViews.childrenByType() )
    {
        uiTreeOrdering.add( view );
    }

    for ( auto view : m_genericViews.childrenByType() )
    {
        uiTreeOrdering.add( view );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericViewCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( auto parentCase = firstAncestorOrThisOfType<RimCase>() )
    {
        parentCase->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericViewCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewViewFeature";
    menuBuilder << "RicNewGenericDataViewFeature";
}
