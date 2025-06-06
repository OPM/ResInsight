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

#include "RicAddEclipseInputPropertyFeature.h"

#include "RiaApplication.h"
#include "RiaDefines.h"

#include "RicImportGeneralDataFeature.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimRoffCase.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QStringList>

CAF_CMD_SOURCE_INIT( RicAddEclipseInputPropertyFeature, "RicAddEclipseInputPropertyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAddEclipseInputPropertyFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimEclipseInputCase>() ||
           caf::SelectionManager::instance()->selectedItemOfType<RimEclipseResultCase>() ||
           caf::SelectionManager::instance()->selectedItemOfType<RimRoffCase>() ||
           caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCellColors>() ||
           caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddEclipseInputPropertyFeature::onActionTriggered( bool isChecked )
{
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase )
    {
        return;
    }

    RimEclipseInputPropertyCollection* inputPropertyCollection = eclipseCase->inputPropertyCollection();
    if ( !inputPropertyCollection ) return;

    QFileInfo fi( eclipseCase->gridFileName() );
    QString   casePath = fi.absolutePath();

    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectoryWithFallback( "INPUT_FILES", casePath );
    QString         filePattern =
        RicImportGeneralDataFeature::getFilePattern( { RiaDefines::ImportFileType::ECLIPSE_INPUT_FILE, RiaDefines::ImportFileType::ROFF_FILE },
                                                     true );
    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Select Eclipse Input Property Files",
                                                                  defaultDir,
                                                                  filePattern );

    if ( fileNames.isEmpty() ) return;

    // Remember the directory to next time
    defaultDir = QFileInfo( fileNames.last() ).absolutePath();
    app->setLastUsedDialogDirectory( "INPUT_FILES", defaultDir );
    eclipseCase->importAsciiInputProperties( fileNames );
    inputPropertyCollection->updateConnectedEditors();
    eclipseCase->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddEclipseInputPropertyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Input Property" );
    actionToSetup->setIcon( QIcon( ":/EclipseInput48x48.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicAddEclipseInputPropertyFeature::getEclipseCase() const
{
    RimEclipseCase* eclipseCase = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>();
    if ( eclipseCase )
    {
        return eclipseCase;
    }

    RimEclipseCellColors* cellColors = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCellColors>();
    if ( cellColors )
    {
        eclipseCase = cellColors->firstAncestorOrThisOfType<RimEclipseCase>();
        if ( eclipseCase )
        {
            return eclipseCase;
        }
    }

    RimEclipseView* eclipseView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    if ( eclipseView )
    {
        eclipseCase = eclipseView->firstAncestorOrThisOfType<RimEclipseCase>();
        if ( eclipseCase )
        {
            return eclipseCase;
        }
    }

    return nullptr;
}
