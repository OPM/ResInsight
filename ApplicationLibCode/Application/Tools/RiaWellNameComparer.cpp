/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaWellNameComparer.h"

#include "RimProject.h"
#include "RimWellPath.h"

#include <regex>

std::map<QString, QString> RiaWellNameComparer::sm_nameWithoutPrefix;
std::map<QString, QString> RiaWellNameComparer::sm_matchedName;

//==================================================================================================
//
//==================================================================================================
QString RiaWellNameComparer::tryFindMatchingSimWellName( QString searchName )
{
    RimProject*                 proj         = RimProject::current();
    const std::vector<QString>& simWellNames = proj->simulationWellNames();

    if ( searchName.isEmpty() || simWellNames.empty() ) return QString();

    searchName = removeWellNamePrefix( searchName );
    return tryMatchNameInList( searchName, simWellNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryFindMatchingWellPath( QString wellName )
{
    RimProject*                      proj      = RimProject::current();
    const std::vector<RimWellPath*>& wellPaths = proj->allWellPaths();

    if ( wellName.isEmpty() || wellPaths.empty() ) return QString();

    std::vector<QString> wellPathNames;
    for ( const RimWellPath* wellPath : wellPaths )
    {
        wellPathNames.push_back( wellPath->name() );
    }

    wellName = removeWellNamePrefix( wellName );
    return tryMatchNameInList( wellName, wellPathNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryMatchNameInList( QString searchName, const std::vector<QString>& nameList )
{
    if ( sm_matchedName.count( searchName ) > 0 )
    {
        return sm_matchedName[searchName];
    };

    // Try exact name match
    QString matchedName = tryMatchName( searchName, nameList );
    if ( !matchedName.isEmpty() )
    {
        sm_matchedName[searchName] = matchedName;
        return matchedName;
    }

    // Try matching ignoring spaces, dashes and underscores
    matchedName = tryMatchName( searchName, nameList, []( const QString& str ) {
        QString s = str;
        s         = removeWellNamePrefix( s );
        return s.remove( ' ' ).remove( '-' ).remove( '_' );
    } );

    if ( !matchedName.isEmpty() )
    {
        sm_matchedName[searchName] = matchedName;
    }

    return matchedName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWellNameComparer::clearCache()
{
    sm_nameWithoutPrefix.clear();
    sm_matchedName.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryMatchName( QString                           searchName,
                                           const std::vector<QString>&       simWellNames,
                                           std::function<QString( QString )> stringFormatter )
{
    if ( searchName.isEmpty() ) return QString();

    if ( stringFormatter != nullptr )
    {
        searchName = stringFormatter( searchName );
    }

    for ( const auto& simWellName : simWellNames )
    {
        QString simWn = simWellName;
        if ( stringFormatter != nullptr )
        {
            simWn = stringFormatter( simWn );
        }
        if ( QString::compare( simWn, searchName, Qt::CaseInsensitive ) == 0 )
        {
            return simWellName;
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::removeWellNamePrefix( const QString& name )
{
    if ( sm_nameWithoutPrefix.count( name ) > 0 )
    {
        return sm_nameWithoutPrefix[name];
    };

    // Try to remove prefix on the format 'xx xxxx/xx-'
    std::regex pattern( "^.*\\d*[/]\\d*[-_]" );

    auto withoutPrefix  = std::regex_replace( name.toStdString(), pattern, "" );
    auto qWithoutPrefix = QString::fromStdString( withoutPrefix );

    sm_nameWithoutPrefix[name] = qWithoutPrefix;

    return qWithoutPrefix;
}
