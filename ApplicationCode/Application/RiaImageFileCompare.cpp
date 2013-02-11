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

#include "RiaImageFileCompare.h"
#include <QtCore/QProcess>


//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaImageFileCompare::RiaImageFileCompare(QString compareExecutable)
:   m_compareExecutable(compareExecutable)
{
    reset();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaImageFileCompare::~RiaImageFileCompare()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaImageFileCompare::reset()
{
    m_imagesEqual = false;
    m_lastError = IC_NO_ERROR;
    m_errorMsg = "";
    m_errorDetails = "";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImageFileCompare::runComparison(QString imgFileName, QString refFileName, QString diffFileName)
{
    reset();

    if (m_compareExecutable.isEmpty())
    {
        m_lastError = SEVERE_ERROR;
        m_errorMsg = "Cannot compare images, no compare executable set";
        return false;
    }


    //QString args = QString("-fuzz 2% -lowlight-color white -metric ae \"%1\" \"%2\" \"%3\"").arg(imgFileName).arg(refFileName).arg((diffFileName));
    QString args = QString("-lowlight-color white -metric ae \"%1\" \"%2\" \"%3\"").arg(imgFileName).arg(refFileName).arg((diffFileName));
    QString completeCommand = QString("\"%1\" %2").arg(m_compareExecutable).arg(args);

    // Launch process and wait
    QProcess proc;
    proc.start(completeCommand);
    proc.waitForFinished(30000);

    QProcess::ProcessError procError = proc.error();
    if (procError != QProcess::UnknownError)
    {
        m_lastError = SEVERE_ERROR;
        m_errorMsg = "Error running compare tool process";
        m_errorDetails = completeCommand;
        return false;
    }

    QByteArray stdErr = proc.readAllStandardError();
    int procExitCode = proc.exitCode();
    if (procExitCode == 0)
    {
        // Strip out whitespace and look for 0 (as in zero pixel differences)
        stdErr = stdErr.simplified();
        if (!stdErr.isEmpty() && stdErr[0] == '0')
        {
            m_imagesEqual = true;
        }

        return true;
    }
    else
    {
        // Report non-severe error
        m_lastError = IC_ERROR;
        m_errorMsg = "Error running compare tool process";
        m_errorDetails = stdErr;

        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImageFileCompare::imagesEqual() const
{
    return m_imagesEqual;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaImageFileCompare::ErrorType RiaImageFileCompare::error() const
{
    return m_lastError;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaImageFileCompare::errorMessage() const
{
    return m_errorMsg;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaImageFileCompare::errorDetails() const
{
    return m_errorDetails;
}
