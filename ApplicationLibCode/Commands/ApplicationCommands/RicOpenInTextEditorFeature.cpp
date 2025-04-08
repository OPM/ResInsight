/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicOpenInTextEditorFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimPressureDepthData.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicOpenInTextEditorFeature, "RicOpenInTextEditorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicOpenInTextEditorFeature::openFileInTextEditor( const QString& filePath, QObject* parent )
{
    RiaApplication* app                      = RiaApplication::instance();
    QString         textEditorExecutablePath = app->scriptEditorPath();
    if ( !textEditorExecutablePath.isEmpty() )
    {
        QStringList arguments;
        arguments << filePath;

        auto* myProcess = new QProcess( parent );
        myProcess->start( textEditorExecutablePath, arguments );

        if ( !myProcess->waitForStarted( 1000 ) )
        {
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(),
                                           "Text editor",
                                           "Failed to start text editor\n" + textEditorExecutablePath );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicOpenInTextEditorFeature::onActionTriggered( bool isChecked )
{
    QString filePath;

    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( !selectedItems.empty() )
    {
        if ( auto templateFileItem = dynamic_cast<RimPlotTemplateFileItem*>( selectedItems.front() ) )
        {
            filePath = templateFileItem->absoluteFilePath();
        }

        if ( auto pressureDepthData = dynamic_cast<RimPressureDepthData*>( selectedItems.front() ) )
        {
            filePath = pressureDepthData->filePath();
        }
    }

    if ( !filePath.isEmpty() )
    {
        RicOpenInTextEditorFeature::openFileInTextEditor( filePath, this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicOpenInTextEditorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Open in Text Editor" );
    actionToSetup->setIcon( QIcon( ":/open-text-editor.svg" ) );
}
