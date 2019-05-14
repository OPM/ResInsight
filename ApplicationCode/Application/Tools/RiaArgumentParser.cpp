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
bool RiaArgumentParser::parseArguments(cvf::ProgramOptions* progOpt)
{
    CVF_ASSERT(progOpt);
    progOpt->registerOption("grpcserver", "[<portnumber>]", "Run as a GRPC server. Default port is 50051", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("console", "", "Run as a console application without Graphics");
    progOpt->registerOption("last", "", "Open last used project.");
    progOpt->registerOption("project", "<filename>", "Open project file <filename>.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("case",
                           "<case name without extension or filename>",
                           "If case or grid filename, import simulation grid data. If summary file name, import summary data",
                           cvf::ProgramOptions::MULTI_VALUE);
    progOpt->registerOption("startdir", "<folder>", "Set startup directory.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("savesnapshots",
                           "all|views|plots",
                           "Save snapshot of all views or plots to project file location sub folder 'snapshots'. Option 'all' "
                           "will include both views and plots. Application closes after snapshots have been written.",
                           cvf::ProgramOptions::OPTIONAL_MULTI_VALUE);
    progOpt->registerOption(
        "size", "<width> <height>", "Set size of the main application window.", cvf::ProgramOptions::MULTI_VALUE);
    progOpt->registerOption(
        "replaceCase",
        "[<caseId>] <newGridFile>",
        "Replace grid in <caseId> or first case with <newgridFile>. Repeat parameter for multiple replace operations.",
        cvf::ProgramOptions::MULTI_VALUE,
        cvf::ProgramOptions::COMBINE_REPEATED);
    progOpt->registerOption("replaceSourceCases",
                           "[<caseGroupId>] <gridListFile>",
                           "Replace source cases in <caseGroupId> or first grid case group with the grid files listed in the "
                           "<gridListFile> file. Repeat parameter for multiple replace operations.",
                           cvf::ProgramOptions::MULTI_VALUE,
                           cvf::ProgramOptions::COMBINE_REPEATED);
    progOpt->registerOption("replacePropertiesFolder",
                           "[<caseId>] <newPropertiesFolder>",
                           "Replace the folder containing property files for an eclipse input case.",
                           cvf::ProgramOptions::MULTI_VALUE);
    progOpt->registerOption("multiCaseSnapshots",
                           "<gridListFile>",
                           "For each grid file listed in the <gridListFile> file, replace the first case in the project and save "
                           "snapshot of all views.",
                           cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("commandFile", "<commandfile>", "Execute the command file.", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption(
        "commandFileProject",
        "<filename>",
        "Project to use if performing case looping for command file. Used in conjunction with 'commandFileReplaceCases'.",
        cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("commandFileReplaceCases",
                           "[<caseId>] <caseListFile>",
                           "Supply list of cases to replace in project, performing command file for each case.",
                           cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("help", "", "Displays help text.");
    progOpt->registerOption("?", "", "Displays help text.");
    progOpt->registerOption("regressiontest", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE);
    progOpt->registerOption("updateregressiontestbase", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE);
#ifdef USE_UNIT_TESTS
    progOpt->registerOption("unittest", "", "System command");
#endif
    progOpt->registerOption("ignoreArgs", "", "Ignore all arguments. Mostly for testing purposes");

    progOpt->setOptionPrefix(cvf::ProgramOptions::DOUBLE_DASH);

    QStringList arguments = QCoreApplication::arguments();
 
    bool parseOk = progOpt->parse(cvfqt::Utils::toStringVector(arguments));

    // If positional parameter functionality is to be supported, the test for existence of positionalParameters must be removed
    // This is based on a pull request by @andlaus https://github.com/OPM/ResInsight/pull/162
    if (!parseOk || progOpt->hasOption("help") || progOpt->hasOption("?") || !progOpt->positionalParameters().empty())
    {
        return false;
    }
    return true;
}
