////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicEditPlotTemplateFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimProject.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicEditPlotTemplateFeature, "RicEditPlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEditPlotTemplateFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );
    if ( uiItems.size() != 1 ) return false;

    RimPlotTemplateFileItem* file = dynamic_cast<RimPlotTemplateFileItem*>( uiItems[0] );
    return ( file != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditPlotTemplateFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    if ( uiItems.size() != 1 ) return;

    RimPlotTemplateFileItem* file = dynamic_cast<RimPlotTemplateFileItem*>( uiItems[0] );
    if ( file == nullptr ) return;

    RiaApplication* app          = RiaApplication::instance();
    QString         scriptEditor = app->scriptEditorPath();
    if ( !scriptEditor.isEmpty() )
    {
        QStringList arguments;
        arguments << file->absoluteFilePath();

        QProcess* myProcess = new QProcess( this );
        myProcess->start( scriptEditor, arguments );

        if ( !myProcess->waitForStarted( 1000 ) )
        {
            RiaLogging::errorInMessageBox( RiuPlotMainWindow::instance(),
                                           "Script editor",
                                           "Failed to start script editor executable\n" + scriptEditor );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditPlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Edit" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}
