/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimCaseDisplayNameTools.h"

#include <QString>

#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RimCaseDisplayNameTools::DisplayName>::setUp()
{
    addItem( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME, "FULL_CASE_NAME", "Full Case Name" );
    addItem( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME, "SHORT_CASE_NAME", "Shortened Case Name" );
    addItem( RimCaseDisplayNameTools::DisplayName::CUSTOM, "CUSTOM_NAME", "Custom Name" );
    setDefault( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCaseDisplayNameTools::uniqueShortName( const QString&           caseName,
                                                  const std::set<QString>& allAutoShortNames,
                                                  int                      shortNameLengthLimit )
{
    bool foundUnique = false;

    QString shortName;

    if ( caseName.size() > shortNameLengthLimit )
    {
        QString candidate;
        candidate += caseName[0];

        for ( int i = 1; i < caseName.size(); ++i )
        {
            if ( foundUnique && !caseName[i].isLetterOrNumber() )
            {
                break;
            }

            candidate += caseName[i];
            if ( allAutoShortNames.count( candidate ) == 0 )
            {
                shortName   = candidate;
                foundUnique = true;
                if ( shortName.length() >= shortNameLengthLimit )
                {
                    break;
                }
            }
        }
    }
    else
    {
        shortName = caseName.left( shortNameLengthLimit );
        if ( allAutoShortNames.count( shortName ) == 0 )
        {
            foundUnique = true;
        }
    }

    int autoNumber = 1;

    while ( !foundUnique )
    {
        QString candidate = QString( "%1 %2" ).arg( shortName ).arg( autoNumber++ );
        if ( allAutoShortNames.count( candidate ) == 0 )
        {
            shortName   = candidate;
            foundUnique = true;
        }
    }

    return shortName;
}
