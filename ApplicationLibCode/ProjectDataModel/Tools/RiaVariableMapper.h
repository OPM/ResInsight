/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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
#include <map>

class RiaVariableMapper
{
public:
    static QString variableToken() { return "$"; }
    static QString pathIdBaseString() { return "PathId_"; }
    static QString postfixName() { return "_name"; }

public:
    RiaVariableMapper( const QString& variableNameValueTable );

    QString addPathAndGetId( const QString& path );

    void    addVariable( const QString& variableName, const QString& variableValue );
    QString valueForVariable( const QString& variableName, bool* isFound ) const;

    void    replaceVariablesInValues();
    QString variableTableAsText() const;

private:
    QString createUnusedId();
    void    resolveVariablesUsedInValues();

private:
    size_t m_maxUsedIdNumber; // Set when parsing the globalPathListTable. Increment while creating new id's

    std::map<QString, QString> m_newVariableToValueMap;
    std::map<QString, QString> m_pathToPathIdMap;

    std::map<QString, QString> m_variableToValueMap;
    std::map<QString, QString> m_valueToVariableMap;
};
