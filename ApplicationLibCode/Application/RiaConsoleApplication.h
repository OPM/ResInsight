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
#pragma once

#include "RiaApplication.h"

#include <QCoreApplication>

#include <QPointer>
#include <QTimer>

namespace cvf
{
class ProgramOptions;
}

class RiaConsoleApplication : public QCoreApplication, public RiaApplication
{
    Q_OBJECT

public:
    static RiaConsoleApplication* instance();

    RiaConsoleApplication( int& argc, char** argv );
    ~RiaConsoleApplication() override;

    // Public RiaApplication overrides
    void              initialize() override;
    ApplicationStatus handleArguments( gsl::not_null<cvf::ProgramOptions*> progOpt ) override;
    void              showFormattedTextInMessageBoxOrConsole( const QString& errMsg ) override;

protected:
    // Protected implementation specific overrides
    void invokeProcessEvents( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents ) override;
    void onProjectOpeningError( const QString& errMsg ) override;
    void onProjectOpened() override;
    void onProjectClosed() override;
};
