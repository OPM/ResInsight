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

#include "RicLinkViewFeature.h"

#include "RiaApplication.h"

#include "RicLinkVisibleViewsFeature.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicLinkViewFeature, "RicLinkViewFeature" );

class RicLinkViewFeatureImpl
{
public:
    // 1. Selected views in the tree
    // 2. Context menu on a viewer

    bool prepareToExecute()
    {
        auto contextViewer = dynamic_cast<RiuViewer*>( caf::CmdFeatureManager::instance()->currentContextMenuTargetWidget() );

        if ( contextViewer )
        {
            // Link only the active view to an existing view link collection.
            auto* activeView = RiaApplication::instance()->activeReservoirView();
            if ( !activeView ) return false;

            if ( activeView->assosiatedViewLinker() ) return false;

            m_viewsToLink.push_back( activeView );
            return true;
        }

        const auto selectedGridViews = caf::SelectionManager::instance()->objectsByTypeStrict<Rim3dView>();
        for ( auto gridView : selectedGridViews )
        {
            if ( !gridView ) continue;

            if ( !gridView->assosiatedViewLinker() )
            {
                m_viewsToLink.push_back( gridView );
            }
        }

        return !m_viewsToLink.empty();
    }

    void execute() { RicLinkVisibleViewsFeature::linkViews( m_viewsToLink ); }

    const std::vector<Rim3dView*>& viewsToLink() { return m_viewsToLink; }

private:
    std::vector<Rim3dView*> m_viewsToLink;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkViewFeature::isCommandEnabled() const
{
    RicLinkViewFeatureImpl cmdImpl;
    return cmdImpl.prepareToExecute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::onActionTriggered( bool isChecked )
{
    RicLinkViewFeatureImpl cmdImpl;
    if ( cmdImpl.prepareToExecute() )
    {
        cmdImpl.execute();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::setupActionLook( QAction* actionToSetup )
{
    RicLinkViewFeatureImpl cmdImpl;
    cmdImpl.prepareToExecute();

    if ( cmdImpl.viewsToLink().size() >= 2u )
    {
        actionToSetup->setText( "Link Selected Views" );
        actionToSetup->setIcon( QIcon( ":/LinkView.svg" ) );
    }
    else
    {
        actionToSetup->setText( "Link View" );
        if ( RimProject::current()->viewLinkerCollection()->viewLinker() )
        {
            actionToSetup->setIcon( QIcon( ":/ControlledView16x16.png" ) );
        }
        else
        {
            actionToSetup->setIcon( QIcon( ":/MasterView16x16.png" ) );
        }
    }
}
