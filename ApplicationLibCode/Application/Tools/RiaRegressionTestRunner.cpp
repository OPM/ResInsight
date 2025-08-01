/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaRegressionTestRunner.h"

#include "RiaGitDiff.h"
#include "RiaGuiApplication.h"
#include "RiaImageCompareReporter.h"
#include "RiaImageFileCompare.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaProjectModifier.h"
#include "RiaRegressionTest.h"
#include "RiaTextFileCompare.h"
#include "RiaTextStringTools.h"

#include "RicfCommandFileExecutor.h"

#include "RimEclipseResultAddress.h"
#include "RimFaultInView.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuDockWidgetTools.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowTools.h"
#include "RiuPlotMainWindow.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"

#include "DockManager.h"

#include "cafMemoryInspector.h"
#include "cafPdmUiTreeView.h"
#include "cafUtils.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QMdiSubWindow>
#include <QSettings>
#include <QStatusBar>
#include <QString>
#include <QUrl>

namespace RegTestNames
{
const QString generatedFilesFolderName = "RegTestGeneratedFiles";
const QString baseFilesFolderName      = "RegTestBaseFiles";
const QString generatedFolderName      = "RegTestGeneratedImages";
const QString diffFolderName           = "RegTestDiffImages";
const QString baseFolderName           = "RegTestBaseImages";
const QString testProjectName          = "RegressionTest";
const QString testFolderFilter         = "TestCase*";
const QString imageCompareExeName      = "compare";
const QString reportFileName           = "ResInsightRegressionTestReport.html";
const QString commandFileFilter        = "commandfile-*";
}; // namespace RegTestNames

