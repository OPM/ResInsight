/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaArgumentParser.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaProjectModifier.h"
#include "RiaRegressionTestRunner.h"

#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "RicfCommandFileExecutor.h"
#include "RicfMessages.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "RicImportSummaryCasesFeature.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include "cafUtils.h"

#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaArgumentParser::parseArguments()
{
    cvf::ProgramOptions progOpt;
    progOpt.registerOption("last", "", "Open last used project.");
    progOpt.registerOption("project", "<filename>", "Open project file <filename>.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("case",
                           "<casename>",
                           "Import Eclipse case <casename> (do not include the .GRID/.EGRID extension.)",
                           cvf::ProgramOptions::MULTI_VALUE);
    progOpt.registerOption("startdir", "<folder>", "Set startup directory.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("savesnapshots",
                           "all|views|plots",
                           "Save snapshot of all views or plots to project file location sub folder 'snapshots'. Option 'all' "
                           "will include both views and plots. Application closes after snapshots have been written.",
                           cvf::ProgramOptions::OPTIONAL_MULTI_VALUE);
    progOpt.registerOption(
        "size", "<width> <height>", "Set size of the main application window.", cvf::ProgramOptions::MULTI_VALUE);
    progOpt.registerOption(
        "replaceCase",
        "[<caseId>] <newGridFile>",
        "Replace grid in <caseId> or first case with <newgridFile>. Repeat parameter for multiple replace operations.",
        cvf::ProgramOptions::MULTI_VALUE,
        cvf::ProgramOptions::COMBINE_REPEATED);
    progOpt.registerOption("replaceSourceCases",
                           "[<caseGroupId>] <gridListFile>",
                           "Replace source cases in <caseGroupId> or first grid case group with the grid files listed in the "
                           "<gridListFile> file. Repeat parameter for multiple replace operations.",
                           cvf::ProgramOptions::MULTI_VALUE,
                           cvf::ProgramOptions::COMBINE_REPEATED);
    progOpt.registerOption("replacePropertiesFolder",
                           "[<caseId>] <newPropertiesFolder>",
                           "Replace the folder containing property files for an eclipse input case.",
                           cvf::ProgramOptions::MULTI_VALUE);
    progOpt.registerOption("multiCaseSnapshots",
                           "<gridListFile>",
                           "For each grid file listed in the <gridListFile> file, replace the first case in the project and save "
                           "snapshot of all views.",
                           cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("commandFile", "<commandfile>", "Execute the command file.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption(
        "commandFileProject",
        "<filename>",
        "Project to use if performing case looping for command file. Used in conjunction with 'commandFileReplaceCases'.",
        cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("commandFileReplaceCases",
                           "[<caseId>] <caseListFile>",
                           "Supply list of cases to replace in project, performing command file for each case.",
                           cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("help", "", "Displays help text.");
    progOpt.registerOption("?", "", "Displays help text.");
    progOpt.registerOption("regressiontest", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("updateregressiontestbase", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt.registerOption("unittest", "", "System command");
    progOpt.registerOption("ignoreArgs", "", "Ignore all arguments. Mostly for testing purposes");

    progOpt.setOptionPrefix(cvf::ProgramOptions::DOUBLE_DASH);

    QString helpText = QString("\n%1 v. %2\n").arg(RI_APPLICATION_NAME).arg(RiaApplication::getVersionStringApp(false));
    helpText += "Copyright Equinor ASA, Ceetron Solution AS, Ceetron AS\n\n";

    const cvf::String usageText = progOpt.usageText(110, 30);
    helpText += cvfqt::Utils::toQString(usageText);

    RiaApplication::instance()->setHelpText(helpText);

    QStringList arguments = QCoreApplication::arguments();

    bool parseOk = progOpt.parse(cvfqt::Utils::toStringVector(arguments));

    // If positional parameter functionality is to be supported, the test for existence of positionalParameters must be removed
    // This is based on a pull request by @andlaus https://github.com/OPM/ResInsight/pull/162
    if (!parseOk || progOpt.hasOption("help") || progOpt.hasOption("?") || !progOpt.positionalParameters().empty())
    {
#if defined(_MSC_VER) && defined(_WIN32)
        RiaApplication::instance()->showFormattedTextInMessageBox(helpText);
#else
        fprintf(stdout, "%s\n", helpText.toLatin1().data());
        fflush(stdout);
#endif
        return false;
    }

    // Handling of the actual command line options
    // --------------------------------------------------------

    if (cvf::Option o = progOpt.option("ignoreArgs"))
    {
        return true;
    }

    if (cvf::Option o = progOpt.option("regressiontest"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        QString regressionTestPath = cvfqt::Utils::toQString(o.value(0));

        // Use a logger writing to stdout instead of message panel
        // This is useful when executing regression tests on a build server, and this is the reason for creating the logger when
        // parsing the command line options
        auto stdLogger = new RiaStdOutLogger;
        stdLogger->setLevel(RI_LL_DEBUG);

        RiaLogging::setLoggerInstance(stdLogger);

        RiaRegressionTestRunner::instance()->executeRegressionTests(regressionTestPath, QStringList());
        return false;
    }

    if (cvf::Option o = progOpt.option("updateregressiontestbase"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        QString regressionTestPath = cvfqt::Utils::toQString(o.value(0));
        RiaRegressionTestRunner::instance()->updateRegressionTest(regressionTestPath);
        return false;
    }

    if (cvf::Option o = progOpt.option("startdir"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        RiaApplication::instance()->setStartDir(cvfqt::Utils::toQString(o.value(0)));
    }

    if (cvf::Option o = progOpt.option("size"))
    {
        RiuMainWindow* mainWnd = RiuMainWindow::instance();
        int            width   = o.safeValue(0).toInt(-1);
        int            height  = o.safeValue(1).toInt(-1);
        if (mainWnd && width > 0 && height > 0)
        {
            mainWnd->resize(width, height);
        }
    }

    QString projectFileName;

    if (progOpt.hasOption("last"))
    {
        projectFileName = RiaApplication::instance()->preferences()->lastUsedProjectFileName;
    }

    if (cvf::Option o = progOpt.option("project"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        projectFileName = cvfqt::Utils::toQString(o.value(0));
    }

    if (!projectFileName.isEmpty())
    {
        if (cvf::Option o = progOpt.option("multiCaseSnapshots"))
        {
            QString              gridListFile = cvfqt::Utils::toQString(o.safeValue(0));
            std::vector<QString> gridFiles    = RiaApplication::readFileListFromTextFile(gridListFile);
            RiaApplication::instance()->runMultiCaseSnapshots(projectFileName, gridFiles, "multiCaseSnapshots");

            return false;
        }
    }

    if (!projectFileName.isEmpty())
    {
        cvf::ref<RiaProjectModifier>      projectModifier;
        RiaApplication::ProjectLoadAction projectLoadAction = RiaApplication::PLA_NONE;

        if (cvf::Option o = progOpt.option("replaceCase"))
        {
            if (projectModifier.isNull()) projectModifier = new RiaProjectModifier;

            if (o.valueCount() == 1)
            {
                // One argument is available, use replace case for first occurrence in the project

                QString gridFileName = cvfqt::Utils::toQString(o.safeValue(0));
                projectModifier->setReplaceCaseFirstOccurrence(gridFileName);
            }
            else
            {
                size_t optionIdx = 0;
                while (optionIdx < o.valueCount())
                {
                    const int caseId       = o.safeValue(optionIdx++).toInt(-1);
                    QString   gridFileName = cvfqt::Utils::toQString(o.safeValue(optionIdx++));

                    if (caseId != -1 && !gridFileName.isEmpty())
                    {
                        projectModifier->setReplaceCase(caseId, gridFileName);
                    }
                }
            }
        }

        if (cvf::Option o = progOpt.option("replaceSourceCases"))
        {
            if (projectModifier.isNull()) projectModifier = new RiaProjectModifier;

            if (o.valueCount() == 1)
            {
                // One argument is available, use replace case for first occurrence in the project

                std::vector<QString> gridFileNames =
                    RiaApplication::readFileListFromTextFile(cvfqt::Utils::toQString(o.safeValue(0)));
                projectModifier->setReplaceSourceCasesFirstOccurrence(gridFileNames);
            }
            else
            {
                size_t optionIdx = 0;
                while (optionIdx < o.valueCount())
                {
                    const int            groupId = o.safeValue(optionIdx++).toInt(-1);
                    std::vector<QString> gridFileNames =
                        RiaApplication::readFileListFromTextFile(cvfqt::Utils::toQString(o.safeValue(optionIdx++)));

                    if (groupId != -1 && !gridFileNames.empty())
                    {
                        projectModifier->setReplaceSourceCasesById(groupId, gridFileNames);
                    }
                }
            }

            projectLoadAction = RiaApplication::PLA_CALCULATE_STATISTICS;
        }

        if (cvf::Option o = progOpt.option("replacePropertiesFolder"))
        {
            if (projectModifier.isNull()) projectModifier = new RiaProjectModifier;

            if (o.valueCount() == 1)
            {
                QString propertiesFolder = cvfqt::Utils::toQString(o.safeValue(0));
                projectModifier->setReplacePropertiesFolderFirstOccurrence(propertiesFolder);
            }
            else
            {
                size_t optionIdx = 0;
                while (optionIdx < o.valueCount())
                {
                    const int caseId           = o.safeValue(optionIdx++).toInt(-1);
                    QString   propertiesFolder = cvfqt::Utils::toQString(o.safeValue(optionIdx++));

                    if (caseId != -1 && !propertiesFolder.isEmpty())
                    {
                        projectModifier->setReplacePropertiesFolder(caseId, propertiesFolder);
                    }
                }
            }
        }

        RiaApplication::instance()->loadProject(projectFileName, projectLoadAction, projectModifier.p());
    }

    if (cvf::Option o = progOpt.option("case"))
    {
        QStringList caseNames = cvfqt::Utils::toQStringList(o.values());
        for (const QString& caseName : caseNames)
        {
            openCaseFromCommandLineParameter(caseName);
        }
    }

    if (cvf::Option o = progOpt.option("savesnapshots"))
    {
        bool snapshotViews = false;
        bool snapshotPlots = false;

        QStringList snapshotItemTexts = cvfqt::Utils::toQStringList(o.values());
        if (snapshotItemTexts.empty())
        {
            // No options will keep backwards compatibility before we introduced snapshot of plots
            snapshotViews = true;
        }

        for (const QString& s : snapshotItemTexts)
        {
            if (s.toLower() == "all")
            {
                snapshotViews = true;
                snapshotPlots = true;
            }
            else if (s.toLower() == "views")
            {
                snapshotViews = true;
            }
            else if (s.toLower() == "plots")
            {
                snapshotPlots = true;
            }
        }

        if (RiaApplication::instance()->project() != nullptr && !RiaApplication::instance()->project()->fileName().isEmpty())
        {
            if (snapshotViews)
            {
                RiuMainWindow* mainWnd = RiuMainWindow::instance();
                CVF_ASSERT(mainWnd);
                mainWnd->hideAllDockWindows();

                // 2016-11-09 : Location of snapshot folder was previously located in 'snapshot' folder
                // relative to current working folder. Now harmonized to behave as RiuMainWindow::slotSnapshotAllViewsToFile()
                QString absolutePathToSnapshotDir =
                    RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("snapshots");
                RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(absolutePathToSnapshotDir);

                mainWnd->loadWinGeoAndDockToolBarLayout();
            }

            if (snapshotPlots)
            {
                if (RiaApplication::instance()->mainPlotWindow())
                {
                    RiaApplication::instance()->mainPlotWindow()->hideAllDockWindows();

                    // Will be saved relative to current directory
                    RicSnapshotAllPlotsToFileFeature::saveAllPlots();

                    RiaApplication::instance()->mainPlotWindow()->loadWinGeoAndDockToolBarLayout();
                }
            }
        }

        // Returning false will exit the application
        return false;
    }

    if (cvf::Option o = progOpt.option("commandFile"))
    {
        QString commandFile = cvfqt::Utils::toQString(o.safeValue(0));

        cvf::Option projectOption = progOpt.option("commandFileProject");
        cvf::Option caseOption    = progOpt.option("commandFileReplaceCases");
        if (projectOption && caseOption)
        {
            projectFileName = cvfqt::Utils::toQString(projectOption.value(0));

            std::vector<int>     caseIds;
            std::vector<QString> caseListFiles;

            if (caseOption.valueCount() == 1)
            {
                caseListFiles.push_back(cvfqt::Utils::toQString(caseOption.safeValue(0)));
            }
            else
            {
                size_t optionIdx = 0;
                while (optionIdx < caseOption.valueCount())
                {
                    const int caseId       = caseOption.safeValue(optionIdx++).toInt(-1);
                    QString   caseListFile = cvfqt::Utils::toQString(caseOption.safeValue(optionIdx++));

                    if (caseId != -1 && !caseListFile.isEmpty())
                    {
                        caseIds.push_back(caseId);
                        caseListFiles.push_back(caseListFile);
                    }
                }
            }

            if (caseIds.empty() && !caseListFiles.empty())
            {
                QString              caseListFile = caseListFiles[0];
                std::vector<QString> caseFiles    = RiaApplication::readFileListFromTextFile(caseListFile);
                for (const QString& caseFile : caseFiles)
                {
                    RiaProjectModifier projectModifier;
                    projectModifier.setReplaceCaseFirstOccurrence(caseFile);
                    RiaApplication::instance()->loadProject(projectFileName, RiaApplication::PLA_NONE, &projectModifier);
                    executeCommandFile(commandFile);
                }
            }
            else
            {
                CVF_ASSERT(caseIds.size() == caseListFiles.size());

                std::vector<std::vector<QString>> allCaseFiles;
                size_t                            maxFiles = 0;

                for (size_t i = 0; i < caseIds.size(); ++i)
                {
                    std::vector<QString> caseFiles = RiaApplication::readFileListFromTextFile(caseListFiles[i]);
                    allCaseFiles.push_back(caseFiles);
                    maxFiles = std::max(caseFiles.size(), maxFiles);
                }

                for (size_t i = 0; i < caseIds.size(); ++i)
                {
                    RiaProjectModifier projectModifier;
                    for (size_t j = 0; j < maxFiles; ++j)
                    {
                        if (allCaseFiles[i].size() > j)
                        {
                            projectModifier.setReplaceCase(caseIds[i], allCaseFiles[i][j]);
                        }
                    }

                    RiaApplication::instance()->loadProject(projectFileName, RiaApplication::PLA_NONE, &projectModifier);
                    executeCommandFile(commandFile);
                }
            }
        }
        else
        {
            executeCommandFile(commandFile);
        }
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaArgumentParser::executeCommandFile(const QString& commandFile)
{
    QFile        file(commandFile);
    RicfMessages messages;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // TODO : Error logging?
        return;
    }

    QTextStream in(&file);
    RicfCommandFileExecutor::instance()->executeCommands(in);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaArgumentParser::openCaseFromCommandLineParameter(const QString& parameter)
{
    if (RiaEclipseFileNameTools::isProjectFile(parameter))
    {
        return RiaApplication::instance()->loadProject(parameter);
    }

    QStringList gridFileNames;
    QStringList summarySpecFileNames;

    if (RiaEclipseFileNameTools::isGridFile(parameter))
    {
        QFileInfo fi(parameter);

        gridFileNames.push_back(fi.absoluteFilePath());
    }
    else if (RiaEclipseFileNameTools::isSummarySpecFile(parameter))
    {
        QFileInfo fi(parameter);

        summarySpecFileNames.push_back(fi.absoluteFilePath());
    }
    else
    {
        RiaEclipseFileNameTools fileNameTools(parameter);

        {
            QString gridFileName = fileNameTools.findRelatedGridFile();
            if (!gridFileName.isEmpty())
            {
                gridFileNames.push_back(gridFileName);
            }
        }

        QString summarySpecFileName = fileNameTools.findRelatedSummarySpecFile();
        if (!summarySpecFileName.isEmpty())
        {
            summarySpecFileNames.push_back(summarySpecFileName);
        }
    }

    bool openCaseResult = true;

    // Open summary cases first. Then, the open of grid file will not open an already open summary case file
    if (!summarySpecFileNames.empty())
    {
        openCaseResult &= RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles(summarySpecFileNames);
        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }

    for (const auto& f : gridFileNames)
    {
        openCaseResult &= RiaImportEclipseCaseTools::openEclipseCasesFromFile(QStringList({f}), nullptr, true);
    }

    return openCaseResult;
}
