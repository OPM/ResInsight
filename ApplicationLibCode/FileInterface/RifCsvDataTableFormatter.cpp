/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-    Equinor ASA
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

#include "RifCsvDataTableFormatter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter::RifCsvDataTableFormatter( QTextStream& out, const QString fieldSeparator )
    : m_out( out )
    , m_fieldSeparator( fieldSeparator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter& RifCsvDataTableFormatter::header( const std::vector<RifTextDataTableColumn>& tableHeader )
{
    outputBuffer();
    m_columnHeaders = tableHeader;

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter& RifCsvDataTableFormatter::add( const QString& str )
{
    QString quotedString = "\"" + str + "\"";
    m_lineBuffer.push_back( quotedString );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter& RifCsvDataTableFormatter::add( double num )
{
    size_t column = m_lineBuffer.size();
    m_lineBuffer.push_back( RifTextDataTableFormatter::format( num, m_columnHeaders[column].doubleFormat ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter& RifCsvDataTableFormatter::add( int num )
{
    m_lineBuffer.push_back( RifTextDataTableFormatter::format( num ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvDataTableFormatter& RifCsvDataTableFormatter::add( size_t num )
{
    m_lineBuffer.push_back( RifTextDataTableFormatter::format( num ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCsvDataTableFormatter::rowCompleted()
{
    RifTextDataTableLine line;
    line.data          = m_lineBuffer;
    line.lineType      = CONTENTS;
    line.appendTextSet = false;
    m_buffer.push_back( line );
    m_lineBuffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCsvDataTableFormatter::tableCompleted()
{
    outputBuffer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCsvDataTableFormatter::outputBuffer()
{
    if ( !m_columnHeaders.empty() )
    {
        for ( size_t i = 0; i < m_columnHeaders.size(); i++ )
        {
            m_out << m_columnHeaders[i].title();

            if ( i < m_columnHeaders.size() - 1 )
            {
                m_out << m_fieldSeparator;
            }
        }
        m_out << "\n";
    }

    for ( const auto& line : m_buffer )
    {
        if ( line.lineType == CONTENTS )
        {
            QString lineText;
            for ( size_t i = 0; i < line.data.size(); i++ )
            {
                lineText += line.data[i];
                if ( i < line.data.size() - 1 )
                {
                    lineText += m_fieldSeparator;
                }
            }

            m_out << lineText << "\n";
        }
    }
    m_columnHeaders.clear();
    m_buffer.clear();
}
