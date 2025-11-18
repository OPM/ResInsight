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

#include "RiaVariableMapper.h"
#include "RiaTextStringTools.h"

#include <QRegularExpression>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaVariableMapper::RiaVariableMapper( const QString& variableNameValueTable )
{
    m_maxUsedIdNumber = 0;
    QStringList lines = RiaTextStringTools::splitSkipEmptyParts( variableNameValueTable, ";" );

    QRegularExpression realizationRe = QRegularExpression( QString( "[%1]\\w*[%2]" ).arg( variableToken() ).arg( variableToken() ) );

    for ( const QString& line : lines )
    {
        QRegularExpressionMatch match = realizationRe.match( line );
        if ( match.hasMatch() )
        {
            QString variableName         = match.captured();
            QString variableNameStripped = variableName;
            variableNameStripped.replace( variableToken(), "" );

            QString variableValue = line.right( line.size() - match.capturedEnd() ).trimmed();

            // Check if we have a standard id, and store the max number

            if ( variableNameStripped.startsWith( pathIdBaseString() ) )
            {
                bool    isOk       = false;
                QString numberText = variableNameStripped.right( variableNameStripped.size() - pathIdBaseString().size() );
                size_t  idNumber   = numberText.toUInt( &isOk );

                if ( isOk )
                {
                    m_maxUsedIdNumber = std::max( m_maxUsedIdNumber, idNumber );
                }
            }

            m_variableToValueMap[variableName]  = variableValue;
            m_valueToVariableMap[variableValue] = variableName;
        }
    }

    resolveVariablesUsedInValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaVariableMapper::addPathAndGetId( const QString& path )
{
    auto computePathId = [this]( const auto& trimmedPath ) -> QString
    {
        QString pathId;
        auto    pathToIdIt = m_valueToVariableMap.find( trimmedPath );
        if ( pathToIdIt != m_valueToVariableMap.end() )
        {
            pathId = pathToIdIt->second;
        }
        else
        {
            auto pathPathIdPairIt = m_pathToPathIdMap.find( trimmedPath );
            if ( pathPathIdPairIt != m_pathToPathIdMap.end() )
            {
                pathId = pathPathIdPairIt->second;
            }
            else
            {
                pathId = createUnusedId();
            }
        }
        return pathId;
    };

    // Want to reuse ids from last save to avoid unnecessary changes and make the behavior predictable
    QString trimmedPath = path.trimmed();
    QString pathId      = computePathId( trimmedPath );

    addVariable( pathId, trimmedPath );

    m_pathToPathIdMap[trimmedPath] = pathId;

    return pathId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaVariableMapper::variableTableAsText() const
{
    std::vector<std::pair<QString, QString>> sortedVariables;
    for ( const auto& it : m_newVariableToValueMap )
    {
        sortedVariables.emplace_back( it );
    }

    const QString pathText = variableToken() + pathIdBaseString();

    // Put path variables at the end of the list
    std::sort( sortedVariables.begin(),
               sortedVariables.end(),
               [pathText]( const auto& lhs, const auto& rhs )
               {
                   bool isLhsPath = lhs.first.startsWith( pathText );
                   bool isRhsPath = rhs.first.startsWith( pathText );

                   if ( isLhsPath && !isRhsPath ) return false;
                   if ( !isLhsPath && isRhsPath ) return true;

                   return lhs.first < rhs.first;
               } );

    QString textTable;
    textTable += "\n";

    for ( const auto& variableNameValuePair : sortedVariables )
    {
        textTable += "        " + variableNameValuePair.first + " " + variableNameValuePair.second + ";\n";
    }

    textTable += "    ";

    return textTable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaVariableMapper::addVariable( const QString& variableName, const QString& variableValue )
{
    m_newVariableToValueMap[variableName] = variableValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaVariableMapper::valueForVariable( const QString& variableName, bool* isFound ) const
{
    auto it = m_variableToValueMap.find( variableName );
    if ( it != m_variableToValueMap.end() )
    {
        if ( isFound ) *isFound = true;
        return it->second;
    }

    if ( isFound ) *isFound = false;
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaVariableMapper::createUnusedId()
{
    m_maxUsedIdNumber++;

    QString numberString   = QString( "%1" ).arg( (uint)m_maxUsedIdNumber, 3, 10, QChar( '0' ) );
    QString pathIdentifier = variableToken() + pathIdBaseString() + numberString + variableToken();

    return pathIdentifier;
}

//--------------------------------------------------------------------------------------------------
// Replace all variables with text in variable values
// $MyPath$ = /path/to/my/file
// $MyFile$ = $MyPath$/myFile.txt  => $MyFile$ = /path/to/my/file/myFile.txt
//--------------------------------------------------------------------------------------------------
void RiaVariableMapper::resolveVariablesUsedInValues()
{
    for ( auto& variableValuePair : m_variableToValueMap )
    {
        QString variableValue = variableValuePair.second;

        for ( const auto& [otherVariableName, otherVariableValue] : m_variableToValueMap )
        {
            variableValue.replace( otherVariableName, otherVariableValue );
        }

        variableValuePair.second = variableValue;
    }
}

//--------------------------------------------------------------------------------------------------
// Replace text in variable values with variables
// $MyPath$ = /path/to/my/file
// $PathId_001$ = $MyPath$/myFile.txt  => $PathId_001$ = $MyPath$/myFile.txt
//--------------------------------------------------------------------------------------------------
void RiaVariableMapper::replaceVariablesInValues()
{
    // Move all non-path variables from the original map imported from the project file to the new map
    for ( auto& [variableName, variableValue] : m_variableToValueMap )
    {
        if ( !variableName.startsWith( variableToken() + pathIdBaseString() ) )
        {
            m_newVariableToValueMap[variableName] = variableValue;
        }
    }

    // Replace text with variables
    for ( auto& [variableName, variableValue] : m_newVariableToValueMap )
    {
        for ( const auto& [otherVariableName, otherVariableValue] : m_newVariableToValueMap )
        {
            if ( otherVariableName != variableName && !otherVariableName.contains( postfixName() ) )
            {
                variableValue.replace( otherVariableValue, otherVariableName );
            }
        }
    }
}
