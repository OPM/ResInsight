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
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <expected>
#include <memory>
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
    RifCsvUserDataParser();
    virtual ~RifCsvUserDataParser();

    std::expected<void, QString> parse( const RifAsciiDataParseOptions&                      parseOptions,
                                        const std::map<QString, QString>&                    nameMapping = {},
                                        const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    const TableData&             tableData() const;

    const Column* columnInfo( size_t columnIndex ) const;
    const Column* dateTimeColumn() const;

    bool        parseColumnInfo( const RifAsciiDataParseOptions& parseOptions );
    QString     previewText( int lineCount, const RifAsciiDataParseOptions& parseOptions );
    QStringList timeColumnPreviewData( int lineCount, const RifAsciiDataParseOptions& parseOptions );

    CsvLayout determineCsvLayout( const QString& cellSeparator );

    QString tryDetermineCellSeparator();
    QString tryDetermineDecimalSeparator( const QString& cellSeparator );

    static QLocale localeFromDecimalSeparator( const QString& decimalSeparator );

protected:
    virtual std::expected<std::unique_ptr<QTextStream>, QString> openDataStream() = 0;

private:
    std::vector<int> parseLineBasedHeader( QStringList headerCols );

    std::expected<int, QString>  parseColumnInfo( QTextStream&                                         dataStream,
                                                  const RifAsciiDataParseOptions&                      parseOptions,
                                                  std::vector<Column>&                                 columnInfoList,
                                                  const std::map<QString, QString>&                    nameMapping = {},
                                                  const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    std::expected<void, QString> parseColumnBasedData( const RifAsciiDataParseOptions&                      parseOptions,
                                                       const std::map<QString, QString>&                    nameMapping = {},
                                                       const std::map<QString, std::pair<QString, double>>& unitMapping = {} );
    std::expected<void, QString> parseLineBasedData( const RifAsciiDataParseOptions& parseOptions );
    static QDateTime             tryParseDateTime( const QString& colData, const QString& format );
    static QDateTime             parseDateTime( const QString& colData, const RifAsciiDataParseOptions& parseOptions );
    static QString               formatParseError( const QString& message, int lineNumber );

private:
    TableData m_tableData;
};

//==================================================================================================
///
//==================================================================================================
class RifCsvUserDataFileParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataFileParser( const QString& fileName );
    ~RifCsvUserDataFileParser() override = default;

protected:
    std::expected<std::unique_ptr<QTextStream>, QString> openDataStream() override;

private:
    QString                m_fileName;
    std::unique_ptr<QFile> m_file;
};

//==================================================================================================
///
//==================================================================================================

class RifCsvUserDataPastedTextParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataPastedTextParser( const QString& text );
    ~RifCsvUserDataPastedTextParser() override = default;

protected:
    std::expected<std::unique_ptr<QTextStream>, QString> openDataStream() override;

private:
    QString m_text;
};
