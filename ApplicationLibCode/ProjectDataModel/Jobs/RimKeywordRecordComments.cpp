/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimKeywordRecordComments.h"

#include "ReservoirDataModel/CompletionsMsw/RigMswTableData.h"

#include <QStringList>

namespace
{
const QString sectionTitlePrefix = QStringLiteral( "\x1f-title-\x1f" );
} // namespace

namespace RimKeywordRecordComments
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString separator()
{
    return QStringLiteral( "\x1f-separator-\x1f" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString sectionTitle( const QString& title )
{
    return sectionTitlePrefix + title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString columnHeader()
{
    return QStringLiteral( "\x1f-header-\x1f" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isSectionTitle( const QString& line )
{
    return line.startsWith( sectionTitlePrefix );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString sectionTitleText( const QString& line )
{
    return line.mid( sectionTitlePrefix.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, QString> welsegsRecordComments( const RigMswTableData& mswData )
{
    std::map<size_t, QString> comments;
    if ( !mswData.hasWelsegsData() ) return comments;

    const QString sep      = separator();
    const QString wellName = QString::fromStdString( mswData.wellName() );

    const WelsegsRow* previous    = nullptr;
    size_t            recordIndex = 1; // Record 0 is the header record
    for ( const auto& row : mswData.welsegsData() )
    {
        const bool isCompletionGroup = !row.groupLabel.empty();
        const bool newGroup = !previous || row.groupLabel != previous->groupLabel || ( !isCompletionGroup && row.branch != previous->branch );
        if ( newGroup )
        {
            if ( isCompletionGroup )
            {
                comments[recordIndex] = sectionTitle( QString::fromStdString( row.groupLabel ) );
            }
            else
            {
                const QString source  = QString::fromStdString( row.sourceWellName );
                const QString name    = source == wellName ? wellName : wellName + "." + source;
                const QString title   = QString( "Branch %1: %2" ).arg( row.branch ).arg( name );
                comments[recordIndex] = QStringList{ sep, title, sep, columnHeader() }.join( '\n' );
            }
        }
        previous = &row;
        ++recordIndex;
    }
    return comments;
}
} // namespace RimKeywordRecordComments
