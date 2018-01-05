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

#pragma once

#include <QString>

//==================================================================================================
//
// Compare two images (currently using ImageMagick in an external process)
//
//==================================================================================================
class RiaImageFileCompare
{
public:
    enum ErrorType
    {
        IC_NO_ERROR,       // No error occurred
        IC_ERROR,          // An error occurred
        SEVERE_ERROR    // Severe error occurred, it is likely that another call to compare() will also fail
    };

public:
    explicit RiaImageFileCompare(QString compareExecutable);
    ~RiaImageFileCompare();

    bool        runComparison(QString imgFileName, QString refFileName, QString diffFileName);
    bool        imagesEqual() const;
    ErrorType   error() const;
    QString     errorMessage() const;
    QString     errorDetails() const;

private:
    void        reset();

private:
    QString                 m_compareExecutable;    // The executable for the ImageMagick compare tool
    bool                    m_imagesEqual;          // Result of last comparison
    ErrorType               m_lastError;            // Error for last execution
    QString                 m_errorMsg;
    QString                 m_errorDetails;
};



