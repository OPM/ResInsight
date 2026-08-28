/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicCreateSumoEnsembleFeature.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSumoEnsembleFeature, "RicCreateSumoEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSumoEnsembleFeature::isCommandEnabled() const
{
    // The context menu is raised from the window holding the tree, so the active window is the one that was
    // right-clicked. Same discrimination as RicViewZoomAllFeature.
    return dynamic_cast<RiuPlotMainWindow*>( RiaGuiApplication::activeWindow() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoEnsembleFeature::onActionTriggered( bool isChecked )
{
    auto dataSources = caf::selectedObjectsByType<RimSumoDataSource*>();

    RimCloudDataSourceCollection::createEnsemblesFromSelectedDataSources( dataSources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Ensemble Plot" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
}
