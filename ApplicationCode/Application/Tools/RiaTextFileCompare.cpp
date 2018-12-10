/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RiaTextFileCompare.h"

#include "cafUtils.h"

#include <QProcess>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaTextFileCompare::RiaTextFileCompare(const QString& pathToDiffTool)
    : m_pathToDiffTool(pathToDiffTool)
{
    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaTextFileCompare::~RiaTextFileCompare() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaTextFileCompare::reset()
{
    m_lastError = IC_NO_ERROR;
    m_errorMsg.clear();
    m_errorDetails.clear();
    m_diffOutput.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaTextFileCompare::runComparison(const QString& baseFolder, const QString& generatedFolder)
{
    reset();

    QString fullFilePath = "diff";
    if (!m_pathToDiffTool.isEmpty())
    {
        fullFilePath = m_pathToDiffTool + "/" + fullFilePath;
    }

    // Command line arguments used when invoking 'diff'
    // See https://docs.freebsd.org/info/diff/diff.info.diff_Options.html
    QString args = "-r -u --strip-trailing-cr";

    QString completeCommand = QString("\"%1\" %2 %3 %4").arg(fullFilePath).arg(baseFolder).arg(generatedFolder).arg(args);

    // Launch process and wait
    QProcess proc;
    proc.start(completeCommand);
    proc.waitForFinished(30000);

    QProcess::ProcessError procError = proc.error();
    if (procError != QProcess::UnknownError)
    {
        m_lastError    = SEVERE_ERROR;
        m_errorMsg     = "Error running 'diff' tool process";
        m_errorDetails = completeCommand;
        return false;
    }

    QByteArray stdErr       = proc.readAllStandardError();
    int        procExitCode = proc.exitCode();

    if (procExitCode == 0)
    {
        return true;
    }
    else if (procExitCode == 1)
    {
        QByteArray stdOut = proc.readAllStandardOutput();
        m_diffOutput      = stdOut;

        return false;
    }
    else
    {
        stdErr = stdErr.simplified();

        // Report non-severe error
        m_lastError    = IC_ERROR;
        m_errorMsg     = "Error running 'diff' tool process";
        m_errorDetails = stdErr;

        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaTextFileCompare::ErrorType RiaTextFileCompare::error() const
{
    return m_lastError;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextFileCompare::errorMessage() const
{
    return m_errorMsg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextFileCompare::errorDetails() const
{
    return m_errorDetails;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextFileCompare::diffOutput() const
{
    return m_diffOutput;
}
