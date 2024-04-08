/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimEclipseViewCollection.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RigCaseCellResultsData.h"

#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"

CAF_PDM_SOURCE_INIT( RimEclipseViewCollection, "EclipseViewCollection", "EclipseViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection::RimEclipseViewCollection()
{
    CAF_PDM_InitObject( "Views", ":/3DView16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_views, "Views", "Eclipse Views" );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection::~RimEclipseViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RimEclipseViewCollection::views() const
{
    return m_views.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseViewCollection::isEmpty()
{
    return !m_views.hasChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseViewCollection::addView( RimEclipseCase* eclipseCase )
{
    RimEclipseView* view = new RimEclipseView();

    view->setEclipseCase( eclipseCase );

    // Set default values
    if ( view->currentGridCellResults() )
    {
        auto defaultResult = view->currentGridCellResults()->defaultResult();
        view->cellResult()->setFromEclipseResultAddress( defaultResult );
    }

    auto prefs = RiaPreferences::current();
    view->faultCollection()->setActive( prefs->enableFaultsByDefault() );

    view->cellEdgeResult()->setResultVariable( "MULT" );
    view->cellEdgeResult()->setActive( false );
    view->fractureColors()->setDefaultResultName();

    caf::PdmDocument::updateUiIconStateRecursively( view );

    m_views.push_back( view );

    view->loadDataAndUpdate();

    updateConnectedEditors();

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseViewCollection::addView( RimEclipseView* view )
{
    m_views.push_back( view );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseViewCollection::removeView( RimEclipseView* view )
{
    m_views.removeChild( view );
    updateConnectedEditors();
}
