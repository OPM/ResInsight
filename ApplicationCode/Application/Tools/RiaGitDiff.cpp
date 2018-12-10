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

#include "RiaGitDiff.h"

#include "cafUtils.h"

#include <QDir>
#include <QProcess>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGitDiff::RiaGitDiff(const QString& pathToDiffTool)
    : m_pathToGitTool(pathToDiffTool)
{
    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGitDiff::~RiaGitDiff() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGitDiff::reset()
{
    m_lastError = IC_NO_ERROR;
    m_errorMsg.clear();
    m_errorDetails.clear();
    m_diffOutput.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGitDiff::executeDiff(const QString& baseFolder)
{
    reset();

    QString fullFilePath = "git";
    if (!m_pathToGitTool.isEmpty())
    {
        fullFilePath = m_pathToGitTool + "/" + fullFilePath;
    }

    QString incomingCurrentPath = QDir::currentPath();
    QDir::setCurrent(baseFolder);

    QString args = "diff";

    QString completeCommand = QString("\"%1\" %2").arg(fullFilePath).arg(args);

    // Launch process and wait
    QProcess proc;
    proc.start(completeCommand);
    proc.waitForFinished(30000);

    QDir::setCurrent(incomingCurrentPath);

    QProcess::ProcessError procError = proc.error();
    if (procError != QProcess::UnknownError)
    {
        m_lastError    = SEVERE_ERROR;
        m_errorMsg     = "Error running 'git' tool process";
        m_errorDetails = completeCommand;
        return false;
    }

    QByteArray stdErr = proc.readAllStandardError();
    QByteArray stdOut = proc.readAllStandardOutput();
    m_diffOutput      = stdOut;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGitDiff::ErrorType RiaGitDiff::error() const
{
    return m_lastError;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaGitDiff::errorMessage() const
{
    return m_errorMsg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaGitDiff::errorDetails() const
{
    return m_errorDetails;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaGitDiff::diffOutput() const
{
    return m_diffOutput;
}