RiaRegressionTestRunner* RiaRegressionTestRunner::sm_singleton = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void logInfoTextWithTimeInSeconds( const QElapsedTimer& time, const QString& msg )
{
    double timeRunning = time.elapsed() / 1000.0;

    QString timeText = QString( "(%1 s) " ).arg( timeRunning, 0, 'f', 1 );

    RiaLogging::info( timeText + msg );
    QApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRegressionTestRunner::RiaRegressionTestRunner()
    : m_runningRegressionTests( false )
    , m_appendAllTestsAfterLastItemInFilter( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRegressionTestRunner* RiaRegressionTestRunner::instance()
{
    CAF_ASSERT( sm_singleton );
    return sm_singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::createSingleton()
{
    if ( !sm_singleton ) sm_singleton = new RiaRegressionTestRunner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::deleteSingleton()
{
    if ( sm_singleton ) delete sm_singleton;
    sm_singleton = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::runRegressionTest()
{
    m_runningRegressionTests = true;

    QString currentApplicationPath = QDir::currentPath();

    std::vector<std::tuple<QString, double, uint64_t, uint64_t>> memoryUsagePerTest;

    RiaRegressionTest regressionTestConfig;
    regressionTestConfig.readSettingsFromApplicationStore();
    if ( !regressionTestConfig.folderContainingCompareTool().isEmpty() )
    {
        // Windows Only : The image compare tool requires current working directory to be at the folder
        // containing the image compare tool

        QDir::setCurrent( regressionTestConfig.folderContainingCompareTool() );
    }

    QString generatedFolderName = RegTestNames::generatedFolderName;
    QString diffFolderName      = RegTestNames::diffFolderName;
    QString baseFolderName      = RegTestNames::baseFolderName;
    QString regTestProjectName  = RegTestNames::testProjectName;
    QString regTestFolderFilter = RegTestNames::testFolderFilter;

    QDir testDir( m_rootPath ); // If string is empty it will end up as cwd
    testDir.setFilter( QDir::Dirs );
    QStringList dirNameFilter;
    dirNameFilter.append( regTestFolderFilter );
    testDir.setNameFilters( dirNameFilter );

    QFileInfoList folderList = subDirectoriesForTestExecution( testDir );

    // delete diff and generated images

    for ( const QFileInfo& fi : folderList )
    {
        QDir testCaseFolder( fi.filePath() );

        {
            QDir genDir( testCaseFolder.filePath( generatedFolderName ) );
            removeDirectoryWithContent( genDir );
        }

        {
            QDir diffDir( testCaseFolder.filePath( diffFolderName ) );
            removeDirectoryWithContent( diffDir );
        }

        {
            QDir generatedFiles( testCaseFolder.filePath( RegTestNames::generatedFilesFolderName ) );
            removeDirectoryWithContent( generatedFiles );
        }
    }

    QString htmlReportFileName = generateHtmlReport( folderList, baseFolderName, generatedFolderName, diffFolderName, testDir );

    if ( regressionTestConfig.openReportInBrowser() )
    {
        QDesktopServices::openUrl( htmlReportFileName );
    }

    RiaLogging::info( "--------------------------------------------------" );
    RiaLogging::info( QTime::currentTime().toString() + ": Launching regression tests" );
    RiaLogging::info( "--------------------------------------------------" );

    QElapsedTimer timeStamp;
    timeStamp.start();
    logInfoTextWithTimeInSeconds( timeStamp, "Starting regression tests\n" );

    RiaApplication* app = RiaApplication::instance();

    for ( const QFileInfo& folderFileInfo : folderList )
    {
        QDir testCaseFolder( folderFileInfo.filePath() );

        bool anyCommandFilesExecuted = findAndExecuteCommandFiles( testCaseFolder, regressionTestConfig, htmlReportFileName );

        if ( !anyCommandFilesExecuted )
        {
            QString projectFileName;

            if ( testCaseFolder.exists( regTestProjectName + ".rip" ) )
            {
                projectFileName = regTestProjectName + ".rip";
            }

            if ( testCaseFolder.exists( regTestProjectName + ".rsp" ) )
            {
                projectFileName = regTestProjectName + ".rsp";
            }

            if ( !projectFileName.isEmpty() )
            {
                QElapsedTimer timerForOneTest;
                timerForOneTest.start();

                cvf::ref<RiaProjectModifier> projectModifier;
                if ( regressionTestConfig.invalidateExternalFilePaths )
                {
                    projectModifier = cvf::make_ref<RiaProjectModifier>();
                    projectModifier->setInvalidateExternalFilePaths();
                }
                logInfoTextWithTimeInSeconds( timeStamp, "Initializing test :" + testCaseFolder.absolutePath() );

                app->loadProject( testCaseFolder.filePath( projectFileName ), RiaApplication::ProjectLoadAction::PLA_NONE, projectModifier.p() );

                RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdates();

                QString fullPathGeneratedFolder = testCaseFolder.absoluteFilePath( generatedFolderName );
                if ( regressionTestConfig.exportSnapshots3dViews )
                {
                    setDefaultSnapshotSizeFor3dViews();

                    RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( fullPathGeneratedFolder );

                    QApplication::processEvents();
                }

                if ( regressionTestConfig.exportSnapshotsPlots )
                {
                    setDefaultSnapshotSizeForPlotWindows();

                    RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotsIntoFolder( fullPathGeneratedFolder );
                }

                uint64_t usedMemoryBeforeClose = caf::MemoryInspector::getApplicationPhysicalMemoryUsageMiB();

                if ( regressionTestConfig.activateObjectsInPropertyEditor )
                {
                    RiaRegressionTestRunner::selectObjectsInProject();
                }

                app->closeProject();

                QApplication::processEvents();

                auto testDuration = timerForOneTest.elapsed() / 1000.0;

                uint64_t usedMemoryAfterClose = caf::MemoryInspector::getApplicationPhysicalMemoryUsageMiB();

                auto folderName = folderFileInfo.baseName();
                memoryUsagePerTest.push_back( { folderName, testDuration, usedMemoryBeforeClose, usedMemoryAfterClose } );
            }
            else
            {
                RiaLogging::error( "Could not find a regression test file named : " + testCaseFolder.absolutePath() + "/" +
                                   regTestProjectName + ".rsp" );
            }
        }

        // Do a complete reset of project settings to avoid transfer of settings to next regression test
        app->resetProject();

        QDir baseDir( testCaseFolder.filePath( baseFolderName ) );
        QDir genDir( testCaseFolder.filePath( generatedFolderName ) );
        QDir diffDir( testCaseFolder.filePath( diffFolderName ) );
        if ( !diffDir.exists() ) testCaseFolder.mkdir( diffFolderName );
        baseDir.setFilter( QDir::Files );
        QStringList baseImageFileNames = baseDir.entryList();

        for ( int fIdx = 0; fIdx < baseImageFileNames.size(); ++fIdx )
        {
            QString             fileName = baseImageFileNames[fIdx];
            RiaImageFileCompare imgComparator( RegTestNames::imageCompareExeName );
            bool ok = imgComparator.runComparison( genDir.filePath( fileName ), baseDir.filePath( fileName ), diffDir.filePath( fileName ) );
            if ( !ok )
            {
                qDebug() << "Error comparing :" << imgComparator.errorMessage() << "\n" << imgComparator.errorDetails();
            }
        }

        logInfoTextWithTimeInSeconds( timeStamp, "Completed test :" + testCaseFolder.absolutePath() );
    }

    // Profiling logging
    RiaLogging::info( "| Duration [sec] | Before Close [MB] | After Close [MB] | Name " );
    for ( const auto& [name, testDuration, beforeMemory, afterMemory] : memoryUsagePerTest )
    {
        auto timeText = QString( "(%1 s) " ).arg( testDuration, 5, 'f', 1 );
        auto logInfo  = QString( "%1 %2    %3 %4" ).arg( timeText ).arg( beforeMemory ).arg( afterMemory ).arg( name );
        RiaLogging::info( logInfo );
    }

    // Invoke git diff

    {
        QString    folderContainingGit = regressionTestConfig.folderContainingGitTool();
        RiaGitDiff gitDiff( folderContainingGit );
        gitDiff.executeDiff( m_rootPath );

        QString diffText = gitDiff.diffOutput();
        if ( !diffText.isEmpty() )
        {
            QFile file( htmlReportFileName );
            if ( file.open( QIODevice::Append | QIODevice::Text ) )
            {
                QTextStream stream( &file );

                QString divSectionForDiff = R"(
<div id = "destination-elem-id"[innerHtml] = "outputHtml">
original
</div>

)";
                stream << divSectionForDiff;
                stream << "</body>";

                {
                    QString generateDiffString = R"(
<script type="text/javascript">

function generateDiff() {
return `
)";

                    generateDiffString += diffText;

                    generateDiffString += R"(
`;
};
)";

                    generateDiffString += R"(
var diffHtml = Diff2Html.getPrettyHtml(
    generateDiff(),
    {inputFormat: 'diff', showFiles: true, matching: 'lines', outputFormat: 'side-by-side'}
);

document.getElementById("destination-elem-id").innerHTML = diffHtml;
</script>
</html>
)";
                    stream << generateDiffString;
                }
            }
        }
    }

    RiaLogging::info( "\n" );
    logInfoTextWithTimeInSeconds( timeStamp, "Completed regression tests" );

    QDir::setCurrent( currentApplicationPath );

    m_runningRegressionTests = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaRegressionTestRunner::findAndExecuteCommandFiles( const QDir&              testCaseFolder,
                                                          const RiaRegressionTest& regressionTestConfig,
                                                          const QString&           htmlReportFileName )
{
    QStringList filterList;
    filterList << RegTestNames::commandFileFilter;

    QFileInfoList commandFileEntries = testCaseFolder.entryInfoList( filterList );
    if ( commandFileEntries.empty() )
    {
        return false;
    }

    QString currentAbsolutePath = QDir::current().absolutePath();

    // Set current path to the folder containing the command file, as this is required when using file references
    // in the command file
    QDir::setCurrent( testCaseFolder.path() );

    for ( const auto& fileInfo : commandFileEntries )
    {
        QString commandFile = fileInfo.absoluteFilePath();

        QFile file( commandFile );
        if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            RiaLogging::error( "Failed to open command file : " + commandFile );
        }
        else
        {
            QTextStream in( &file );
            RicfCommandFileExecutor::instance()->executeCommands( in );
        }
    }

    QDir::setCurrent( currentAbsolutePath );

    // Create diff based on generated folders
    {
        QString html;

        RiaTextFileCompare textFileCompare( regressionTestConfig.folderContainingDiffTool() );

        QString baseFilesFolderName      = testCaseFolder.filePath( RegTestNames::baseFilesFolderName );
        QString generatedFilesFolderName = testCaseFolder.filePath( RegTestNames::generatedFilesFolderName );

        QFileInfo fib( baseFilesFolderName );
        QFileInfo fig( generatedFilesFolderName );

        if ( fib.exists() && fig.exists() )
        {
            {
                QString headerText = testCaseFolder.dirName();

                html += "<table>\n";
                html += "  <tr>\n";
                html += "    <td colspan=\"3\" bgcolor=\"darkblue\" height=\"40\">  <b><font color=\"white\" size=\"3\"> " + headerText +
                        " </font></b> </td>\n";
                html += "  </tr>\n";

                textFileCompare.runComparison( baseFilesFolderName, generatedFilesFolderName );

                QString diff = textFileCompare.diffOutput();
                if ( diff.isEmpty() )
                {
                    html += "  <tr>\n";
                    html += "    <td colspan=\"3\" bgcolor=\"lightgray\"> <font color=\"green\">No text diff "
                            "detected</font> </td> \n";
                    html += "  </tr>\n";
                }
                else
                {
                    html += "  <tr>\n";
                    html += "    <td colspan=\"3\" bgcolor=\"lightgray\"> <font color=\"red\">Text diff detected - "
                            "output from diff tool : </font> </td> \n";
                    html += "  </tr>\n";
                }

                // Table end
                html += "</table>\n";

                if ( !diff.isEmpty() )
                {
                    html += QString( "<code> %1 </code>" ).arg( diff );
                }
            }

            QFile file( htmlReportFileName );
            if ( file.open( QIODevice::Append | QIODevice::Text ) )
            {
                QTextStream stream( &file );

                stream << html;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTestRunner::generateHtmlReport( const QFileInfoList& folderList,
                                                     const QString&       baseFolderName,
                                                     const QString&       generatedFolderName,
                                                     const QString&       diffFolderName,
                                                     const QDir&          testDir )
{
    RiaImageCompareReporter imageCompareReporter;

    // Minor workaround
    // Use registry to define if interactive diff images should be created
    // Defined by user in RiaRegressionTest
    {
        QSettings settings;

        bool useInteractiveDiff = settings.value( "showInteractiveDiffImages" ).toBool();
        if ( useInteractiveDiff )
        {
            imageCompareReporter.showInteractiveOnly();
        }
    }

    for ( const QFileInfo& fi : folderList )
    {
        QDir testCaseFolder( fi.filePath() );

        QString testFolderName            = testCaseFolder.dirName();
        QString reportBaseFolderName      = testCaseFolder.filePath( baseFolderName );
        QString reportGeneratedFolderName = testCaseFolder.filePath( generatedFolderName );
        QString reportDiffFolderName      = testCaseFolder.filePath( diffFolderName );

        imageCompareReporter.addImageDirectoryComparisonSet( testFolderName.toStdString(),
                                                             reportBaseFolderName.toStdString(),
                                                             reportGeneratedFolderName.toStdString(),
                                                             reportDiffFolderName.toStdString() );
    }

    QString htmlReportFileName = testDir.filePath( RegTestNames::reportFileName );

    QString htmldiff2htmlText = diff2htmlHeaderText( m_rootPath );
    imageCompareReporter.generateHTMLReport( htmlReportFileName.toStdString(), htmldiff2htmlText.toStdString() );

    return htmlReportFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::removeDirectoryWithContent( QDir& dirToDelete )
{
    caf::Utils::removeDirectoryAndFilesRecursively( dirToDelete.absolutePath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::setDefaultSnapshotSizeFor3dViews()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if ( !mainWnd ) return;

    QSize defaultSize = RiaRegressionTestRunner::regressionDefaultImageSize();

    RiuMainWindowTools::setFixedWindowSizeFor3dViews( mainWnd, defaultSize.width(), defaultSize.height() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::setDefaultSnapshotSizeForPlotWindows()
{
    RiuPlotMainWindow* plotMainWindow = RiaGuiApplication::instance()->mainPlotWindow();
    if ( !plotMainWindow ) return;

    QSize defaultSize = RiaRegressionTestRunner::regressionDefaultImageSize();

    RiuMainWindowTools::setWindowSizeOnWidgetsInMdiWindows( plotMainWindow, defaultSize.width(), defaultSize.height() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiaRegressionTestRunner::regressionDefaultImageSize()
{
    return QSize( 1000, 745 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTestRunner::diff2htmlHeaderText( const QString& testRootPath )
{
    QString html;

    QString     oldProjPath = QDir::fromNativeSeparators( testRootPath );
    QStringList pathFolders = oldProjPath.split( "/" );

    QString path;
    for ( const auto& f : pathFolders )
    {
        if ( f.compare( "ProjectFiles", Qt::CaseInsensitive ) == 0 ) break;
        path += f;
        path += "/";
    }

    {
        html = R"(
<!-- CSS -->
<link rel = "stylesheet" type = "text/css" href = "dist/diff2html.css">

<!--Javascripts-->
<script type = "text/javascript" src = "dist/diff2html.js"></script>
<script type = "text/javascript" src = "dist/diff2html-ui.js"></script>
)";

        QString pathToDiff2html = path + "diff2html/dist/";
        html                    = html.replace( "dist/", pathToDiff2html );
    }

    return html;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFileInfoList RiaRegressionTestRunner::subDirectoriesForTestExecution( const QDir& directory )
{
    if ( m_testFilter.isEmpty() )
    {
        QFileInfoList folderList = directory.entryInfoList();

        return folderList;
    }

    bool anyMatchFound = false;

    QFileInfoList foldersMatchingTestFilter;

    QFileInfoList folderList = directory.entryInfoList();
    for ( const auto& fi : folderList )
    {
        QString path     = fi.path();
        QString baseName = fi.baseName();

        for ( const auto& s : m_testFilter )
        {
            QString trimmed = s.trimmed();
            if ( ( m_appendAllTestsAfterLastItemInFilter && anyMatchFound ) || baseName.compare( trimmed, Qt::CaseInsensitive ) == 0 )
            {
                foldersMatchingTestFilter.push_back( fi );
                anyMatchFound = true;
            }
        }
    }

    return foldersMatchingTestFilter;
}

//--------------------------------------------------------------------------------------------------
/// Find relevant object to show in property editor. Loop through all objects in project and select them, this will activate the Property
/// Editor. Avoid RimSummaryCaseMainCollection, as this container contains all the summary addresses, and can potentially contain extremely
/// many objects
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::selectObjectsInProject()
{
    auto project = RimProject::current();
    if ( !project ) return;

    std::vector<caf::PdmObject*> baseObjects;

    auto oilField = project->activeOilField();

    std::vector<caf::PdmFieldHandle*> fields = oilField->fields();
    for ( auto f : fields )
    {
        std::vector<caf::PdmObjectHandle*> childObjects = f->children();
        for ( auto childObject : childObjects )
        {
            // Skip RimSummaryCaseMainCollection, as this container contains all the summary addresses, and can potentially contain extemely
            // many objects
            if ( dynamic_cast<RimSummaryCaseMainCollection*>( childObject ) != nullptr )
            {
                continue;
            }

            caf::PdmObject* pdmObjectChild = dynamic_cast<caf::PdmObject*>( childObject );
            if ( pdmObjectChild )
            {
                baseObjects.push_back( pdmObjectChild );
            }
        }
    }

    baseObjects.push_back( project->mainPlotCollection() );

    std::vector<caf::PdmObject*> allObjects;
    for ( auto baseObj : baseObjects )
    {
        auto objectsForSelection = baseObj->descendantsIncludingThisOfType<caf::PdmObject>();

        allObjects.insert( allObjects.end(), objectsForSelection.begin(), objectsForSelection.end() );
    }

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

    auto mainWindow     = RiuMainWindow::instance();
    auto plotMainWindow = RiuPlotMainWindow::instance();

    for ( auto obj : allObjects )
    {
        if ( dynamic_cast<RimFaultInView*>( obj ) ) continue;
        if ( dynamic_cast<RimEclipseResultAddress*>( obj ) ) continue;
        if ( dynamic_cast<RimSimWellInView*>( obj ) ) continue;

        if ( auto treeView = mainWindow->getTreeViewWithItem( obj ) )
        {
            treeView->selectItems( { obj } );

            QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        }

        if ( auto treeView = plotMainWindow->getTreeViewWithItem( obj ) )
        {
            treeView->selectItems( { obj } );

            QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::executeRegressionTests()
{
    RiaRegressionTest testConfig;
    testConfig.readSettingsFromApplicationStore();

    QString     testPath   = testConfig.regressionTestFolder();
    QStringList testFilter = RiaTextStringTools::splitSkipEmptyParts( testConfig.testFilter(), ";" );

    if ( testConfig.appendTestsAfterTestFilter )
    {
        m_appendAllTestsAfterLastItemInFilter = true;
    }

    executeRegressionTests( testPath, testFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::executeRegressionTests( const QString& regressionTestPath, const QStringList& testFilter )
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if ( mainWnd )
    {
        mainWnd->statusBar()->close();
        QByteArray curState = mainWnd->dockManager()->saveState( 0 );
        mainWnd->dockManager()->restoreState( RiuDockWidgetTools::defaultDockState( RiuDockWidgetTools::dockStateHideAll3DWindowName() ) );

        mainWnd->setDefaultWindowSize();

        m_regressionTestSettings.readSettingsFromApplicationStore();

        m_rootPath   = regressionTestPath;
        m_testFilter = testFilter;
        runRegressionTest();
        mainWnd->dockManager()->restoreState( curState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaRegressionTestRunner::isRunningRegressionTests() const
{
    return m_runningRegressionTests;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaRegressionTestRunner::useOpenMPForGeometryCreation() const
{
    if ( !m_runningRegressionTests ) return false;

    return m_regressionTestSettings.useOpenMPForGeometryCreation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::updateRegressionTest( const QString& testRootPath )
{
    // Find all sub folders

    QDir testDir( testRootPath ); // If string is empty it will end up as cwd
    testDir.setFilter( QDir::Dirs );
    QStringList dirNameFilter;
    dirNameFilter.append( RegTestNames::testFolderFilter );
    testDir.setNameFilters( dirNameFilter );

    QFileInfoList folderList = testDir.entryInfoList();

    for ( const auto& fi : folderList )
    {
        QDir testCaseFolder( fi.filePath() );

        QDir baseDir( testCaseFolder.filePath( RegTestNames::baseFolderName ) );
        removeDirectoryWithContent( baseDir );
        testCaseFolder.mkdir( RegTestNames::baseFolderName );

        QDir genDir( testCaseFolder.filePath( RegTestNames::generatedFolderName ) );

        QStringList imageFileNames = genDir.entryList();

        for ( int fIdx = 0; fIdx < imageFileNames.size(); ++fIdx )
        {
            QString fileName = imageFileNames[fIdx];
            QFile::copy( genDir.filePath( fileName ), baseDir.filePath( fileName ) );
        }
    }
}
