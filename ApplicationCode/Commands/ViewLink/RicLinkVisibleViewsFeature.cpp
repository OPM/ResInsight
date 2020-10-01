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

#include "RimEclipseContourMapView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"

#include "RimGeoMechContourMapView.h"
#include <QAction>
#include <QMessageBox>
#include <QTreeView>

CAF_CMD_SOURCE_INIT( RicLinkVisibleViewsFeature, "RicLinkVisibleViewsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkVisibleViewsFeature::isCommandEnabled()
{
    RimProject*               proj = RimProject::current();
    std::vector<Rim3dView*>   visibleViews;
    std::vector<RimGridView*> linkedviews;
    std::vector<RimGridView*> visibleGridViews;

    proj->allVisibleViews( visibleViews );
    for ( Rim3dView* view : visibleViews )
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>( view );
        if ( gridView ) visibleGridViews.push_back( gridView );
    }

    if ( proj->viewLinkerCollection() && proj->viewLinkerCollection()->viewLinker() )
    {
        proj->viewLinkerCollection()->viewLinker()->allViews( linkedviews );
    }

    if ( visibleGridViews.size() >= 2 && ( linkedviews.size() < visibleGridViews.size() ) )
    {
        std::vector<RimGridView*> views;
        findLinkableVisibleViews( views );
        RicLinkVisibleViewsFeatureUi testUi;
        testUi.setViews( views );
        return !testUi.masterViewCandidates().empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimGridView*> linkableViews;
    findLinkableVisibleViews( linkableViews );

    linkViews( linkableViews );
    return;
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
void RicLinkVisibleViewsFeature::allLinkedViews( std::vector<RimGridView*>& views )
{
    RimProject* proj = RimProject::current();
    if ( proj->viewLinkerCollection()->viewLinker() )
    {
        proj->viewLinkerCollection()->viewLinker()->allViews( views );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::findLinkableVisibleViews( std::vector<RimGridView*>& views )
{
    RimProject* proj = RimProject::current();

    std::vector<RimGridView*> alreadyLinkedViews;
    allLinkedViews( alreadyLinkedViews );

    std::vector<RimGridView*> visibleGridViews;
    proj->allVisibleGridViews( visibleGridViews );

    for ( auto gridView : visibleGridViews )
    {
        if ( dynamic_cast<RimEclipseContourMapView*>( gridView ) ) continue;
        if ( dynamic_cast<RimGeoMechContourMapView*>( gridView ) ) continue;
        if ( gridView->assosiatedViewLinker() ) continue;

        views.push_back( gridView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::linkViews( std::vector<RimGridView*>& linkableViews )
{
    RimProject*    proj       = RimProject::current();
    RimViewLinker* viewLinker = proj->viewLinkerCollection->viewLinker();

    std::vector<RimGridView*> masterCandidates = linkableViews;

    if ( !viewLinker )
    {
        // Create a new view linker

        RimGridView* masterView = masterCandidates.front();

        viewLinker = new RimViewLinker;

        proj->viewLinkerCollection()->viewLinker = viewLinker;
        viewLinker->setMasterView( masterView );
    }

    for ( size_t i = 0; i < linkableViews.size(); i++ )
    {
        RimGridView* rimView = linkableViews[i];
        if ( rimView == viewLinker->masterView() ) continue;

        viewLinker->addDependentView( rimView );
    }

    viewLinker->updateDependentViews();

    viewLinker->updateUiNameAndIcon();

    proj->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    proj->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded( proj->viewLinkerCollection() );
}
