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

#include "RicNewGenericDataViewFeature.h"

#include "RimGeneric3dView.h"
#include "RimGenericViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGenericDataViewFeature, "RicNewGenericDataViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGenericDataViewFeature::isCommandEnabled() const
{
    // A data view is useful even in an empty project; content can be added afterwards.
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGenericDataViewFeature::onActionTriggered( bool isChecked )
{
    createDataView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGenericDataViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );
    actionToSetup->setText( "New Generic View" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeneric3dView* RicNewGenericDataViewFeature::createInitialViewIfNeeded()
{
    auto proj = RimProject::current();
    if ( !proj ) return nullptr;

    // Only auto-create when the project has no 3D view at all, mirroring RicNewSeismicViewFeature
    if ( !proj->allViews().empty() ) return nullptr;

    return createDataView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeneric3dView* RicNewGenericDataViewFeature::createDataView()
{
    auto proj = RimProject::current();
    if ( !proj ) return nullptr;

    auto oilField = proj->activeOilField();
    if ( !oilField || !oilField->genericViewCollection() ) return nullptr;

    auto view = oilField->genericViewCollection()->addGenericView();
    if ( !view ) return nullptr;

    oilField->genericViewCollection()->updateAllRequiredEditors();
    view->scheduleCreateDisplayModelAndRedraw();

    Riu3DMainWindowTools::selectAsCurrentItem( view );

    return view;
}
