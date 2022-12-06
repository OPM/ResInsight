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

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VariableNameValueMapper::VariableNameValueMapper( const QString& globalPathListTable )
{
    m_maxUsedIdNumber     = 0;
    QStringList pathPairs = RiaTextStringTools::splitSkipEmptyParts( globalPathListTable, ";" );

    for ( const QString& pathIdPathPair : pathPairs )
    {
        QStringList pathIdPathComponents = pathIdPathPair.trimmed().split( variableToken() );

        if ( pathIdPathComponents.size() == 3 && pathIdPathComponents[0].size() == 0 )
        {
            QString pathIdCore = pathIdPathComponents[1];
            QString pathId     = variableToken() + pathIdCore + variableToken();
            QString path       = pathIdPathComponents[2].trimmed();

            // Check if we have a standard id, and store the max number

            if ( pathIdCore.startsWith( pathIdBaseString ) )
            {
                bool    isOk       = false;
                QString numberText = pathIdCore.right( pathIdCore.size() - pathIdBaseString.size() );
                size_t  idNumber   = numberText.toUInt( &isOk );

                if ( isOk )
                {
                    m_maxUsedIdNumber = std::max( m_maxUsedIdNumber, idNumber );
                }
            }

            m_variableToValueMap[pathId] = path;
            m_valueToVariableMap[path]   = pathId;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VariableNameValueMapper::addPathAndGetId( const QString& path )
{
    // Want to re-use ids from last save to avoid unnecessary changes and make the behavior predictable
    QString pathId;
    QString trimmedPath = path.trimmed();

    auto pathToIdIt = m_valueToVariableMap.find( trimmedPath );
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

    addVariable( pathId, trimmedPath );

    m_pathToPathIdMap[trimmedPath] = pathId;

    return pathId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VariableNameValueMapper::variableTableAsText() const
{
    QString textTable;
    textTable += "\n";
    for ( const auto& variableNameValuePair : m_newVariableToValueMap )
    {
        textTable += "        " + variableNameValuePair.first + " " + variableNameValuePair.second + ";\n";
    }

    textTable += "    ";

    return textTable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VariableNameValueMapper::addVariable( const QString& variableName, const QString& variableValue )
{
    m_newVariableToValueMap[variableName] = variableValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VariableNameValueMapper::valueForVariable( const QString& variableName, bool* isFound ) const
{
    auto it = m_variableToValueMap.find( variableName );
    if ( it != m_variableToValueMap.end() )
    {
        ( *isFound ) = true;
        return it->second;
    }

    ( *isFound ) = false;
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VariableNameValueMapper::createUnusedId()
{
    m_maxUsedIdNumber++;

    QString numberString   = QString( "%1" ).arg( (uint)m_maxUsedIdNumber, 3, 10, QChar( '0' ) );
    QString pathIdentifier = variableToken() + pathIdBaseString + numberString + variableToken();

    return pathIdentifier;
}
