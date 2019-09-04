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
enum RifEclipseOutputTableLineType
{
    COMMENT,
    CONTENTS,
    HORIZONTAL_LINE
};

//==================================================================================================
//
//==================================================================================================
enum RifEclipseOutputTableAlignment
{
    LEFT,
    RIGHT
};

//==================================================================================================
//
//==================================================================================================
enum RifEclipseOutputTableDoubleFormat
{
    RIF_SCIENTIFIC,
    RIF_FLOAT,
    RIF_CONSISE
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableLine
{
    RifEclipseOutputTableLineType lineType;
    std::vector<QString>          data;
    bool                          appendTextSet;
    QString                       appendText;
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableDoubleFormatting
{
    RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat format = RIF_FLOAT, int precision = 5)
        : format(format)
        , precision(precision)
    {
    }

    RifEclipseOutputTableDoubleFormat format;
    int                               precision;
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableColumn
{
    RifEclipseOutputTableColumn(const QString&                        title,
                                RifEclipseOutputTableDoubleFormatting doubleFormat = RifEclipseOutputTableDoubleFormatting(),
                                RifEclipseOutputTableAlignment        alignment    = LEFT,
                                int                                   width        = -1)
        : title(title)
        , doubleFormat(doubleFormat)
        , alignment(alignment)
        , width(width)
    {
    }

    QString                               title;
    RifEclipseOutputTableDoubleFormatting doubleFormat;
    RifEclipseOutputTableAlignment        alignment;
    int                                   width;
};

//==================================================================================================
//
//==================================================================================================
class RifEclipseDataTableFormatter
{
public:
    RifEclipseDataTableFormatter(QTextStream& out);
    RifEclipseDataTableFormatter(const RifEclipseDataTableFormatter& rhs);

    virtual ~RifEclipseDataTableFormatter();

    int     columnSpacing() const;
    void    setColumnSpacing(int spacing);
    QString tableRowPrependText() const;
    QString tableRowAppendText() const;
    void    setTableRowPrependText(const QString& text);
    void    setTableRowLineAppendText(const QString& text);
    QString commentPrefix() const;
    void    setCommentPrefix(const QString& commentPrefix);
    void    setUnlimitedDataRowWidth();
    int     maxDataRowWidth() const;

    RifEclipseDataTableFormatter& keyword(const QString& keyword);
    RifEclipseDataTableFormatter& header(std::vector<RifEclipseOutputTableColumn> tableHeader);
    RifEclipseDataTableFormatter& add(const QString& str);
    RifEclipseDataTableFormatter& add(double num);
    RifEclipseDataTableFormatter& add(int num);
    RifEclipseDataTableFormatter& add(size_t num);
    RifEclipseDataTableFormatter& addOneBasedCellIndex(size_t zeroBasedIndex);
    RifEclipseDataTableFormatter& addValueOrDefaultMarker(double value, double defaultValue);
    RifEclipseDataTableFormatter& comment(const QString& str);
    RifEclipseDataTableFormatter& addHorizontalLine(const QChar& str);
    void                          rowCompleted();
    void                          rowCompleted(const QString& appendText);
    void                          tableCompleted();
    void                          tableCompleted(const QString& appendText, bool appendNewline);

    int tableWidth() const;

protected:
    friend class RifCsvDataTableFormatter;

    int measure(const QString str);
    int measure(double num, RifEclipseOutputTableDoubleFormatting doubleFormat);
    int measure(int num);
    int measure(size_t num);

    static QString format(double num, RifEclipseOutputTableDoubleFormatting doubleFormat);
    static QString format(int num);
    static QString format(size_t num);
    QString        formatColumn(const QString str, size_t columnIndex) const;

    void outputBuffer();
    void outputComment(RifEclipseOutputTableLine& comment);
    void outputHorizontalLine(RifEclipseOutputTableLine& comment);

    bool isAllHeadersEmpty(const std::vector<RifEclipseOutputTableColumn>& headers);

private:
    std::vector<RifEclipseOutputTableColumn> m_columns;
    std::vector<RifEclipseOutputTableLine>   m_buffer;
    std::vector<QString>                     m_lineBuffer;
    QTextStream&                             m_out;
    int                                      m_colSpacing;
    QString                                  m_tableRowPrependText;
    QString                                  m_tableRowAppendText;
    QString                                  m_commentPrefix;
    int                                      m_maxDataRowWidth;
};
