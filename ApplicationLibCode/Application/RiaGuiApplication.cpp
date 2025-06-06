/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaGuiApplication.h"

#include "Cloud/RiaConnectorTools.h"
#include "RiaArgumentParser.h"
#include "RiaBaseDefs.h"
#include "RiaDefines.h"
#include "RiaFileLogger.h"
#include "RiaFilePathTools.h"
#include "RiaFontCache.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"
#include "RiaPreferencesGrid.h"
#include "RiaPreferencesSystem.h"
#include "RiaProjectModifier.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSocketServer.h"
#include "RiaVersionInfo.h"
#include "RiaViewRedrawScheduler.h"
#include "Summary/RiaSummaryTools.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "HoloLensCommands/RicHoloLensSessionManager.h"
#include "RicImportGeneralDataFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "Formations/RimFormationNamesCollection.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationTextAppearance.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"
#include "RimTextAnnotation.h"
#include "RimTextAnnotationInView.h"
#include "RimTools.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "Riu3dSelectionManager.h"
#include "RiuDockWidgetTools.h"
#include "RiuFileDialogTools.h"
#include "RiuGuiTheme.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowTools.h"
#include "RiuMdiMaximizeWindowGuard.h"
#include "RiuMessagePanel.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuProcessMonitor.h"
#include "RiuRecentFileActionProvider.h"
#include "RiuViewer.h"

#include "DockManager.h"

#include "cafAppEnum.h"
#include "cafCmdExecCommandManager.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafPdmUiTreeView.h"
#include "cafProgressInfo.h"
#include "cafQTreeViewStateSerializer.h"
#include "cafSelectionManager.h"
#include "cafUiProcess.h"
#include "cafUtils.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QDebug>
#include <QDir>
#include <QErrorMessage>
#include <QGridLayout>
#include <QKeyEvent>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeView>

#include <iostream>

#ifndef WIN32
#include <unistd.h> // for usleep
#endif // WIN32

