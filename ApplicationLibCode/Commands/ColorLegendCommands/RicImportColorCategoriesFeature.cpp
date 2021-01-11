/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicImportColorCategoriesFeature.h"

#include "RiaApplication.h"

#include "RigFormationNames.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimProject.h"

#include "RifColorLegendData.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportColorCategoriesFeature, "RicImportColorCategoriesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportColorCategoriesFeature::isCommandEnabled()
{
    RimColorLegendCollection* legendCollection = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( legendCollection );
    }

    if ( legendCollection ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportColorCategoriesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString filterText = QString( "Formation Names description File (*.lyr);;All Files (*.*)" );

    QString fileName = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                            "Import Formation File",
                                                            defaultDir,
                                                            filterText );

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileName ).absolutePath() );

    QString                                                                 errormessage;
    std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>> result =
        RifColorLegendData::readFormationNamesFile( fileName, &errormessage );

    RimColorLegend* colorLegend = result.second.p();
    RimProject*     proj        = RimProject::current();

    RimColorLegendCollection* colorLegendCollection = proj->colorLegendCollection;

    colorLegendCollection->appendCustomColorLegend( colorLegend );

    colorLegendCollection->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded( colorLegend );
    Riu3DMainWindowTools::selectAsCurrentItem( colorLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportColorCategoriesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "Import Color Legend from LYR Formation File" );
}
