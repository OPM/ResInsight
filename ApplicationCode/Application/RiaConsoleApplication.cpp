////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiaConsoleApplication.h"

#include "RiaArgumentParser.h"
#ifdef ENABLE_GRPC
#include "RiaGrpcServer.h"
#endif
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaProjectModifier.h"
#include "RiaSocketServer.h"

#include "RicImportGeneralDataFeature.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QFileInfo>

#ifdef WIN32
#include <windows.h>
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaConsoleApplication* RiaConsoleApplication::instance()
{
    RiaConsoleApplication* currentConsoleApp = dynamic_cast<RiaConsoleApplication*>(RiaApplication::instance());
    CAF_ASSERT(currentConsoleApp && "Should never be called from a method that isn't within the Console context");
    return currentConsoleApp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaConsoleApplication::RiaConsoleApplication(int& argc, char** argv)
    : QCoreApplication(argc, argv)
    , RiaApplication()
{
    installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaConsoleApplication::~RiaConsoleApplication()
{
    RiaLogging::deleteLoggerInstance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::initialize()
{
#ifdef _WIN32
#pragma warning(push) // Saves the current warning state.
#pragma warning(disable : 4996) // Temporarily disables warning 4996.
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#pragma warning(pop)
#endif

    RiaApplication::initialize();

    RiaLogging::setLoggerInstance(new RiaStdOutLogger);
    RiaLogging::loggerInstance()->setLevel(RI_LL_DEBUG);

    m_socketServer = new RiaSocketServer(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::ApplicationStatus RiaConsoleApplication::handleArguments(cvf::ProgramOptions* progOpt)
{
    CVF_ASSERT(progOpt);

    // Handling of the actual command line options
    // --------------------------------------------------------
    if (cvf::Option o = progOpt->option("ignoreArgs"))
    {
        return KEEP_GOING;
    }

    // Unit testing
    // --------------------------------------------------------
    if (cvf::Option o = progOpt->option("unittest"))
    {
        int testReturnValue = launchUnitTestsWithConsole();
        if (testReturnValue == 0)
        {
            return RiaApplication::EXIT_COMPLETED;
        }
        else
        {
            RiaLogging::error("Error running unit tests");
            return RiaApplication::EXIT_WITH_ERROR;
        }
    }

    if (cvf::Option o = progOpt->option("startdir"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        setStartDir(cvfqt::Utils::toQString(o.value(0)));
    }

    QString projectFileName;

    if (progOpt->hasOption("last"))
    {
        projectFileName = preferences()->lastUsedProjectFileName;
    }

    if (cvf::Option o = progOpt->option("project"))
    {
        CVF_ASSERT(o.valueCount() == 1);
        projectFileName = cvfqt::Utils::toQString(o.value(0));
    }

    if (!projectFileName.isEmpty())
    {
        cvf::ref<RiaProjectModifier>      projectModifier;
        RiaApplication::ProjectLoadAction projectLoadAction = RiaApplication::PLA_NONE;

        if (cvf::Option o = progOpt->option("replaceCase"))
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

        if (cvf::Option o = progOpt->option("replaceSourceCases"))
        {
            if (projectModifier.isNull()) projectModifier = new RiaProjectModifier;

            if (o.valueCount() == 1)
            {
                // One argument is available, use replace case for first occurrence in the project

                std::vector<QString> gridFileNames = readFileListFromTextFile(cvfqt::Utils::toQString(o.safeValue(0)));
                projectModifier->setReplaceSourceCasesFirstOccurrence(gridFileNames);
            }
            else
            {
                size_t optionIdx = 0;
                while (optionIdx < o.valueCount())
                {
                    const int            groupId = o.safeValue(optionIdx++).toInt(-1);
                    std::vector<QString> gridFileNames =
                        readFileListFromTextFile(cvfqt::Utils::toQString(o.safeValue(optionIdx++)));

                    if (groupId != -1 && !gridFileNames.empty())
                    {
                        projectModifier->setReplaceSourceCasesById(groupId, gridFileNames);
                    }
                }
            }

            projectLoadAction = RiaApplication::PLA_CALCULATE_STATISTICS;
        }

        if (cvf::Option o = progOpt->option("replacePropertiesFolder"))
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

        loadProject(projectFileName, projectLoadAction, projectModifier.p());
    }

    if (cvf::Option o = progOpt->option("case"))
    {
        QStringList                                  caseNames = cvfqt::Utils::toQStringList(o.values());
        RicImportGeneralDataFeature::OpenCaseResults results =
            RicImportGeneralDataFeature::openEclipseFilesFromFileNames(caseNames);
    }

    if (cvf::Option o = progOpt->option("commandFile"))
    {
        QString commandFile = cvfqt::Utils::toQString(o.safeValue(0));
        if (!progOpt->hasOption("startdir"))
        {
            QFileInfo commandFileInfo(commandFile);
            QString commandDir = commandFileInfo.absolutePath();
            setStartDir(commandDir);
        }

        cvf::Option projectOption = progOpt->option("commandFileProject");
        cvf::Option caseOption    = progOpt->option("commandFileReplaceCases");
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
                std::vector<QString> caseFiles    = readFileListFromTextFile(caseListFile);
                for (const QString& caseFile : caseFiles)
                {
                    RiaProjectModifier projectModifier;
                    projectModifier.setReplaceCaseFirstOccurrence(caseFile);
                    loadProject(projectFileName, RiaApplication::PLA_NONE, &projectModifier);
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
                    std::vector<QString> caseFiles = readFileListFromTextFile(caseListFiles[i]);
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

                    loadProject(projectFileName, RiaApplication::PLA_NONE, &projectModifier);
                    executeCommandFile(commandFile);
                }
            }
        }
        else
        {
            executeCommandFile(commandFile);
        }
        return EXIT_COMPLETED;
    }

    if (cvf::Option o = progOpt->option("server"))
    {
#ifdef ENABLE_GRPC
        RiaGrpcServer server;
        server.run();
        return EXIT_COMPLETED;
#else
        std::cout << "ResInsight has not been compiled with GRPC. Cannot use --server option" << std::endl;
        return EXIT_WITH_ERROR;
#endif
    }

    return KEEP_GOING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::showInformationMessage(const QString& text)
{
    RiaLogging::info(text);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::showErrorMessage(const QString& errMsg)
{
    RiaLogging::error(errMsg);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::invokeProcessEvents(QEventLoop::ProcessEventsFlags flags /*= QEventLoop::AllEvents*/)
{
    processEvents(flags);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::onProjectOpeningError(const QString& errMsg)
{
    showErrorMessage(errMsg);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::onProjectOpened()
{
    processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConsoleApplication::onProjectClosed()
{
    processEvents();
}

