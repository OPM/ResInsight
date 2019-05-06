/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "RiaConsoleApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

RiaApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (!qstrcmp(argv[i], "--console") || !qstrcmp(argv[i], "--unittest"))
        {
            return new RiaConsoleApplication(argc, argv);
        }
    }
    return new RiaGuiApplication(argc, argv);
}

int main(int argc, char *argv[])
{
    RiaLogging::loggerInstance()->setLevel(RI_LL_DEBUG);

    std::unique_ptr<RiaApplication> app (createApplication(argc, argv));
    app->initialize();

    cvf::ProgramOptions progOpt;

    bool result    = RiaArgumentParser::parseArguments(&progOpt);

    if (!result)
    {
        std::vector<cvf::String> unknownOptions = progOpt.unknownOptions();
        QStringList unknownOptionsText;
        for (cvf::String option : unknownOptions)
        {
            unknownOptionsText += QString("Unknown option: %1").arg(cvfqt::Utils::toQString(option));
        }

        const cvf::String usageText = progOpt.usageText(110, 30);
        app->showErrorMessage(RiaApplication::commandLineParameterHelp() +
                              cvfqt::Utils::toQString(usageText) +
                              unknownOptionsText.join("\n"));
        app->cleanupBeforeProgramExit();
        if (dynamic_cast<RiaGuiApplication*>(app.get()) == nullptr)
        {
            return 1;
        }
    }
  
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    setlocale(LC_NUMERIC,"C");

    RiaApplication::ApplicationStatus status = app->handleArguments(&progOpt);
    if (status == RiaApplication::EXIT_COMPLETED)
    {
        return 0;
    }
    else if (status == RiaApplication::EXIT_WITH_ERROR)
    {
        return 2;
    }
    else if (status == RiaApplication::KEEP_GOING)
    {
        int exitCode = 0;
        try
        {
            exitCode = QCoreApplication::instance()->exec();
        }
        catch (std::exception& exep )
        {
            std::cout << "A standard c++ exception that terminated ResInsight caught in RiaMain.cpp: " << exep.what() << std::endl;
            throw;
        }
        catch(...)
        {
            std::cout << "An unknown exception that terminated ResInsight caught in RiaMain.cpp.  " << std::endl;
            throw;
        }

        return exitCode;
    }

    CVF_ASSERT(false && "Unknown ApplicationStatus");
    return -1;
}

