/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicLinkVisibleViewsFeature.h"

#include "RicLinkVisibleViewsFeatureUi.h"

#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QTreeView>

CAF_CMD_SOURCE_INIT( RicLinkVisibleViewsFeature, "RicLinkVisibleViewsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkVisibleViewsFeature::isCommandEnabled() const
{
    RimProject* proj = RimProject::current();
    if ( !proj ) return false;

    std::vector<Rim3dView*> linkedviews;
    if ( proj->viewLinkerCollection() && proj->viewLinkerCollection()->viewLinker() )
    {
        linkedviews = proj->viewLinkerCollection()->viewLinker()->allViews();
    }

    std::vector<Rim3dView*> visibleViews = proj->allVisibleViews();
    if ( visibleViews.size() >= 2 && ( linkedviews.size() < visibleViews.size() ) )
    {
        return !findLinkableVisibleViews().empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::onActionTriggered( bool isChecked )
{
    auto linkableViews = findLinkableVisibleViews();
    RicLinkVisibleViewsFeature::linkViews( linkableViews );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Link Visible Views" );
    actionToSetup->setIcon( QIcon( ":/LinkView.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RicLinkVisibleViewsFeature::findLinkableVisibleViews()
{
    RimProject* proj = RimProject::current();

    std::vector<Rim3dView*> views;
    for ( auto gridView : proj->allVisibleViews() )
    {
        if ( gridView && !gridView->assosiatedViewLinker() ) views.push_back( gridView );
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::linkViews( std::vector<Rim3dView*>& linkableViews )
{
    if ( linkableViews.empty() ) return;

    RimProject*    proj       = RimProject::current();
    RimViewLinker* viewLinker = proj->viewLinkerCollection->viewLinker();

    if ( !viewLinker )
    {
        viewLinker = new RimViewLinker;

        proj->viewLinkerCollection()->viewLinker = viewLinker;
        viewLinker->setMasterView( linkableViews.front() );
    }

    Rim3dView* primaryView = viewLinker->masterView();

    auto matchingViews = RicLinkVisibleViewsFeature::matchingViews( primaryView, linkableViews );
    for ( auto v : matchingViews )
    {
        viewLinker->addDependentView( v );
    }

    viewLinker->updateDependentViews();
    viewLinker->updateUiNameAndIcon();

    proj->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    proj->viewLinkerCollection->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded( proj->viewLinkerCollection() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RicLinkVisibleViewsFeature::matchingViews( Rim3dView* primaryView, std::vector<Rim3dView*>& candidates )
{
    if ( !primaryView ) return {};

    std::vector<Rim3dView*> matchingViews;

    RiaDefines::View3dContent primaryContent = primaryView->viewContent();
    if ( primaryContent == RiaDefines::View3dContent::FLAT_INTERSECTION )
    {
        for ( auto v : candidates )
        {
            if ( v != primaryView && v->viewContent() == RiaDefines::View3dContent::FLAT_INTERSECTION ) matchingViews.emplace_back( v );
        }

        return matchingViews;
    }

    // We have a 3D view or contour map as primary view, include all views except flat intersection views
    for ( auto v : candidates )
    {
        if ( v != primaryView && v->viewContent() != RiaDefines::View3dContent::FLAT_INTERSECTION ) matchingViews.emplace_back( v );
    }

    return matchingViews;
}
