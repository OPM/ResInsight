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

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseUserDataParserTools.h"

#include <QString>
#include <QPointer>
#include <QStringList>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QLocale>

#include <vector>

class ColumnInfo;
class AsciiDataParseOptions;

//==================================================================================================
/// 
//==================================================================================================
class RifCsvUserDataParser
{
public:
    RifCsvUserDataParser(QString* errorText = nullptr);
    virtual ~RifCsvUserDataParser();

    bool                parse(const AsciiDataParseOptions& parseOptions);
    const TableData&    tableData() const;

    const ColumnInfo*   columnInfo(size_t columnIndex) const;
    const ColumnInfo*   dateTimeColumn() const;

    bool                parseColumnInfo(const AsciiDataParseOptions& parseOptions);
    QString             previewText(int lineCount, const AsciiDataParseOptions& parseOptions);

    QString             tryDetermineCellSeparator();
    QString             tryDetermineDecimalSeparator(const QString& cellSeparator);

    static QLocale      localeFromDecimalSeparator(const QString& decimalSeparator);

protected:
    virtual QTextStream* openDataStream() = 0;
    virtual void         closeDataStream() = 0;

private:
    bool                parseColumnInfo(QTextStream* dataStream,
                                        const AsciiDataParseOptions& parseOptions,
                                        std::vector<ColumnInfo>* columnInfoList);
    bool                parseData(const AsciiDataParseOptions& parseOptions);
    QString             columnifyText(const QString& text, const QString& cellSeparator);
    static QStringList  splitLineAndTrim(const QString& line, const QString& separator);
    static QDateTime    tryParseDateTime(const std::string& colData, const QString& format);

private:
    TableData               m_tableData;
    QString*                m_errorText;
};

//==================================================================================================
/// 
//==================================================================================================
class RifCsvUserDataFileParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataFileParser(const QString& fileName, QString* errorText = nullptr);
    virtual ~RifCsvUserDataFileParser();

protected:
    virtual QTextStream* openDataStream() override;
    virtual void         closeDataStream() override;

private:
    bool                openFile();
    void                closeFile();

private:
    QString                 m_fileName;
    QFile*                  m_file;
    QTextStream*            m_textStream;
};

//==================================================================================================
/// 
//==================================================================================================

class RifCsvUserDataPastedTextParser : public RifCsvUserDataParser
{
public:
    RifCsvUserDataPastedTextParser(const QString& text, QString* errorText = nullptr);
    virtual ~RifCsvUserDataPastedTextParser();

protected:
    virtual QTextStream* openDataStream() override;
    virtual void         closeDataStream() override;

private:
    QString                 m_text;
    QTextStream*            m_textStream;
};
