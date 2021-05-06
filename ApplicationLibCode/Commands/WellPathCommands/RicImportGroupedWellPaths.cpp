#include "RicImportGroupedWellPaths.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimFileWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QAction>
#include <QDir>

RICF_SOURCE_INIT( RicImportGroupedWellPaths, "RicImportGroupedWellPathsFeature", "importGroupedWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportGroupedWellPaths::RicImportGroupedWellPaths()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGroupedWellPaths::onActionTriggered( bool isChecked )
{
    RiaApplication* app                = RiaApplication::instance();
    QString         lastUsedGridFolder = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QString         defaultDir         = app->lastUsedDialogDirectoryWithFallback( "WELLPATH_DIR", lastUsedGridFolder );

    QString nameList = QString( "Well Paths (%1);;All Files (*.*)" ).arg( wellPathNameFilters().join( " " ) );

    QStringList wellPathFilePaths = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                          "Import Well Paths",
                                                                          defaultDir,
                                                                          nameList );

    if ( !wellPathFilePaths.empty() )
    {
        m_importGrouped                 = true;
        m_wellPathFiles.v()             = std::vector<QString>( wellPathFilePaths.begin(), wellPathFilePaths.end() );
        caf::PdmScriptResponse response = execute();
        QStringList            messages = response.messages();

        if ( !messages.empty() )
        {
            QString displayMessage = QString( "Problem loading well path files:\n%2" ).arg( messages.join( "\n" ) );
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Well Path Loading", displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGroupedWellPaths::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import &Grouped Well Paths from File" );
    actionToSetup->setIcon( QIcon( ":/WellPathGroup.svg" ) );
}
