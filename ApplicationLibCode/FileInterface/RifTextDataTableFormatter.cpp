/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RifTextDataTableFormatter.h"

#include "cvfAssert.h"

#include <limits>

#define MAX_ECLIPSE_DATA_ROW_WIDTH 132 // Maximum eclipse data row width

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter::RifTextDataTableFormatter( QTextStream& out )
    : m_out( out )
    , m_colSpacing( 5 )
    , m_tableRowPrependText( "   " )
    , m_tableRowAppendText( " /" )
    , m_commentPrefix( "-- " )
    , m_headerPrefix( "-- " )
    , m_maxDataRowWidth( MAX_ECLIPSE_DATA_ROW_WIDTH )
    , m_defaultMarker( "1*" )
    , m_isOptionalCommentEnabled( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter::RifTextDataTableFormatter( const RifTextDataTableFormatter& rhs )
    : m_out( rhs.m_out )
    , m_colSpacing( rhs.m_colSpacing )
    , m_tableRowPrependText( rhs.m_tableRowPrependText )
    , m_tableRowAppendText( rhs.m_tableRowAppendText )
    , m_commentPrefix( rhs.m_commentPrefix )
    , m_headerPrefix( rhs.m_headerPrefix )
    , m_maxDataRowWidth( rhs.m_maxDataRowWidth )
    , m_defaultMarker( rhs.m_defaultMarker )
    , m_isOptionalCommentEnabled( rhs.isOptionalCommentEnabled() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter::~RifTextDataTableFormatter()
{
    CVF_ASSERT( m_buffer.empty() );
    CVF_ASSERT( m_columns.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::columnSpacing() const
{
    return m_colSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setColumnSpacing( int spacing )
{
    m_colSpacing = spacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::tableRowPrependText() const
{
    return m_tableRowPrependText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::tableRowAppendText() const
{
    return m_tableRowAppendText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setTableRowPrependText( const QString& text )
{
    m_tableRowPrependText = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setTableRowLineAppendText( const QString& text )
{
    m_tableRowAppendText = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::commentPrefix() const
{
    return m_commentPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setCommentPrefix( const QString& commentPrefix )
{
    m_commentPrefix = commentPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::headerPrefix() const
{
    return m_headerPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setHeaderPrefix( const QString& headerPrefix )
{
    m_headerPrefix = headerPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setUnlimitedDataRowWidth()
{
    m_maxDataRowWidth = std::numeric_limits<int>::max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::maxDataRowWidth() const
{
    return m_maxDataRowWidth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setDefaultMarker( const QString& defaultMarker )
{
    m_defaultMarker = defaultMarker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::defaultMarker() const
{
    return m_defaultMarker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::setOptionalComment( bool enable )
{
    m_isOptionalCommentEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifTextDataTableFormatter::isOptionalCommentEnabled() const
{
    return m_isOptionalCommentEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::outputBuffer()
{
    if ( !m_columns.empty() && !isAllHeadersEmpty( m_columns ) )
    {
        m_out << m_headerPrefix;
        for ( size_t i = 0u; i < m_columns.size(); ++i )
        {
            m_out << formatColumn( m_columns[i].title, i );
        }
        m_out << "\n";
    }

    for ( auto line : m_buffer )
    {
        if ( line.lineType == COMMENT )
        {
            outputComment( line );
        }
        else if ( line.lineType == HORIZONTAL_LINE )
        {
            outputHorizontalLine( line );
        }
        else if ( line.lineType == CONTENTS )
        {
            QString lineText   = m_tableRowPrependText;
            bool    isComment  = m_tableRowPrependText.startsWith( m_commentPrefix );
            QString appendText = ( line.appendTextSet ? line.appendText : m_tableRowAppendText );

            for ( size_t i = 0; i < line.data.size(); ++i )
            {
                QString column      = formatColumn( line.data[i], i );
                QString newLineText = lineText + column;
                if ( i == line.data.size() - 1 )
                {
                    newLineText += appendText;
                }
                if ( !isComment && newLineText.length() > maxDataRowWidth() )
                {
                    m_out << lineText << "\n";
                    lineText = m_tableRowPrependText;
                }
                lineText += column;
            }

            m_out << lineText << appendText << "\n";
        }
    }
    m_columns.clear();
    m_buffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::outputComment( const RifTextDataTableLine& comment )
{
    m_out << m_commentPrefix << comment.data[0] << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::outputHorizontalLine( RifTextDataTableLine& comment )
{
    if ( comment.lineType == HORIZONTAL_LINE )
    {
        int charCount = tableWidth();

        QChar fillChar = ' ';
        if ( !comment.data.empty() )
        {
            QString firstString = comment.data[0];
            if ( !firstString.isEmpty() )
            {
                fillChar = firstString[0];
            }
        }

        QString str;
        str.fill( fillChar, charCount );

        m_out << m_commentPrefix << str << "\n";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifTextDataTableFormatter::isAllHeadersEmpty( const std::vector<RifTextDataTableColumn>& headers )
{
    for ( auto& header : headers )
    {
        if ( !header.title.isEmpty() ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::tableCompleted()
{
    outputBuffer();

    // Output an "empty" line after a finished table
    m_out << m_tableRowPrependText << m_tableRowAppendText << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::tableCompleted( const QString& appendText, bool appendNewline )
{
    outputBuffer();

    // Output an "empty" line after a finished table
    if ( !appendText.isEmpty() || appendNewline )
    {
        m_out << m_tableRowPrependText << appendText << ( appendNewline ? "\n" : "" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::keyword( const QString& keyword )
{
    CVF_ASSERT( m_buffer.empty() );
    CVF_ASSERT( m_columns.empty() );
    m_out << keyword << "\n";
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::header( const std::vector<RifTextDataTableColumn> header )
{
    outputBuffer();
    m_columns = header;

    for ( size_t colNumber = 0u; colNumber < m_columns.size(); ++colNumber )
    {
        m_columns[colNumber].width = measure( m_columns[colNumber].title );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::comment( const QString& comment )
{
    RifTextDataTableLine line;
    line.data.push_back( comment );
    line.lineType      = COMMENT;
    line.appendTextSet = false;
    if ( m_columns.empty() )
    {
        outputComment( line );
    }
    else
    {
        m_buffer.push_back( line );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::addOptionalComment( const QString& str )
{
    if ( m_isOptionalCommentEnabled )
    {
        return comment( str );
    }

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::addHorizontalLine( const QChar& character )
{
    RifTextDataTableLine line;
    QString              data;
    data += character;
    line.data.push_back( data );
    line.lineType      = HORIZONTAL_LINE;
    line.appendTextSet = false;
    if ( m_columns.empty() )
    {
        outputComment( line );
    }
    else
    {
        m_buffer.push_back( line );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::add( const QString& str )
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT( column < m_columns.size() );
    m_columns[column].width = std::max( measure( str ), m_columns[column].width );
    m_lineBuffer.push_back( str );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::add( double num )
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT( column < m_columns.size() );
    m_columns[column].width = std::max( measure( num, m_columns[column].doubleFormat ), m_columns[column].width );
    m_lineBuffer.push_back( format( num, m_columns[column].doubleFormat ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::add( int num )
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT( column < m_columns.size() );
    m_columns[column].width = std::max( measure( num ), m_columns[column].width );
    m_lineBuffer.push_back( format( num ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::add( size_t num )
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT( column < m_columns.size() );
    m_columns[column].width = std::max( measure( num ), m_columns[column].width );
    m_lineBuffer.push_back( format( num ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::addOneBasedCellIndex( size_t zeroBasedIndex )
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT( column < m_columns.size() );

    // Increase index by 1 to use Eclipse 1-based cell index instead of ResInsight 0-based
    zeroBasedIndex++;

    m_columns[column].width = std::max( measure( zeroBasedIndex ), m_columns[column].width );
    m_lineBuffer.push_back( format( zeroBasedIndex ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
/// Add default marker if the value equals the defaultValue, otherwise add value.
//--------------------------------------------------------------------------------------------------
RifTextDataTableFormatter& RifTextDataTableFormatter::addValueOrDefaultMarker( double value, double defaultValue )
{
    if ( value == defaultValue )
    {
        return add( m_defaultMarker );
    }
    return add( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifTextDataTableFormatter::rowCompleted()
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
void RifTextDataTableFormatter::rowCompleted( const QString& appendText )
{
    RifTextDataTableLine line;
    line.data          = m_lineBuffer;
    line.lineType      = CONTENTS;
    line.appendTextSet = true;
    line.appendText    = appendText;
    m_buffer.push_back( line );
    m_lineBuffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::measure( const QString str )
{
    return str.length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::measure( double num, RifTextDataTableDoubleFormatting doubleFormat )
{
    return format( num, doubleFormat ).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::measure( int num )
{
    return format( num ).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::measure( size_t num )
{
    return format( num ).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifTextDataTableFormatter::tableWidth() const
{
    int characterCount = m_tableRowPrependText.length();

    for ( size_t i = 0u; i < m_columns.size(); ++i )
    {
        characterCount += formatColumn( " ", i ).size();
    }
    characterCount += m_tableRowAppendText.length();

    return characterCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::format( double num, RifTextDataTableDoubleFormatting doubleFormat )
{
    switch ( doubleFormat.format )
    {
        case RifTextDataTableDoubleFormat::RIF_FLOAT:
            return QString( "%1" ).arg( num, 0, 'f', doubleFormat.precision );
        case RifTextDataTableDoubleFormat::RIF_SCIENTIFIC:
            return QString( "%1" ).arg( num, 0, 'E' );
        case RifTextDataTableDoubleFormat::RIF_CONSISE:
            return QString::number( num, 'g', doubleFormat.precision );
        default:
            return QString( "%1" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::format( int num )
{
    return QString( "%1" ).arg( num );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::format( size_t num )
{
    return QString( "%1" ).arg( num );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifTextDataTableFormatter::formatColumn( const QString str, size_t columnIndex ) const
{
    const RifTextDataTableColumn& column = m_columns[columnIndex];

    if ( column.alignment == LEFT )
    {
        int colSpacing = ( columnIndex == m_columns.size() - 1 ) ? 0 : m_colSpacing;
        return str.leftJustified( column.width + colSpacing, ' ' );
    }
    else
    {
        int colSpacing = ( columnIndex == 0 ) ? 0 : m_colSpacing;
        return str.rightJustified( column.width + colSpacing, ' ' );
    }
}
