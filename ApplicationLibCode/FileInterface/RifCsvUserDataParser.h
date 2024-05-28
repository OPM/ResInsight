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

#pragma once

#include "RifEclipseUserDataParserTools.h"

#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <vector>

class Column;
class RifAsciiDataParseOptions;

//==================================================================================================
///
//==================================================================================================
class RifCsvUserDataParser
{
public:
    enum CsvLayout
    {
        ColumnBased,
        LineBased
    };

public:
    RifCsvUserDataParser( QString* errorText = nullptr );
    virtual ~RifCsvUserDataParser();

    bool             parse( const RifAsciiDataParseOptions&                      parseOptions,
                            const std::map<QString, QString>&                    nameMapping = {},
                            const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    const TableData& tableData() const;

    const Column* columnInfo( size_t columnIndex ) const;
    const Column* dateTimeColumn() const;

    bool        parseColumnInfo( const RifAsciiDataParseOptions& parseOptions );
    QString     previewText( int lineCount, const RifAsciiDataParseOptions& parseOptions );
    QStringList timeColumnPreviewData( int lineCount, const RifAsciiDataParseOptions& parseOptions );

    CsvLayout determineCsvLayout();

    QString tryDetermineCellSeparator();
    QString tryDetermineDecimalSeparator( const QString& cellSeparator );

    static QLocale localeFromDecimalSeparator( const QString& decimalSeparator );

protected:
    virtual QTextStream* openDataStream()  = 0;
    virtual void         closeDataStream() = 0;

private:
    std::vector<int> parseLineBasedHeader( QStringList headerCols );

    bool             parseColumnInfo( QTextStream*                                         dataStream,
                                      const RifAsciiDataParseOptions&                      parseOptions,
                                      std::vector<Column>*                                 columnInfoList,
                                      const std::map<QString, QString>&                    nameMapping = {},
                                      const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    bool             parseColumnBasedData( const RifAsciiDataParseOptions&                      parseOptions,
                                           const std::map<QString, QString>&                    nameMapping = {},
                                           const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    bool             parseLineBasedData( const RifAsciiDataParseOptions& parseOptions );
    static QDateTime tryParseDateTime( const std::string& colData, const QString& format );

private:
    TableData m_tableData;
    QString*  m_errorText;
};

//==================================================================================================
///
//==================================================================================================
class RifCsvUserDataFileParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataFileParser( const QString& fileName, QString* errorText = nullptr );
    ~RifCsvUserDataFileParser() override;

protected:
    QTextStream* openDataStream() override;
    void         closeDataStream() override;

private:
    bool openFile();
    void closeFile();

private:
    QString      m_fileName;
    QFile*       m_file;
    QTextStream* m_textStream;
};

//==================================================================================================
///
//==================================================================================================

class RifCsvUserDataPastedTextParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataPastedTextParser( const QString& text, QString* errorText = nullptr );
    ~RifCsvUserDataPastedTextParser() override;

protected:
    QTextStream* openDataStream() override;
    void         closeDataStream() override;

private:
    QString      m_text;
    QTextStream* m_textStream;
};
