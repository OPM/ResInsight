/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Statoil ASA
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
#include "RicCompareTo3dViewFeature.h"

#include "RiaApplication.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimGridView.h"

#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCompareTo3dViewFeature, "RicCompareTo3dViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCompareTo3dViewFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompareTo3dViewFeature::onActionTriggered( bool isChecked )
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    QVariant userData = this->userData();
    auto     view     = static_cast<Rim3dView*>( userData.value<void*>() );

    if ( view && activeView )
    {
        activeView->setComparisonView( view );
        activeView->scheduleCreateDisplayModelAndRedraw();
        activeView->overlayInfoConfig()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompareTo3dViewFeature::setupActionLook( QAction* actionToSetup )
{
    QVariant userData = actionToSetup->data();

    auto view = static_cast<Rim3dView*>( userData.value<void*>() );
    if ( view )
    {
        auto icon = view->uiIconProvider().icon();
        if ( icon ) actionToSetup->setIcon( *icon );
    }
    else
    {
        caf::IconProvider iconProvider( ":/ComparisonView16x16.png" );
        auto              icon = iconProvider.icon();
        if ( icon ) actionToSetup->setIcon( *icon );
    }
}
