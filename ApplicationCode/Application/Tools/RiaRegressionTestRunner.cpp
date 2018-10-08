/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RiaApplication.h"
#include "RiaGitDiff.h"
#include "RiaImageCompareReporter.h"
#include "RiaImageFileCompare.h"
#include "RiaLogging.h"
#include "RiaRegressionTest.h"
#include "RiaTextFileCompare.h"

#include "RicfCommandFileExecutor.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuViewer.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void logInfoTextWithTimeInSeconds(const QTime& time, const QString& msg)
{
    double timeRunning = time.elapsed() / 1000.0;

    QString timeText = QString("(%1 s) ").arg(timeRunning, 0, 'f', 1);

    RiaLogging::info(timeText + msg);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRegressionTestRunner::RiaRegressionTestRunner()
    : m_runningRegressionTests(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRegressionTestRunner* RiaRegressionTestRunner::instance()
{
    static RiaRegressionTestRunner* singleton = new RiaRegressionTestRunner;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::runRegressionTest(const QString& testRootPath, const QStringList& testFilter)
{
    m_runningRegressionTests = true;

    QString currentApplicationPath = QDir::currentPath();

    RiaRegressionTest regressionTestConfig;
    regressionTestConfig.readSettingsFromApplicationStore();
    if (!regressionTestConfig.folderContainingCompareTool().isEmpty())
    {
        // Windows Only : The image compare tool requires current working directory to be at the folder
        // containing the image compare tool

        QDir::setCurrent(regressionTestConfig.folderContainingCompareTool());
    }

    QString generatedFolderName = RegTestNames::generatedFolderName;
    QString diffFolderName      = RegTestNames::diffFolderName;
    QString baseFolderName      = RegTestNames::baseFolderName;
    QString regTestProjectName  = RegTestNames::testProjectName;
    QString regTestFolderFilter = RegTestNames::testFolderFilter;

    QDir testDir(testRootPath); // If string is empty it will end up as cwd
    testDir.setFilter(QDir::Dirs);
    QStringList dirNameFilter;
    dirNameFilter.append(regTestFolderFilter);
    testDir.setNameFilters(dirNameFilter);

    QFileInfoList folderList = testDir.entryInfoList();

    if (!testFilter.isEmpty())
    {
        QFileInfoList subset;

        for (auto fi : folderList)
        {
            QString path     = fi.path();
            QString baseName = fi.baseName();

            for (auto s : testFilter)
            {
                QString trimmed = s.trimmed();
                if (baseName.contains(trimmed, Qt::CaseInsensitive))
                {
                    subset.push_back(fi);
                }
            }
        }

        folderList = subset;
    }

    // delete diff and generated images

    for (int i = 0; i < folderList.size(); ++i)
    {
        QDir testCaseFolder(folderList[i].filePath());

        {
            QDir genDir(testCaseFolder.filePath(generatedFolderName));
            removeDirectoryWithContent(genDir);
        }

        {
            QDir diffDir(testCaseFolder.filePath(diffFolderName));
            removeDirectoryWithContent(diffDir);
        }

        {
            QDir generatedFiles(testCaseFolder.filePath(RegTestNames::generatedFilesFolderName));
            removeDirectoryWithContent(generatedFiles);
        }
    }

    // Generate html report

    RiaImageCompareReporter imageCompareReporter;

    // Minor workaround
    // Use registry to define if interactive diff images should be created
    // Defined by user in RiaRegressionTest
    {
        QSettings settings;

        bool useInteractiveDiff = settings.value("showInteractiveDiffImages").toBool();
        if (useInteractiveDiff)
        {
            imageCompareReporter.showInteractiveOnly();
        }
    }

    QTime timeStamp;
    timeStamp.start();

    logInfoTextWithTimeInSeconds(timeStamp, "Starting regression tests\n");

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());

        QString testFolderName            = testCaseFolder.dirName();
        QString reportBaseFolderName      = testCaseFolder.filePath(baseFolderName);
        QString reportGeneratedFolderName = testCaseFolder.filePath(generatedFolderName);
        QString reportDiffFolderName      = testCaseFolder.filePath(diffFolderName);

        imageCompareReporter.addImageDirectoryComparisonSet(testFolderName.toStdString(),
                                                            reportBaseFolderName.toStdString(),
                                                            reportGeneratedFolderName.toStdString(),
                                                            reportDiffFolderName.toStdString());
    }

    QString htmlReportFileName = testDir.filePath(RegTestNames::reportFileName);

    QString htmldiff2htmlText = diff2htmlHeaderText(testRootPath);
    imageCompareReporter.generateHTMLReport(htmlReportFileName.toStdString(), htmldiff2htmlText.toStdString());

    // Open HTML report
    QDesktopServices::openUrl(htmlReportFileName);

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());

        // Detect any command files
        QStringList filterList;
        filterList << RegTestNames::commandFileFilter;

        QFileInfoList commandFileEntries = testCaseFolder.entryInfoList(filterList);
        if (!commandFileEntries.empty())
        {
            QString currentAbsolutePath = QDir::current().absolutePath();

            // Set current path to the folder containing the command file, as this is required when using file references
            // in the command file
            QDir::setCurrent(folderList[dirIdx].filePath());

            for (const auto& fileInfo : commandFileEntries)
            {
                QString commandFile = fileInfo.absoluteFilePath();

                QFile file(commandFile);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    RiaLogging::error("Failed to open command file : " + commandFile);
                }
                else
                {
                    QTextStream in(&file);
                    RicfCommandFileExecutor::instance()->executeCommands(in);
                }
            }

            QDir::setCurrent(currentAbsolutePath);

            // Create diff based on generated folders
            {
                QString html;

                RiaTextFileCompare textFileCompare(regressionTestConfig.folderContainingDiffTool());

                QString baseFilesFolderName      = testCaseFolder.filePath(RegTestNames::baseFilesFolderName);
                QString generatedFilesFolderName = testCaseFolder.filePath(RegTestNames::generatedFilesFolderName);

                QFileInfo fib(baseFilesFolderName);
                QFileInfo fig(generatedFilesFolderName);

                if (fib.exists() && fig.exists())
                {
                    {
                        QString headerText = testCaseFolder.dirName();

                        html += "<table>\n";
                        html += "  <tr>\n";
                        html +=
                            "    <td colspan=\"3\" bgcolor=\"darkblue\" height=\"40\">  <b><font color=\"white\" size=\"3\"> " +
                            headerText + " </font></b> </td>\n";
                        html += "  </tr>\n";

                        textFileCompare.runComparison(baseFilesFolderName, generatedFilesFolderName);

                        QString diff = textFileCompare.diffOutput();
                        if (diff.isEmpty())
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

                        if (!diff.isEmpty())
                        {
                            html += QString("<code> %1 </code>").arg(diff);
                        }
                    }

                    QFile file(htmlReportFileName);
                    if (file.open(QIODevice::Append | QIODevice::Text))
                    {
                        QTextStream stream(&file);

                        stream << html;
                    }
                }
            }
        }

        QString projectFileName;

        if (testCaseFolder.exists(regTestProjectName + ".rip"))
        {
            projectFileName = regTestProjectName + ".rip";
        }

        if (testCaseFolder.exists(regTestProjectName + ".rsp"))
        {
            projectFileName = regTestProjectName + ".rsp";
        }

        if (!projectFileName.isEmpty())
        {
            logInfoTextWithTimeInSeconds(timeStamp, "Initializing test :" + testCaseFolder.absolutePath());

            RiaApplication* app = RiaApplication::instance();

            app->loadProject(testCaseFolder.filePath(projectFileName));

            // Wait until all command objects have completed
            app->waitUntilCommandObjectsHasBeenProcessed();

            regressionTestConfigureProject();

            resizeMaximizedPlotWindows();

            QString fullPathGeneratedFolder = testCaseFolder.absoluteFilePath(generatedFolderName);
            RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(fullPathGeneratedFolder);

            RicSnapshotAllPlotsToFileFeature::exportSnapshotOfAllPlotsIntoFolder(fullPathGeneratedFolder);

            QDir baseDir(testCaseFolder.filePath(baseFolderName));
            QDir genDir(testCaseFolder.filePath(generatedFolderName));
            QDir diffDir(testCaseFolder.filePath(diffFolderName));
            if (!diffDir.exists()) testCaseFolder.mkdir(diffFolderName);
            baseDir.setFilter(QDir::Files);
            QStringList baseImageFileNames = baseDir.entryList();

            for (int fIdx = 0; fIdx < baseImageFileNames.size(); ++fIdx)
            {
                QString             fileName = baseImageFileNames[fIdx];
                RiaImageFileCompare imgComparator(RegTestNames::imageCompareExeName);
                bool                ok = imgComparator.runComparison(
                    genDir.filePath(fileName), baseDir.filePath(fileName), diffDir.filePath(fileName));
                if (!ok)
                {
                    qDebug() << "Error comparing :" << imgComparator.errorMessage() << "\n" << imgComparator.errorDetails();
                }
            }

            app->closeProject();

            logInfoTextWithTimeInSeconds(timeStamp, "Completed test :" + testCaseFolder.absolutePath());
        }
        else
        {
            RiaLogging::error("Could not find a regression test file named : " + testCaseFolder.absolutePath() + "/" +
                              regTestProjectName + ".rsp");
        }
    }

    // Invoke git diff

    {
        QString    folderContainingGit = regressionTestConfig.folderContainingGitTool();
        RiaGitDiff gitDiff(folderContainingGit);
        gitDiff.executeDiff(testRootPath);

        QString diffText = gitDiff.diffOutput();
        if (!diffText.isEmpty())
        {
            QFile file(htmlReportFileName);
            if (file.open(QIODevice::Append | QIODevice::Text))
            {
                QTextStream stream(&file);

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

    RiaLogging::info("\n");
    logInfoTextWithTimeInSeconds(timeStamp, "Completed regression tests");

    QDir::setCurrent(currentApplicationPath);

    m_runningRegressionTests = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::removeDirectoryWithContent(QDir& dirToDelete)
{
    caf::Utils::removeDirectoryAndFilesRecursively(dirToDelete.absolutePath());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::regressionTestConfigureProject()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    RimProject* proj = RiaApplication::instance()->project();
    if (!proj) return;

    std::vector<RimCase*> projectCases;
    proj->allCases(projectCases);

    for (size_t i = 0; i < projectCases.size(); i++)
    {
        RimCase* cas = projectCases[i];
        if (!cas) continue;

        std::vector<Rim3dView*> views = cas->views();

        for (size_t j = 0; j < views.size(); j++)
        {
            Rim3dView* riv = views[j];

            if (riv && riv->viewer())
            {
                // Make sure all views are maximized for snapshotting
                QMdiSubWindow* subWnd = mainWnd->findMdiSubWindow(riv->viewer()->layoutWidget());
                if (subWnd)
                {
                    subWnd->showMaximized();
                }

                // This size is set to match the regression test reference images
                riv->viewer()->setFixedSize(RiaRegressionTestRunner::regressionDefaultImageSize());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::resizeMaximizedPlotWindows()
{
    RimProject* proj = RiaApplication::instance()->project();
    if (!proj) return;

    RiuPlotMainWindow* plotMainWindow = RiaApplication::instance()->mainPlotWindow();
    if (!plotMainWindow) return;

    std::vector<RimViewWindow*> viewWindows;

    proj->mainPlotCollection()->descendantsIncludingThisOfType(viewWindows);

    for (auto viewWindow : viewWindows)
    {
        if (viewWindow->isMdiWindow())
        {
            RimMdiWindowGeometry wndGeo = viewWindow->mdiWindowGeometry();
            if (wndGeo.isMaximized)
            {
                QWidget* viewWidget = viewWindow->viewWidget();

                if (viewWidget)
                {
                    QMdiSubWindow* mdiWindow = plotMainWindow->findMdiSubWindow(viewWidget);
                    if (mdiWindow)
                    {
                        mdiWindow->showNormal();

                        viewWidget->resize(RiaRegressionTestRunner::regressionDefaultImageSize());
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiaRegressionTestRunner::regressionDefaultImageSize()
{
    return QSize(1000, 745);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTestRunner::diff2htmlHeaderText(const QString& testRootPath)
{
    QString html;

    QString     oldProjPath = QDir::fromNativeSeparators(testRootPath);
    QStringList pathFolders = oldProjPath.split("/", QString::KeepEmptyParts);

    QString path;
    for (const auto& f : pathFolders)
    {
        path += f;
        path += "/";
        if (f == "ResInsight-regression-test") break;
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
        html                    = html.replace("dist/", pathToDiff2html);
    }

    return html;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::executeRegressionTests()
{
    RiaRegressionTest testConfig;
    testConfig.readSettingsFromApplicationStore();

    QString     testPath   = testConfig.regressionTestFolder();
    QStringList testFilter = testConfig.testFilter().split(";", QString::SkipEmptyParts);

    executeRegressionTests(testPath, testFilter);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaRegressionTestRunner::executeRegressionTests(const QString& regressionTestPath, const QStringList& testFilter)
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (mainWnd)
    {
        mainWnd->hideAllDockWindows();
        mainWnd->statusBar()->close();

        mainWnd->setDefaultWindowSize();
        runRegressionTest(regressionTestPath, testFilter);

        mainWnd->loadWinGeoAndDockToolBarLayout();
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
void RiaRegressionTestRunner::updateRegressionTest(const QString& testRootPath)
{
    // Find all sub folders

    QDir testDir(testRootPath); // If string is empty it will end up as cwd
    testDir.setFilter(QDir::Dirs);
    QStringList dirNameFilter;
    dirNameFilter.append(RegTestNames::testFolderFilter);
    testDir.setNameFilters(dirNameFilter);

    QFileInfoList folderList = testDir.entryInfoList();

    for (int i = 0; i < folderList.size(); ++i)
    {
        QDir testCaseFolder(folderList[i].filePath());

        QDir baseDir(testCaseFolder.filePath(RegTestNames::baseFolderName));
        removeDirectoryWithContent(baseDir);
        testCaseFolder.mkdir(RegTestNames::baseFolderName);

        QDir genDir(testCaseFolder.filePath(RegTestNames::generatedFolderName));

        QStringList imageFileNames = genDir.entryList();

        for (int fIdx = 0; fIdx < imageFileNames.size(); ++fIdx)
        {
            QString fileName = imageFileNames[fIdx];
            QFile::copy(genDir.filePath(fileName), baseDir.filePath(fileName));
        }
    }
}
