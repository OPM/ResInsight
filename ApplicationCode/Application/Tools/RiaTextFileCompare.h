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

#pragma once

#include <QString>

//==================================================================================================
//
// Execute text compare between to folders recursively using the external tool 'diff'
//
//==================================================================================================
class RiaTextFileCompare
{
public:
    enum ErrorType
    {
        IC_NO_ERROR, // No error occurred
        IC_ERROR, // An error occurred
        SEVERE_ERROR // Severe error occurred, it is likely that another call to compare() will also fail
    };

public:
    explicit RiaTextFileCompare(const QString& pathToDiffTool);
    ~RiaTextFileCompare();

    bool      runComparison(const QString& baseFolder, const QString& generatedFolder);
    ErrorType error() const;
    QString   errorMessage() const;
    QString   errorDetails() const;
    QString   diffOutput() const;

private:
    void reset();

private:
    const QString m_pathToDiffTool;
    ErrorType     m_lastError;
    QString       m_errorMsg;
    QString       m_errorDetails;
    QString       m_diffOutput;
};
