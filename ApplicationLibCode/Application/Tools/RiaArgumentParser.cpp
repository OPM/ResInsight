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

#include "RiaBaseDefs.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"
#include "RiaRegressionTestRunner.h"

#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "RicfCommandFileExecutor.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "RicImportSummaryCasesFeature.h"

#include "cafPdmScriptIOMessages.h"
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
bool RiaArgumentParser::parseArguments( cvf::ProgramOptions* progOpt )
{
    CVF_ASSERT( progOpt );
    progOpt->registerOption( "help", "", "Displays help text and exits." );
    progOpt->registerOption( "?", "", "Displays help text and exits." );

    progOpt->registerOption( "project", "<filename>", "Open project file <filename>.", cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "last", "", "Open last used project." );
    progOpt->registerOption( "case",
                             "<casename|filename> [<casename|filename> ...]",
                             "Imports the Eclipse cases specified by case name with or without extension."
                             "If <casename>, import the corresponding grid file and summary file"
                             "If <filename> has extension .GRRID/.EGRID, import the grid file and corresponding "
                             "summary file"
                             "If <filename> has extension .SMSPEC, import the summary file (does not open the "
                             "corresponding grid file)",
                             cvf::ProgramOptions::MULTI_VALUE );
    progOpt->registerOption( "size",
                             "<width> <height>",
                             "Set size of the main application window.",
                             cvf::ProgramOptions::MULTI_VALUE );
    progOpt->registerOption( "console", "", "Launch as a console application without graphics" );
    progOpt->registerOption( "server",
                             "[<portnumber>]",
                             "Launch as a GRPC server. Default port is 50051",
                             cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "startdir", "<folder>", "Set startup directory.\n", cvf::ProgramOptions::SINGLE_VALUE );

    progOpt->registerOption( "summaryplot",
                             "[<plotOptions>] <eclipsesummaryvectors> <eclipsedatafiles>",
                             "Creates a summary plot using all the <eclipsedatafiles>,"
                             "and all the summary vectors defined in <eclipsesummaryvectors>."
                             "Use --summaryplot -help to show a more detailed help text.\n",
                             cvf::ProgramOptions::OPTIONAL_MULTI_VALUE );

    progOpt->registerOption( "commandFile", "<commandfile>", "Execute the command file.", cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "commandFileReplaceCases",
                             "[<caseId>] <caseListFile>",
                             "Supply list of cases to replace in project, performing command file for each case.",
                             cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "commandFileProject",
                             "<filename>",
                             "Project to use if performing case looping for command file. Used in conjunction with "
                             "'commandFileReplaceCases'.\n",
                             cvf::ProgramOptions::SINGLE_VALUE );

    progOpt->registerOption( "snapshotsize",
                             "<width> <height>",
                             "Set size of exported snapshot images.",
                             cvf::ProgramOptions::MULTI_VALUE );
    progOpt->registerOption( "snapshotfolder",
                             "<folder>",
                             "Set the destination folder for exported snapshot images.\n",
                             cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "savesnapshots",
                             "all|views|plots",
                             "Save snapshot of all views or plots to project file location sub folder 'snapshots'. "
                             "Option 'all' "
                             "will include both views and plots. Application closes after snapshots have been written.",
                             cvf::ProgramOptions::OPTIONAL_MULTI_VALUE );
    progOpt->registerOption( "multiCaseSnapshots",
                             "<gridListFile>",
                             "For each grid file listed in the <gridListFile> file, replace the first case in the "
                             "project and save "
                             "snapshots of all views.\n",
                             cvf::ProgramOptions::SINGLE_VALUE );

    progOpt->registerOption( "replaceCase",
                             "[<caseId>] <newGridFile>",
                             "Replace grid in <caseId> or first case with <newgridFile>. Repeat parameter for multiple "
                             "replace operations.",
                             cvf::ProgramOptions::MULTI_VALUE,
                             cvf::ProgramOptions::COMBINE_REPEATED );
    progOpt->registerOption( "replaceSourceCases",
                             "[<caseGroupId>] <gridListFile>",
                             "Replace source cases in <caseGroupId> or first grid case group with the grid files "
                             "listed in the "
                             "<gridListFile> file. Repeat parameter for multiple replace operations.",
                             cvf::ProgramOptions::MULTI_VALUE,
                             cvf::ProgramOptions::COMBINE_REPEATED );
    progOpt->registerOption( "replacePropertiesFolder",
                             "[<caseId>] <newPropertiesFolder>",
                             "Replace the folder containing property files for an eclipse input case.\n",
                             cvf::ProgramOptions::MULTI_VALUE );

    progOpt->registerOption( "updateregressiontestbase", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "regressiontest", "<folder>", "System command", cvf::ProgramOptions::SINGLE_VALUE );
#ifdef USE_UNIT_TESTS
    progOpt->registerOption( "unittest", "", "System command" );
#endif
    progOpt->registerOption( "generate", "[<outputFile>]", "Generate code or documentation", cvf::ProgramOptions::SINGLE_VALUE );
    progOpt->registerOption( "ignoreArgs", "", "System command. Ignore all arguments. Mostly for testing purposes" );

    progOpt->setOptionPrefix( cvf::ProgramOptions::DOUBLE_DASH );

    QStringList arguments = QCoreApplication::arguments();

    bool parseOk = progOpt->parse( cvfqt::Utils::toStringVector( arguments ) );

    // If positional parameter functionality is to be supported, the test for existence of positionalParameters must be
    // removed This is based on a pull request by @andlaus https://github.com/OPM/ResInsight/pull/162
    if ( !parseOk || !progOpt->positionalParameters().empty() )
    {
        return false;
    }
    return true;
}
