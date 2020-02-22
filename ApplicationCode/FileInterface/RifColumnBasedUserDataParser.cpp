/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RifColumnBasedUserDataParser.h"

#include "RifEclipseUserDataKeywordTools.h"
#include "RifEclipseUserDataParserTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifColumnBasedUserDataParser::RifColumnBasedUserDataParser( const QString& data, QString* errorText )
    : m_errorText( errorText )
{
    parseTableData( data );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<TableData>& RifColumnBasedUserDataParser::tableData() const
{
    return m_tableDatas;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Column* RifColumnBasedUserDataParser::columnInfo( size_t tableIndex, size_t columnIndex ) const
{
    if ( tableIndex >= m_tableDatas.size() ) return nullptr;

    if ( columnIndex >= m_tableDatas[tableIndex].columnInfos().size() ) return nullptr;

    return &( m_tableDatas[tableIndex].columnInfos()[columnIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifColumnBasedUserDataParser::parseTableData( const QString& data )
{
    std::string stdData      = data.toStdString();
    bool        isFixedWidth = RifEclipseUserDataParserTools::isFixedWidthHeader( stdData );

    std::stringstream streamData;
    streamData.str( stdData );

    std::vector<TableData> rawTables;

    do
    {
        std::vector<std::string> errorStrings;

        TableData table;

        if ( isFixedWidth )
        {
            auto columnInfos = RifEclipseUserDataParserTools::columnInfoForFixedColumnWidth( streamData );
            table            = TableData( "", "", columnInfos );
        }
        else
        {
            table = RifEclipseUserDataParserTools::tableDataFromText( streamData, &errorStrings );
        }

        if ( m_errorText )
        {
            for ( auto s : errorStrings )
            {
                QString errorText = QString( "\n%1" ).arg( QString::fromStdString( s ) );
                m_errorText->append( errorText );
            }
        }

        std::vector<Column>& columnInfos = table.columnInfos();
        int                  columnCount = static_cast<int>( columnInfos.size() );
        if ( columnCount == 0 ) break;

        int stepTypeIndex = -1;
        for ( size_t i = 0; i < columnInfos.size(); i++ )
        {
            if ( RifEclipseUserDataKeywordTools::isStepType( columnInfos[i].summaryAddress.quantityName() ) )
            {
                stepTypeIndex = static_cast<int>( i );
            }
        }

        std::string line;
        std::getline( streamData, line );

        do
        {
            QString     qLine   = QString::fromStdString( line );
            QStringList entries = qLine.split( " ", QString::SkipEmptyParts );

            if ( stepTypeIndex > -1 && (unsigned int)entries.size() < columnInfos.size() )
            {
                entries.insert( stepTypeIndex, " " );
            }

            if ( entries.size() < columnCount ) break;

            for ( int i = 0; i < columnCount; i++ )
            {
                if ( columnInfos[i].dataType == Column::TEXT )
                {
                    columnInfos[i].textValues.push_back( entries[i].toStdString() );
                }
                else
                {
                    double entry = entries[i].toDouble();
                    columnInfos[i].values.push_back( entry );
                }
            }
        } while ( std::getline( streamData, line ) );

        rawTables.push_back( table );

    } while ( streamData.good() );

    m_tableDatas = RifEclipseUserDataParserTools::mergeEqualTimeSteps( rawTables );
}
