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

#pragma once

#include <QString>
#include <QTextStream>

#include <vector>

//==================================================================================================
//
//==================================================================================================
enum RifTextDataTableLineType
{
    COMMENT,
    CONTENTS,
    HORIZONTAL_LINE
};

//==================================================================================================
//
//==================================================================================================
enum RifTextDataTableAlignment
{
    LEFT,
    RIGHT
};

//==================================================================================================
//
//==================================================================================================
enum RifTextDataTableDoubleFormat
{
    RIF_SCIENTIFIC,
    RIF_FLOAT,
    RIF_CONSISE
};

//==================================================================================================
//
//==================================================================================================
struct RifTextDataTableLine
{
    RifTextDataTableLineType lineType;
    std::vector<QString>     data;
    bool                     appendTextSet;
    QString                  appendText;
};

//==================================================================================================
//
//==================================================================================================
struct RifTextDataTableDoubleFormatting
{
    RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat format = RIF_FLOAT, int precision = 5 )
        : format( format )
        , precision( precision )
    {
    }

    RifTextDataTableDoubleFormat format;
    int                          precision;
};

//==================================================================================================
//
//==================================================================================================
struct RifTextDataTableColumn
{
    RifTextDataTableColumn( const QString&                   title,
                            RifTextDataTableDoubleFormatting doubleFormat = RifTextDataTableDoubleFormatting(),
                            RifTextDataTableAlignment        alignment    = LEFT,
                            int                              width        = -1 )
        : doubleFormat( doubleFormat )
        , alignment( alignment )
        , width( width )
    {
        titles.push_back( title );
    }

    RifTextDataTableColumn( const QString&                   title,
                            const QString&                   subTitle,
                            RifTextDataTableDoubleFormatting doubleFormat = RifTextDataTableDoubleFormatting(),
                            RifTextDataTableAlignment        alignment    = LEFT,
                            int                              width        = -1 )
        : doubleFormat( doubleFormat )
        , alignment( alignment )
        , width( width )
    {
        titles.push_back( title );
        titles.push_back( subTitle );
    }

    RifTextDataTableColumn( const QString&                   title,
                            const QString&                   subTitle1,
                            const QString&                   subTitle2,
                            RifTextDataTableDoubleFormatting doubleFormat = RifTextDataTableDoubleFormatting(),
                            RifTextDataTableAlignment        alignment    = LEFT,
                            int                              width        = -1 )
        : doubleFormat( doubleFormat )
        , alignment( alignment )
        , width( width )
    {
        titles.push_back( title );
        titles.push_back( subTitle1 );
        titles.push_back( subTitle2 );
    }

    QString title() const
    {
        if ( !titles.empty() ) return titles.front();

        return "";
    }

    RifTextDataTableDoubleFormatting doubleFormat;
    RifTextDataTableAlignment        alignment;
    int                              width;
    std::vector<QString>             titles;
};

//==================================================================================================
//
//==================================================================================================
class RifTextDataTableFormatter
{
public:
    RifTextDataTableFormatter( QTextStream& out );
    RifTextDataTableFormatter( const RifTextDataTableFormatter& rhs );

    virtual ~RifTextDataTableFormatter();

    int     columnSpacing() const;
    void    setColumnSpacing( int spacing );
    QString tableRowPrependText() const;
    QString tableRowAppendText() const;
    void    setTableRowPrependText( const QString& text );
    void    setTableRowLineAppendText( const QString& text );
    QString commentPrefix() const;
    void    setCommentPrefix( const QString& commentPrefix );
    QString headerPrefix() const;
    void    setHeaderPrefix( const QString& headerPrefix );
    void    setUnlimitedDataRowWidth();
    int     maxDataRowWidth() const;
    void    setDefaultMarker( const QString& defaultMarker );
    QString defaultMarker() const;

    void setOptionalComment( bool enable );
    bool isOptionalCommentEnabled() const;

    RifTextDataTableFormatter& keyword( const QString& keyword );
    RifTextDataTableFormatter& header( std::vector<RifTextDataTableColumn> tableHeader );
    RifTextDataTableFormatter& add( const QString& str );
    RifTextDataTableFormatter& add( double num );
    RifTextDataTableFormatter& add( int num );
    RifTextDataTableFormatter& add( size_t num );
    RifTextDataTableFormatter& addOneBasedCellIndex( size_t zeroBasedIndex );
    RifTextDataTableFormatter& addValueOrDefaultMarker( double value, double defaultValue );
    RifTextDataTableFormatter& comment( const QString& str );
    RifTextDataTableFormatter& addOptionalComment( const QString& str );
    RifTextDataTableFormatter& addHorizontalLine( const QChar& str );
    void                       rowCompleted();
    void                       rowCompleted( const QString& appendText );
    void                       tableCompleted();
    void                       tableCompleted( const QString& appendText, bool appendNewline );

    int tableWidth() const;

protected:
    friend class RifCsvDataTableFormatter;

    int measure( const QString str );
    int measure( double num, RifTextDataTableDoubleFormatting doubleFormat );
    int measure( int num );
    int measure( size_t num );

    static QString format( double num, RifTextDataTableDoubleFormatting doubleFormat );
    static QString format( int num );
    static QString format( size_t num );
    QString        formatColumn( const QString str, size_t columnIndex ) const;

    void outputBuffer();
    void outputComment( const RifTextDataTableLine& comment );
    void outputHorizontalLine( RifTextDataTableLine& comment );

    bool isAllHeadersEmpty( const std::vector<RifTextDataTableColumn>& headers );

private:
    std::vector<RifTextDataTableColumn> m_columns;
    std::vector<RifTextDataTableLine>   m_buffer;
    std::vector<QString>                m_lineBuffer;
    QTextStream&                        m_out;
    int                                 m_colSpacing;
    QString                             m_tableRowPrependText;
    QString                             m_tableRowAppendText;
    QString                             m_commentPrefix;
    QString                             m_headerPrefix;
    int                                 m_maxDataRowWidth;
    QString                             m_defaultMarker;
    bool                                m_isOptionalCommentEnabled;
};
