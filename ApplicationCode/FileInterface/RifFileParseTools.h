/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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
#include <QStringList>
#include <QRegExp>

//==================================================================================================
///
//==================================================================================================
class RifFileParseTools
{
public:
    static QStringList  splitLineAndTrim(const QString& line, const QString& separator, bool skipEmptyParts = false);
    static QStringList  splitLineAndTrim(const QString& line, const QRegExp& regexp, bool skipEmptyParts = false);
};

//==================================================================================================
///
//==================================================================================================
class FileParseException
{
public:
    FileParseException(const QString &message) : message(message) {}
    QString  message;
};