//==================================================================================================
///
/// \class RiaGuiApplication
///
/// Application class
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::isRunning()
{
    return dynamic_cast<RiaGuiApplication*>( RiaApplication::instance() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication* RiaGuiApplication::instance()
{
    RiaGuiApplication* currentGuiApp = dynamic_cast<RiaGuiApplication*>( RiaApplication::instance() );
    CAF_ASSERT( currentGuiApp && "Should never be called from a method that isn't within the GUI context" );
    return currentGuiApp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication::RiaGuiApplication( int& argc, char** argv )
    : QApplication( argc, argv )
    , RiaApplication()
    , m_mainWindow( nullptr )
    , m_mainPlotWindow( nullptr )
{
    setWindowIcon( QIcon( ":/AppLogo48x48.png" ) );

    m_recentFileActionProvider = std::make_unique<RiuRecentFileActionProvider>();

    connect( this, SIGNAL( lastWindowClosed() ), SLOT( onLastWindowClosed() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication::~RiaGuiApplication()
{
    delete m_mainWindow.data();
    m_mainWindow.clear();

    m_mainPlotWindow.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::saveProject()
{
    CVF_ASSERT( m_project );

    QString fileName;
    if ( !isProjectSavedToDisc() )
    {
        fileName = promptForProjectSaveAsFileName();
    }
    else
    {
        fileName = m_project->fileName();
    }
    return saveProjectAs( fileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaGuiApplication::promptForProjectSaveAsFileName() const
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    QString startPath;
    if ( !m_project->fileName().isEmpty() )
    {
        startPath = m_project->fileName();
    }
    else
    {
        startPath = app->lastUsedDialogDirectory( "BINARY_GRID" );
        startPath += "/ResInsightProject.rsp";
    }

    QString fileName =
        RiuFileDialogTools::getSaveFileName( nullptr, tr( "Save File" ), startPath, tr( "Project Files (*.rsp);;All files(*.*)" ) );
    return fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::askUserToSaveModifiedProject()
{
    if ( RiaPreferencesSystem::current()->showProjectChangedDialog() && caf::PdmUiModelChangeDetector::instance()->isModelChanged() )
    {
        QMessageBox msgBox;
        msgBox.setIcon( QMessageBox::Question );

        QString questionText;
        questionText = QString( "The current project is modified.\n\nDo you want to save the changes?" );

        msgBox.setText( questionText );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

        int ret = msgBox.exec();
        if ( ret == QMessageBox::Cancel )
        {
            return false;
        }
        else if ( ret == QMessageBox::Yes )
        {
            if ( !saveProject() )
            {
                return false;
            }
        }
        else
        {
            caf::PdmUiModelChangeDetector::instance()->reset();
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::saveProjectAs( const QString& fileName )
{
    QString errMsg;
    if ( !RiaApplication::saveProjectAs( fileName, &errMsg ) )
    {
        QMessageBox::warning( nullptr, "Error when saving project file", errMsg );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::storeTreeViewState()
{
    if ( m_mainWindow )
    {
        QStringList treeStates;
        QStringList treeIndexes;

        for ( auto& tv : m_mainWindow->projectTreeViews() )
        {
            QString treeViewState;
            tv->storeTreeViewStateToString( treeViewState );
            treeStates.append( treeViewState );

            QModelIndex mi = tv->treeView()->currentIndex();
            QString     encodedModelIndexString;
            caf::QTreeViewStateSerializer::encodeStringFromModelIndex( mi, encodedModelIndexString );
            treeIndexes.append( encodedModelIndexString );
        }

        project()->mainWindowTreeViewStates         = treeStates.join( RiaDefines::stringListSeparator() );
        project()->mainWindowCurrentModelIndexPaths = treeIndexes.join( RiaDefines::stringListSeparator() );
    }

    if ( m_mainPlotWindow )
    {
        QStringList treeStates;
        QStringList treeIndexes;

        for ( auto& tv : mainPlotWindow()->projectTreeViews() )
        {
            QString treeViewState;
            tv->storeTreeViewStateToString( treeViewState );
            treeStates.append( treeViewState );

            QModelIndex mi = tv->treeView()->currentIndex();
            QString     encodedModelIndexString;
            caf::QTreeViewStateSerializer::encodeStringFromModelIndex( mi, encodedModelIndexString );
            treeIndexes.append( encodedModelIndexString );
        }

        project()->plotWindowTreeViewStates         = treeStates.join( RiaDefines::stringListSeparator() );
        project()->plotWindowCurrentModelIndexPaths = treeIndexes.join( RiaDefines::stringListSeparator() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::setWindowCaptionFromAppState()
{
    // The stuff being done here should really be handled by Qt automatically as a result of
    // setting applicationName and windowFilePath
    // Was unable to make this work in Qt4.4.0!

    QString capt = RI_APPLICATION_NAME;
#ifdef _DEBUG
    capt += " ##DEBUG##";
#endif

    {
        QString projFileName;

        if ( !m_project->fileName().isEmpty() )
        {
            QFileInfo fi( m_project->fileName() );
            projFileName = fi.fileName();
        }

        if ( projFileName.isEmpty() ) projFileName = "Untitled project";

        capt = projFileName + QString( "[*]" ) + QString( " - " ) + capt;
    }

    if ( m_mainWindow ) m_mainWindow->setWindowTitle( capt );
    if ( m_mainPlotWindow ) m_mainPlotWindow->setWindowTitle( capt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaGuiApplication::activePlotWindow() const
{
    RimViewWindow* viewWindow = nullptr;

    if ( m_mainPlotWindow )
    {
        QList<QMdiSubWindow*> subwindows = m_mainPlotWindow->subWindowList( QMdiArea::StackingOrder );
        if ( !subwindows.empty() )
        {
            viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( subwindows.back()->widget() );
        }
    }

    return viewWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::useShaders() const
{
    if ( !RiaPreferencesSystem::current()->useShaders() ) return false;

    bool isShadersSupported = true;
    if ( platformName() != "offscreen" ) // Avoid opengl access if we are in qt offscreen mode
    {
        isShadersSupported = caf::Viewer::isShadersSupported();
    }

    return isShadersSupported;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::RINavigationPolicy RiaGuiApplication::navigationPolicy() const
{
    return m_preferences->navigationPolicy();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::initialize()
{
    RiaApplication::initialize();

    applyGuiPreferences( nullptr );

    // Create main windows
    // The plot window is created to be able to set expanded state on created objects, but hidden by default
    getOrCreateAndShowMainWindow();
    getOrCreateMainPlotWindow();

    RiuGuiTheme::updateGuiTheme( m_preferences->guiTheme() );

    {
        auto logger = std::make_unique<RiuMessagePanelLogger>();
        logger->addMessagePanel( m_mainWindow->messagePanel() );
        logger->addMessagePanel( m_mainPlotWindow->messagePanel() );
        logger->setLevel( int( RiaLogging::logLevelBasedOnPreferences() ) );

        RiaLogging::appendLoggerInstance( std::move( logger ) );
    }

    if ( RiaPreferencesSystem::current()->logToFile() )
    {
        auto logFolder  = QDir::homePath() + "/.resinsight/logs";
        auto fileLogger = std::make_unique<RiaFileLogger>( logFolder.toStdString() );
        fileLogger->setLevel( int( RiaLogging::logLevelBasedOnPreferences() ) );

        RiaLogging::appendLoggerInstance( std::move( fileLogger ) );
    }
    m_socketServer = new RiaSocketServer( this );

    RiaConnectorTools::readCloudConfigFiles( m_preferences.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::ApplicationStatus RiaGuiApplication::handleArguments( gsl::not_null<cvf::ProgramOptions*> progOpt )
{
    // Handling of the actual command line options
    // --------------------------------------------------------
    if ( cvf::Option o = progOpt->option( "ignoreArgs" ) )
    {
        return ApplicationStatus::KEEP_GOING;
    }

    if ( progOpt->option( "help" ) || progOpt->option( "?" ) )
    {
        showFormattedTextInMessageBoxOrConsole( "The current command line options in ResInsight are:\n" + commandLineParameterHelp() );
        return RiaApplication::ApplicationStatus::EXIT_COMPLETED;
    }

    if ( progOpt->option( "version" ) )
    {
        QString text = QString( STRPRODUCTVER ) + "\n";
        text += "SHA " + QString( RESINSIGHT_GIT_HASH ) + "\n";

        showFormattedTextInMessageBoxOrConsole( text );

        return RiaApplication::ApplicationStatus::EXIT_COMPLETED;
    }

    // Code generation
    // -----------------
    if ( cvf::Option o = progOpt->option( "generate" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        QString outputFile = cvfqt::Utils::toQString( o.value( 0 ) );

        QString errMsg;
        if ( !RiaApplication::generateCode( outputFile, &errMsg ) )
        {
            RiaLogging::error( QString( "Error: %1" ).arg( errMsg ) );
            return RiaApplication::ApplicationStatus::KEEP_GOING;
        }

        return RiaApplication::ApplicationStatus::EXIT_COMPLETED;
    }

    if ( cvf::Option o = progOpt->option( "preferences" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        m_preferencesFileName = cvfqt::Utils::toQString( o.value( 0 ) );
        RiaApplication::initialize();
        onProjectClosed();
    }

    if ( cvf::Option o = progOpt->option( "regressiontest" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        QString regressionTestPath = cvfqt::Utils::toQString( o.value( 0 ) );

        // Use a logger writing to stdout instead of message panel
        // This is useful when executing regression tests on a build server, and this is the reason for creating the
        // logger when parsing the command line options
        auto stdLogger = std::make_unique<RiaStdOutLogger>();
        stdLogger->setLevel( int( RILogLevel::RI_LL_DEBUG ) );

        RiaLogging::appendLoggerInstance( std::move( stdLogger ) );

        RiaRegressionTestRunner::instance()->executeRegressionTests( regressionTestPath, QStringList() );
        return ApplicationStatus::EXIT_COMPLETED;
    }

    if ( cvf::Option o = progOpt->option( "updateregressiontestbase" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        QString regressionTestPath = cvfqt::Utils::toQString( o.value( 0 ) );
        RiaRegressionTestRunner::updateRegressionTest( regressionTestPath );
        return ApplicationStatus::EXIT_COMPLETED;
    }

    if ( cvf::Option o = progOpt->option( "startdir" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        setStartDir( cvfqt::Utils::toQString( o.value( 0 ) ) );
    }

    if ( cvf::Option o = progOpt->option( "egridReader" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        std::string readerName = o.value( 0 ).toLower().toStdString();
        m_preferences->gridPreferences()->setGridModelReaderOverride( readerName );
    }

    if ( cvf::Option o = progOpt->option( "size" ) )
    {
        int width  = o.safeValue( 0 ).toInt( -1 );
        int height = o.safeValue( 1 ).toInt( -1 );

        if ( width > 0 && height > 0 )
        {
            auto mainWindow = RiuMainWindow::instance();
            if ( mainWindow )
            {
                mainWindow->resize( width, height );
            }

            auto plotWindow = mainPlotWindow();
            if ( plotWindow )
            {
                plotWindow->resize( width, height );
            }
        }
    }

    int snapshotWidth  = -1;
    int snapshotHeight = -1;
    if ( cvf::Option o = progOpt->option( "snapshotsize" ) )
    {
        int width  = o.safeValue( 0 ).toInt( -1 );
        int height = o.safeValue( 1 ).toInt( -1 );

        if ( width > 0 && height > 0 )
        {
            snapshotWidth  = width;
            snapshotHeight = height;
        }
    }

    QString snapshotFolderFromCommandLine;
    if ( cvf::Option o = progOpt->option( "snapshotfolder" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        snapshotFolderFromCommandLine = cvfqt::Utils::toQString( o.value( 0 ) );
    }

    if ( cvf::Option o = progOpt->option( "summaryplot" ) )
    {
        RicSummaryPlotFeatureImpl::createSummaryPlotsFromArgumentLine( cvfqt::Utils::toQStringList( o.values() ) );
    }

    if ( cvf::Option o = progOpt->option( "openplotwindow" ) )
    {
        if ( m_mainWindow ) m_mainWindow->hide();
        getOrCreateAndShowMainPlotWindow();
    }

    QString projectFileName;

    if ( progOpt->hasOption( "last" ) )
    {
        projectFileName = preferences()->lastUsedProjectFileName;
    }

    if ( cvf::Option o = progOpt->option( "project" ) )
    {
        CVF_ASSERT( o.valueCount() == 1 );
        projectFileName = cvfqt::Utils::toQString( o.value( 0 ) );
    }

    if ( !projectFileName.isEmpty() )
    {
        if ( cvf::Option o = progOpt->option( "multiCaseSnapshots" ) )
        {
            QString              gridListFile = cvfqt::Utils::toQString( o.safeValue( 0 ) );
            std::vector<QString> gridFiles    = readFileListFromTextFile( gridListFile );
            runMultiCaseSnapshots( projectFileName, gridFiles, "multiCaseSnapshots" );

            return ApplicationStatus::EXIT_COMPLETED;
        }
    }

    if ( !projectFileName.isEmpty() )
    {
        cvf::ref<RiaProjectModifier>      projectModifier;
        RiaApplication::ProjectLoadAction projectLoadAction = RiaApplication::ProjectLoadAction::PLA_NONE;

        if ( cvf::Option o = progOpt->option( "replaceCase" ) )
        {
            if ( projectModifier.isNull() ) projectModifier = cvf::make_ref<RiaProjectModifier>();

            if ( o.valueCount() == 1 )
            {
                // One argument is available, use replace case for first occurrence in the project

                QString gridFileName = cvfqt::Utils::toQString( o.safeValue( 0 ) );
                projectModifier->setReplaceCaseFirstOccurrence( gridFileName );
            }
            else
            {
                size_t optionIdx = 0;
                while ( optionIdx < o.valueCount() )
                {
                    const int caseId       = o.safeValue( optionIdx++ ).toInt( -1 );
                    QString   gridFileName = cvfqt::Utils::toQString( o.safeValue( optionIdx++ ) );

                    if ( caseId != -1 && !gridFileName.isEmpty() )
                    {
                        projectModifier->setReplaceCase( caseId, gridFileName );
                    }
                }
            }
        }

        if ( cvf::Option o = progOpt->option( "replaceSourceCases" ) )
        {
            if ( projectModifier.isNull() ) projectModifier = cvf::make_ref<RiaProjectModifier>();

            if ( o.valueCount() == 1 )
            {
                // One argument is available, use replace case for first occurrence in the project

                std::vector<QString> gridFileNames = readFileListFromTextFile( cvfqt::Utils::toQString( o.safeValue( 0 ) ) );
                projectModifier->setReplaceSourceCasesFirstOccurrence( gridFileNames );
            }
            else
            {
                size_t optionIdx = 0;
                while ( optionIdx < o.valueCount() )
                {
                    const int            groupId       = o.safeValue( optionIdx++ ).toInt( -1 );
                    std::vector<QString> gridFileNames = readFileListFromTextFile( cvfqt::Utils::toQString( o.safeValue( optionIdx++ ) ) );

                    if ( groupId != -1 && !gridFileNames.empty() )
                    {
                        projectModifier->setReplaceSourceCasesById( groupId, gridFileNames );
                    }
                }
            }

            projectLoadAction = RiaApplication::ProjectLoadAction::PLA_CALCULATE_STATISTICS;
        }

        if ( cvf::Option o = progOpt->option( "replacePropertiesFolder" ) )
        {
            if ( projectModifier.isNull() ) projectModifier = cvf::make_ref<RiaProjectModifier>();

            if ( o.valueCount() == 1 )
            {
                QString propertiesFolder = cvfqt::Utils::toQString( o.safeValue( 0 ) );
                projectModifier->setReplacePropertiesFolderFirstOccurrence( propertiesFolder );
            }
            else
            {
                size_t optionIdx = 0;
                while ( optionIdx < o.valueCount() )
                {
                    const int caseId           = o.safeValue( optionIdx++ ).toInt( -1 );
                    QString   propertiesFolder = cvfqt::Utils::toQString( o.safeValue( optionIdx++ ) );

                    if ( caseId != -1 && !propertiesFolder.isEmpty() )
                    {
                        projectModifier->setReplacePropertiesFolder( caseId, propertiesFolder );
                    }
                }
            }
        }

        loadProject( projectFileName, projectLoadAction, projectModifier.p() );
    }

    if ( cvf::Option o = progOpt->option( "case" ) )
    {
        QStringList fileNames = RicImportGeneralDataFeature::fileNamesFromCaseNames( cvfqt::Utils::toQStringList( o.values() ) );

        bool createView = true;
        bool createPlot = true;

        RicImportGeneralDataFeature::OpenCaseResults results =
            RicImportGeneralDataFeature::openEclipseFilesFromFileNames( fileNames, createPlot, createView );
        if ( results && !results.eclipseSummaryFiles.empty() )
        {
            getOrCreateAndShowMainPlotWindow();
        }
    }

    if ( cvf::Option o = progOpt->option( "savesnapshots" ) )
    {
        bool snapshotViews = false;
        bool snapshotPlots = false;

        QStringList snapshotItemTexts = cvfqt::Utils::toQStringList( o.values() );
        if ( snapshotItemTexts.empty() )
        {
            // No options will keep backwards compatibility before we introduced snapshot of plots
            snapshotViews = true;
        }

        for ( const QString& s : snapshotItemTexts )
        {
            if ( s.toLower() == "all" )
            {
                snapshotViews = true;
                snapshotPlots = true;
            }
            else if ( s.toLower() == "views" )
            {
                snapshotViews = true;
            }
            else if ( s.toLower() == "plots" )
            {
                snapshotPlots = true;
            }
        }

        QString snapshotFolder;

        if ( snapshotFolderFromCommandLine.isEmpty() )
        {
            snapshotFolder = QDir::currentPath() + "/snapshots";
        }
        else
        {
            snapshotFolder = snapshotFolderFromCommandLine;
        }

        if ( snapshotPlots )
        {
            auto mainPlotWnd = mainPlotWindow();
            if ( mainPlotWnd )
            {
                mainPlotWnd->show();
                mainPlotWnd->raise();

                if ( snapshotHeight > -1 && snapshotWidth > -1 )
                {
                    RiuMainWindowTools::setWindowSizeOnWidgetsInMdiWindows( mainPlotWnd, snapshotWidth, snapshotHeight );
                }

                processEvents();

                RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotsIntoFolder( snapshotFolder );
            }
        }

        if ( snapshotViews )
        {
            auto mainWnd = RiuMainWindow::instance();
            mainWnd->show();
            mainWnd->raise();

            if ( snapshotHeight > -1 && snapshotWidth > -1 )
            {
                RiuMainWindowTools::setFixedWindowSizeFor3dViews( mainWnd, snapshotWidth, snapshotHeight );
            }

            processEvents();

            RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( snapshotFolder );
        }

        auto mainPlotWnd = mainPlotWindow();
        if ( mainPlotWnd )
        {
            mainPlotWnd->loadWinGeoAndDockToolBarLayout();
        }

        if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->loadWinGeoAndDockToolBarLayout();

        return ApplicationStatus::EXIT_COMPLETED;
    }

    if ( cvf::Option o = progOpt->option( "commandFile" ) )
    {
        QString commandFile = cvfqt::Utils::toQString( o.safeValue( 0 ) );

        if ( !progOpt->hasOption( "startdir" ) )
        {
            QFileInfo commandFileInfo( commandFile );
            QString   commandDir = commandFileInfo.absolutePath();
            setStartDir( commandDir );
        }

        cvf::Option projectOption = progOpt->option( "commandFileProject" );
        cvf::Option caseOption    = progOpt->option( "commandFileReplaceCases" );
        if ( projectOption && caseOption )
        {
            projectFileName = cvfqt::Utils::toQString( projectOption.value( 0 ) );

            std::vector<int>     caseIds;
            std::vector<QString> caseListFiles;

            if ( caseOption.valueCount() == 1 )
            {
                caseListFiles.push_back( cvfqt::Utils::toQString( caseOption.safeValue( 0 ) ) );
            }
            else
            {
                size_t optionIdx = 0;
                while ( optionIdx < caseOption.valueCount() )
                {
                    const int caseId       = caseOption.safeValue( optionIdx++ ).toInt( -1 );
                    QString   caseListFile = cvfqt::Utils::toQString( caseOption.safeValue( optionIdx++ ) );

                    if ( caseId != -1 && !caseListFile.isEmpty() )
                    {
                        caseIds.push_back( caseId );
                        caseListFiles.push_back( caseListFile );
                    }
                }
            }

            if ( caseIds.empty() && !caseListFiles.empty() )
            {
                QString              caseListFile = caseListFiles[0];
                std::vector<QString> caseFiles    = readFileListFromTextFile( caseListFile );
                for ( const QString& caseFile : caseFiles )
                {
                    RiaProjectModifier projectModifier;
                    projectModifier.setReplaceCaseFirstOccurrence( caseFile );
                    loadProject( projectFileName, RiaApplication::ProjectLoadAction::PLA_NONE, &projectModifier );
                    executeCommandFile( commandFile );
                }
            }
            else
            {
                CVF_ASSERT( caseIds.size() == caseListFiles.size() );

                std::vector<std::vector<QString>> allCaseFiles;
                size_t                            maxFiles = 0;

                for ( size_t i = 0; i < caseIds.size(); ++i )
                {
                    std::vector<QString> caseFiles = readFileListFromTextFile( caseListFiles[i] );
                    allCaseFiles.push_back( caseFiles );
                    maxFiles = std::max( caseFiles.size(), maxFiles );
                }

                for ( size_t i = 0; i < caseIds.size(); ++i )
                {
                    RiaProjectModifier projectModifier;
                    for ( size_t j = 0; j < maxFiles; ++j )
                    {
                        if ( allCaseFiles[i].size() > j )
                        {
                            projectModifier.setReplaceCase( caseIds[i], allCaseFiles[i][j] );
                        }
                    }

                    loadProject( projectFileName, RiaApplication::ProjectLoadAction::PLA_NONE, &projectModifier );
                    executeCommandFile( commandFile );
                }
            }
        }
        else
        {
            executeCommandFile( commandFile );
        }
        return ApplicationStatus::EXIT_COMPLETED;
    }

    return ApplicationStatus::KEEP_GOING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindow* RiaGuiApplication::getOrCreateAndShowMainWindow()
{
    if ( !m_mainWindow )
    {
        createMainWindow();
    }
    else
    {
        m_mainWindow->show();
    }

    return m_mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindow* RiaGuiApplication::mainWindow()
{
    return m_mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaGuiApplication::getOrCreateMainPlotWindow()
{
    if ( !m_mainPlotWindow )
    {
        createMainPlotWindow();
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
        loadAndUpdatePlotData();
    }
    return m_mainPlotWindow.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::createMainWindow()
{
    CVF_ASSERT( m_mainWindow == nullptr );

    // Always enable undo/redo framework, as multi-select operations perform significantly better with it enabled
    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem( true );

    m_mainWindow     = new RiuMainWindow;
    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    m_mainWindow->setWindowTitle( "ResInsight " + platform );
    m_mainWindow->setDefaultWindowSize();
    m_mainWindow->setDefaultToolbarVisibility();
    m_mainWindow->loadWinGeoAndDockToolBarLayout();
    m_mainWindow->showWindow();

    // if there is an existing logger, reconnect to it

    for ( auto logger : RiaLogging::loggerInstances() )
    {
        auto messagePanelLogger = dynamic_cast<RiuMessagePanelLogger*>( logger );
        if ( messagePanelLogger )
        {
            messagePanelLogger->addMessagePanel( m_mainWindow->messagePanel() );
        }
    }

    // Initialize OpenGL here to avoid flickering when creating the first 3D view
    QOpenGLWidget openGLWidget( m_mainWindow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::createMainPlotWindow()
{
    CVF_ASSERT( m_mainPlotWindow == nullptr );

    // Always enable undo/redo framework, as multi-select operations perform significantly better with it enabled
    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem( true );

    m_mainPlotWindow = std::make_unique<RiuPlotMainWindow>();
    m_mainPlotWindow->setWindowTitle( "Plots - ResInsight" );
    m_mainPlotWindow->setDefaultWindowSize();
    m_mainPlotWindow->loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaGuiApplication::getOrCreateAndShowMainPlotWindow()
{
    if ( !m_mainPlotWindow )
    {
        createMainPlotWindow();
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
        loadAndUpdatePlotData();
    }

    if ( m_mainPlotWindow->isMinimized() )
    {
        m_mainPlotWindow->showNormal();
        m_mainPlotWindow->update();
    }
    else
    {
        m_mainPlotWindow->show();
    }

    m_mainPlotWindow->raise();
    m_mainPlotWindow->activateWindow();

    return m_mainPlotWindow.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaGuiApplication::mainPlotWindow()
{
    return m_mainPlotWindow.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RiaGuiApplication::mainWindowByID( int mainWindowID )
{
    if ( mainWindowID == 0 )
        return m_mainWindow;
    else if ( mainWindowID == 1 )
        return m_mainPlotWindow.get();
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaGuiApplication::activeViewWindow()
{
    RimViewWindow* viewWindow = nullptr;

    QWidget* mainWindowWidget = RiaGuiApplication::activeWindow();

    if ( dynamic_cast<RiuMainWindow*>( mainWindowWidget ) )
    {
        viewWindow = RiaGuiApplication::instance()->activeReservoirView();
    }
    else if ( dynamic_cast<RiuPlotMainWindow*>( mainWindowWidget ) )
    {
        RiuPlotMainWindow* mainPlotWindow = dynamic_cast<RiuPlotMainWindow*>( mainWindowWidget );

        QList<QMdiSubWindow*> subwindows = mainPlotWindow->subWindowList( QMdiArea::StackingOrder );
        if ( !subwindows.empty() )
        {
            viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( subwindows.back()->widget() );
        }
    }

    return viewWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RiaGuiApplication::activeMainWindow()
{
    QWidget*           mainWindowWidget = RiaGuiApplication::activeWindow();
    RiuMainWindowBase* mainWindow       = dynamic_cast<RiuMainWindowBase*>( mainWindowWidget );

    return mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::isMain3dWindowVisible() const
{
    return m_mainWindow && m_mainWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::isMainPlotWindowVisible() const
{
    return m_mainPlotWindow && m_mainPlotWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::addToRecentFiles( const QString& fileName )
{
    CVF_ASSERT( m_recentFileActionProvider && "The provider needs to be created before any attempts to use the recent file actions" );
    m_recentFileActionProvider->addFileName( fileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QAction*> RiaGuiApplication::recentFileActions() const
{
    CVF_ASSERT( m_recentFileActionProvider && "The provider needs to be created before any attempts to use the recent file actions" );
    return m_recentFileActionProvider->actions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::clearAllSelections()
{
    Riu3dSelectionManager::instance()->deleteAllItems( Riu3dSelectionManager::RUI_APPLICATION_GLOBAL );
    Riu3dSelectionManager::instance()->deleteAllItems( Riu3dSelectionManager::RUI_TEMPORARY );
    caf::SelectionManager::instance()->clearAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::showFormattedTextInMessageBoxOrConsole( const QString& text )
{
    // Create a message dialog with cut/paste friendly text
    QDialog dlg;
    dlg.setModal( true );

    QGridLayout* layout = new QGridLayout;
    dlg.setLayout( layout );

    QTextEdit* textEdit = new QTextEdit( &dlg );
    layout->addWidget( textEdit, 0, 1, 1, 2 );
    layout->setColumnStretch( 1, 3 );

    QPushButton* okButton = new QPushButton;
    okButton->setText( "Ok" );
    layout->addWidget( okButton, 2, 2 );
    QObject::connect( okButton, SIGNAL( clicked() ), &dlg, SLOT( accept() ) );

    // Convert text to text edit friendly format
    QString formattedText = text;
    formattedText.replace( "&", "&amp;" );
    formattedText.replace( "<", "&lt;" );
    formattedText.replace( ">", "&gt;" );
    formattedText = QString( "<pre>%1</pre>" ).arg( formattedText );

    textEdit->setText( formattedText );
    textEdit->setReadOnly( true );

    // Resize dialog to fit text etc.
    textEdit->document()->adjustSize();
    QSizeF docSize = textEdit->document()->size();
    dlg.resize( 20 + docSize.width() + 2 * layout->contentsMargins().left(),
                20 + docSize.height() + 2 * layout->contentsMargins().left() + layout->spacing() + okButton->sizeHint().height() );

    dlg.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::invokeProcessEvents( QEventLoop::ProcessEventsFlags flags )
{
    processEvents( flags );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onFileSuccessfullyLoaded( const QString& fileName, RiaDefines::ImportFileType fileType )
{
    if ( RiaDefines::isEclipseFileType( fileType ) )
    {
        if ( fileType == RiaDefines::ImportFileType::ECLIPSE_SUMMARY_FILE )
        {
            auto plotWindow = getOrCreateAndShowMainPlotWindow();
            plotWindow->raise();
        }
        else
        {
            auto mainWindow = getOrCreateAndShowMainWindow();
            mainWindow->raise();
        }
    }

    if ( !RiaGuiApplication::hasValidProjectFileExtension( fileName ) )
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectBeingOpened()
{
    // When importing a project, do not maximize the first MDI window to be created
    m_maximizeWindowGuard = std::make_unique<RiuMdiMaximizeWindowGuard>();

    if ( m_mainWindow ) m_mainWindow->setBlockSubWindowActivatedSignal( true );
    if ( mainPlotWindow() ) mainPlotWindow()->setBlockSubWindowActivatedSignal( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectOpeningError( const QString& errMsg )
{
    RiaLogging::errorInMessageBox( nullptr, "Error when opening project file", errMsg );
    if ( m_mainWindow ) m_mainWindow->setPdmRoot( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectOpened()
{
    if ( m_project->show3DWindow() )
    {
        getOrCreateAndShowMainWindow();
    }
    else
    {
        if ( m_mainWindow ) m_mainWindow->hide();
    }

    if ( m_project->showPlotWindow() )
    {
        if ( !m_mainPlotWindow )
        {
            createMainPlotWindow();
            m_mainPlotWindow->show();
        }
        else
        {
            m_mainPlotWindow->show();
            m_mainPlotWindow->raise();
        }
    }
    else if ( mainPlotWindow() )
    {
        mainPlotWindow()->hide();
    }

    loadAndUpdatePlotData();

    if ( m_mainWindow )
    {
        m_mainWindow->initializeGuiNewProjectLoaded();
    }
    if ( m_mainPlotWindow )
    {
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
    }

    setWindowCaptionFromAppState();

    m_maximizeWindowGuard.reset();

    processEvents();

    if ( m_mainWindow ) m_mainWindow->setBlockSubWindowActivatedSignal( false );
    if ( mainPlotWindow() ) mainPlotWindow()->setBlockSubWindowActivatedSignal( false );

    // Make sure to process events before this function to avoid strange Qt crash
    RiuPlotMainWindowTools::refreshToolbars();

    if ( m_project->showPlotWindow() && m_project->showPlotWindowOnTop() )
    {
        m_mainPlotWindow->raise();
        m_mainPlotWindow->activateWindow();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectBeingClosed()
{
    RicHoloLensSessionManager::instance()->terminateSession();
    RicHoloLensSessionManager::refreshToolbarState();

    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();
    RiaPlotWindowRedrawScheduler::instance()->clearAllScheduledUpdates();

    RiaGuiApplication::clearAllSelections();

    if ( m_mainWindow ) m_mainWindow->cleanupGuiBeforeProjectClose();
    if ( m_mainPlotWindow ) m_mainPlotWindow->cleanupGuiBeforeProjectClose();

    caf::EffectGenerator::clearEffectCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectClosed()
{
    if ( m_mainWindow ) m_mainWindow->initializeGuiNewProjectLoaded();
    if ( m_mainPlotWindow ) m_mainPlotWindow->initializeGuiNewProjectLoaded();

    setWindowCaptionFromAppState();

    processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectBeingSaved()
{
    setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( m_project->fileName() ).absolutePath() );
    storeTreeViewState();

    RiaSummaryTools::summaryCaseMainCollection()->onProjectBeingSaved();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectSaved()
{
    setWindowCaptionFromAppState();
    m_recentFileActionProvider->addFileName( m_project->fileName() );
    caf::PdmUiModelChangeDetector::instance()->reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::applyGuiPreferences( const RiaPreferences*                         oldPreferences,
                                             const std::vector<caf::FontHolderInterface*>& defaultFontObjects )
{
    if ( m_activeReservoirView && m_activeReservoirView->viewer() )
    {
        m_activeReservoirView->viewer()->updateNavigationPolicy();
        m_activeReservoirView->viewer()->enablePerfInfoHud( RiaPreferencesSystem::current()->show3dInformation() );
    }

    if ( useShaders() )
    {
        caf::EffectGenerator::setRenderingMode( caf::EffectGenerator::SHADER_BASED );
    }
    else
    {
        caf::EffectGenerator::setRenderingMode( caf::EffectGenerator::FIXED_FUNCTION );
    }

    if ( m_mainWindow )
    {
        for ( auto& tv : m_mainWindow->projectTreeViews() )
        {
            tv->enableAppendOfClassNameToUiItemText( RiaPreferencesSystem::current()->appendClassNameToUiText() );
        }
    }
    if ( mainPlotWindow() )
    {
        for ( auto& tv : mainPlotWindow()->projectTreeViews() )
        {
            tv->enableAppendOfClassNameToUiItemText( RiaPreferencesSystem::current()->appendClassNameToUiText() );
        }
    }

    for ( auto fontObject : defaultFontObjects )
    {
        fontObject->updateFonts();
    }

    if ( project() )
    {
        std::vector<RimViewWindow*> allViewWindows = project()->descendantsIncludingThisOfType<RimViewWindow>();

        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();

        bool existingViewsWithDifferentMeshLines = false;
        bool existingViewsWithCustomColors       = false;
        bool existingViewsWithCustomZScale       = false;
        if ( oldPreferences )
        {
            for ( auto viewWindow : allViewWindows )
            {
                auto rim3dView = dynamic_cast<Rim3dView*>( viewWindow );
                if ( rim3dView )
                {
                    if ( m_preferences->defaultMeshModeType() != oldPreferences->defaultMeshModeType() &&
                         rim3dView->meshMode() != oldPreferences->defaultMeshModeType() &&
                         rim3dView->meshMode() != m_preferences->defaultMeshModeType() )
                    {
                        existingViewsWithDifferentMeshLines = true;
                    }
                    if ( m_preferences->defaultViewerBackgroundColor() != oldPreferences->defaultViewerBackgroundColor() &&
                         rim3dView->backgroundColor() != oldPreferences->defaultViewerBackgroundColor() &&
                         rim3dView->backgroundColor() != m_preferences->defaultViewerBackgroundColor() )
                    {
                        existingViewsWithCustomColors = true;
                    }
                    if ( m_preferences->defaultScaleFactorZ() != oldPreferences->defaultScaleFactorZ() &&
                         rim3dView->scaleZ() != oldPreferences->defaultScaleFactorZ() &&
                         rim3dView->scaleZ() != m_preferences->defaultScaleFactorZ() )
                    {
                        existingViewsWithCustomZScale = true;
                    }

                    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( rim3dView );
                    if ( eclipseView )
                    {
                        if ( m_preferences->defaultWellLabelColor() != oldPreferences->defaultWellLabelColor() &&
                             eclipseView->wellCollection()->wellLabelColor() != oldPreferences->defaultWellLabelColor() &&
                             eclipseView->wellCollection()->wellLabelColor() != m_preferences->defaultWellLabelColor() )
                        {
                            existingViewsWithCustomColors = true;
                        }
                    }
                }
            }

            if ( oldPreferences->defaultWellLabelColor() != wellPathCollection->wellPathLabelColor() )
            {
                existingViewsWithCustomColors = true;
            }
        }

        bool applySettingsToAllViews = false;
        if ( existingViewsWithCustomColors || existingViewsWithCustomZScale || existingViewsWithDifferentMeshLines )
        {
            QStringList changedData;
            if ( existingViewsWithDifferentMeshLines ) changedData << "Mesh Visibility";
            if ( existingViewsWithCustomColors ) changedData << "Colors";
            if ( existingViewsWithCustomZScale ) changedData << "Z-Scale";

            QString listString = changedData.takeLast();
            if ( !changedData.empty() )
            {
                listString = changedData.join( ", " ) + " and " + listString;
            }

            QMessageBox::StandardButton reply;
            reply                   = QMessageBox::question( activeMainWindow(),
                                           QString( "Apply %1 to Existing Views or Plots?" ).arg( listString ),
                                           QString( "You have changed default %1 and have existing views or plots with "
                                                                      "different settings.\n" )
                                                   .arg( listString ) +
                                               QString( "Do you want to apply the new default settings to all existing "
                                                                          "views?" ),
                                           QMessageBox::Ok | QMessageBox::Cancel );
            applySettingsToAllViews = ( reply == QMessageBox::Ok );
        }

        for ( auto viewWindow : allViewWindows )
        {
            auto rim3dView = dynamic_cast<Rim3dView*>( viewWindow );
            if ( rim3dView )
            {
                if ( oldPreferences && ( applySettingsToAllViews || rim3dView->meshMode() == oldPreferences->defaultMeshModeType() ) )
                {
                    rim3dView->meshMode = m_preferences->defaultMeshModeType();
                }

                if ( oldPreferences &&
                     ( applySettingsToAllViews || rim3dView->backgroundColor() == oldPreferences->defaultViewerBackgroundColor() ) )
                {
                    rim3dView->setBackgroundColor( m_preferences->defaultViewerBackgroundColor() );
                    rim3dView->applyFontChanges();
                }

                if ( oldPreferences && ( applySettingsToAllViews || rim3dView->scaleZ() == oldPreferences->defaultScaleFactorZ() ) )
                {
                    rim3dView->setScaleZ( m_preferences->defaultScaleFactorZ() );
                    rim3dView->updateScaling();
                    if ( rim3dView == activeViewWindow() )
                    {
                        if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->updateScaleValue();
                    }
                }

                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( rim3dView );
                if ( eclipseView )
                {
                    if ( oldPreferences && ( applySettingsToAllViews ||
                                             eclipseView->wellCollection()->wellLabelColor() == oldPreferences->defaultWellLabelColor() ) )
                    {
                        eclipseView->wellCollection()->wellLabelColor = m_preferences->defaultWellLabelColor();
                    }
                    eclipseView->scheduleReservoirGridGeometryRegen();
                }
                rim3dView->scheduleCreateDisplayModelAndRedraw();
            }
        }

        if ( oldPreferences )
        {
            bool matchingColor = wellPathCollection->wellPathLabelColor() == oldPreferences->defaultWellLabelColor();
            if ( applySettingsToAllViews || matchingColor )
            {
                wellPathCollection->wellPathLabelColor = oldPreferences->defaultWellLabelColor();
            }

            if ( oldPreferences->defaultPlotFontSize() != m_preferences->defaultPlotFontSize() )
            {
                if ( m_mainWindow )
                {
                    m_mainWindow->applyFontSizesToDockedPlots();
                }
            }

            if ( oldPreferences->defaultPageLayout() != m_preferences->defaultPageLayout() )
            {
                for ( RimViewWindow* viewWindow : allViewWindows )
                {
                    RimPlotWindow* plotWindow = dynamic_cast<RimPlotWindow*>( viewWindow );
                    if ( plotWindow )
                    {
                        plotWindow->updateLayout();
                    }
                }
            }
        }

        for ( caf::PdmUiItem* uiItem : caf::SelectionManager::instance()->selectedItems() )
        {
            uiItem->updateConnectedEditors();
        }
    }
    caf::PdmUiItem::enableExtraDebugText( RiaPreferencesSystem::current()->appendFieldKeywordToToolTipText() );

    if ( oldPreferences )
    {
        onGuiPreferencesChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGuiApplication::applicationResolution()
{
    if ( auto screen = QGuiApplication::primaryScreen() )
    {
        return screen->logicalDotsPerInchX();
    }

    const int defaultDPI = 96;
    return defaultDPI;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::startMonitoringWorkProgress( caf::UiProcess* uiProcess )
{
    CAF_ASSERT( m_mainWindow );
    m_mainWindow->processMonitor()->startMonitorWorkProcess( uiProcess );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::stopMonitoringWorkProgress()
{
    CAF_ASSERT( m_mainWindow );
    m_mainWindow->processMonitor()->stopMonitorWorkProcess();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::slotWorkerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( m_mainWindow == nullptr ) return;

    m_mainWindow->processMonitor()->stopMonitorWorkProcess();

    QProcessEnvironment processEnvironment = m_workerProcess->processEnvironment();

    // Execute delete later so that other slots that are hooked up
    // get a chance to run before we delete the object
    if ( m_workerProcess )
    {
        m_workerProcess->close();
    }
    m_workerProcess = nullptr;

    // Either the work process crashed or was aborted by the user
    if ( exitStatus == QProcess::CrashExit )
    {
        //    MFLog::error("Simulation execution crashed or was aborted.");
        m_runningWorkerProcess = false;
        return;
    }

    // Exit code != 0 means we have an error
    if ( exitCode != 0 )
    {
        //  MFLog::error(QString("Simulation execution failed (exit code %1).").arg(exitCode));
        m_runningWorkerProcess = false;
        return;
    }

    // If multiple cases are present, invoke launchProcess() which will set next current case, and run script on this case
    if ( !m_scriptCaseIds.empty() )
    {
        launchProcess( m_currentProgram, m_currentArguments, processEnvironment );
    }
    else
    {
        // Disable concept of current case
        m_currentScriptCaseId  = -1;
        m_runningWorkerProcess = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onLastWindowClosed()
{
    // Qt can send the lastWindowClosed signal multiple times. Disconnect to avoid reentry.
    QObject::disconnect( this, SIGNAL( lastWindowClosed() ), this, SLOT( onLastWindowClosed() ) );

    closeProject();
    quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::runMultiCaseSnapshots( const QString&       templateProjectFileName,
                                               std::vector<QString> gridFileNames,
                                               const QString&       snapshotFolderName )
{
    if ( !m_mainWindow ) return;

    QByteArray curState = m_mainWindow->dockManager()->saveState( 0 );
    m_mainWindow->dockManager()->restoreState( RiuDockWidgetTools::defaultDockState( RiuDockWidgetTools::dockStateHideAll3DWindowName() ) );

    const size_t numGridFiles = gridFileNames.size();
    for ( size_t i = 0; i < numGridFiles; i++ )
    {
        QString gridFn = gridFileNames[i];

        RiaProjectModifier modifier;
        modifier.setReplaceCaseFirstOccurrence( gridFn );

        bool loadOk = loadProject( templateProjectFileName, ProjectLoadAction::PLA_NONE, &modifier );
        if ( loadOk )
        {
            RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( snapshotFolderName );
        }
    }
    m_mainWindow->dockManager()->restoreState( curState );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::notify( QObject* receiver, QEvent* event )
{
    // Pre-allocating a memory exhaustion message
    // Doing som e trickery to avoid deadlock, as creating a messagebox actually triggers a call to this notify method.

    static QMessageBox* memoryExhaustedBox   = nullptr;
    static bool         allocatingMessageBox = false;
    if ( !memoryExhaustedBox && !allocatingMessageBox )
    {
        allocatingMessageBox = true;
        memoryExhaustedBox   = new QMessageBox( QMessageBox::Critical,
                                              "ResInsight Exhausted Memory",
                                              "Memory is Exhausted!\n ResInsight could not allocate the memory needed, and is now "
                                                "unstable and will probably crash soon." );
    }

    bool done = false;
    try
    {
        if ( event->type() == QEvent::KeyPress )
        {
            if ( activeWindow() != mainWindow() )
            {
                QKeyEvent*     keyEvent = static_cast<QKeyEvent*>( event );
                RimPlotWindow* plot     = dynamic_cast<RimPlotWindow*>( activePlotWindow() );
                if ( plot ) done = plot->handleGlobalKeyEvent( keyEvent );
            }
        }
        else if ( event->type() == QEvent::Wheel )
        {
            if ( activeWindow() != mainWindow() )
            {
                QWheelEvent*   wheelEvent = static_cast<QWheelEvent*>( event );
                RimPlotWindow* plot       = dynamic_cast<RimPlotWindow*>( activePlotWindow() );
                if ( plot ) done = plot->handleGlobalWheelEvent( wheelEvent );
            }
        }
        if ( !done )
        {
            done = QApplication::notify( receiver, event );
        }
    }
    catch ( const std::bad_alloc& )
    {
        done = true;
        if ( memoryExhaustedBox ) memoryExhaustedBox->exec();
        std::cout << "ResInsight: Memory is Exhausted!\n ResInsight could not allocate the memory needed, and is now "
                     "unstable "
                     "and will probably crash soon."
                  << std::endl;
        // If we really want to crash instead of limping forward:
        // throw;
    }

    return done;
}
